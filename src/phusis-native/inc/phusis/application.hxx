#ifndef PHUSIS_APPLICATION_HXX
#define PHUSIS_APPLICATION_HXX

#include "fw.hxx"
#include "engineobject.hxx"

namespace Phusis
{
	enum class ApplicationMode
	{
		Performant,
		Quality
	};

	using Window = GLFWwindow*;

	class ThreadData
	{
	public:
		VkCommandPool CommandPool;
		VkCommandBuffer CommandBuffer;
	};

	class Application
	{
	private:
		std::vector<std::string> _requiredLayers;
		std::vector<std::string> _requiredExtensions;
		ApplicationMode _mode;

	public:
		uint32_t Width, Height;

		VkPhysicalDevice PhysicalDevice = nullptr;
		VkDevice Device = nullptr;
		VkInstance Instance = nullptr;
		VkSurfaceKHR Surface = nullptr;
		VkSwapchainKHR Swapchain = nullptr;
		VkQueue Queue = nullptr;
		VkCommandPool PrimaryCommandPool = nullptr;
		VkCommandBuffer PrimaryCommandBuffer = nullptr;
		VkRenderPass RenderPass = nullptr;
		VkPipelineLayout PipelineLayout = nullptr;
		VkPipeline Pipeline = nullptr;

		glm::mat4 Projection, View;

	private:
		VkPresentModeKHR _presentMode = VK_PRESENT_MODE_FIFO_KHR;

		uint32_t _queueFamilyIdx = 0;

		std::vector<VkImage> _swapchainBuffers{};
		std::vector<VkImageView> _swapchainViews{};
		std::vector<ThreadData> _threads{ std::thread::hardware_concurrency()};

		std::vector<VkFramebuffer> _framebuffers{};

		std::vector<EngineObjectData> _objects{};

	private:
		VkSurfaceFormatKHR _surfaceFormat{};
		VkSurfaceCapabilitiesKHR _surfaceCapabilities{};
		VkSwapchainCreateInfoKHR _swapchainInfo{};
		VkRenderPassCreateInfo _defaultRenderPassInfo{};

	private:
		Window _window = nullptr;

	public:
		Application(
				const std::vector<std::string>& requiredLayers,
				const std::vector<std::string>& requiredExtensions,
				ApplicationMode mode) noexcept;

		~Application() noexcept;

	private:
		bool GLFWInitialize() noexcept;

		bool GLFWCreateWindow() noexcept;

		bool VkValidateLayer() noexcept;

		bool VkValidateExtension() noexcept;

		bool VkInitializeInstance() noexcept;

		bool VkInitializePhysicalDevice() noexcept;

		bool VkInitializeLogicalDevice() noexcept;

		bool VkCreateSurface() noexcept;

		bool VkValidateSwapchain() noexcept;

		bool VkInitializeSurface() noexcept;

		bool VkInitializeSwapchain() noexcept;

		bool VkInitializeImageViews() noexcept;

		bool VkInitializeCommandPool() noexcept;

		bool VkInitializeCommandBuffer() noexcept;

		bool VkInitializeRenderPass() noexcept;

		bool VkInitializeFramebuffers() noexcept;

		bool VkInitializePipelineLayout() noexcept;

		bool UpdateCommandBuffers(VkFramebuffer buffer) noexcept;

	private:
		int32_t InitializeDeviceDependents() noexcept;
		void ReleaseDeviceDependents() noexcept;
		void ReleaseDeviceIndependents() noexcept;

	public:
		int32_t InitializeComponents() noexcept;

		int32_t Run() noexcept;
	};
}

#endif //PHUSIS_APPLICATION_HXX
