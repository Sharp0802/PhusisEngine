#ifndef PHUSIS_APPLICATION_HXX
#define PHUSIS_APPLICATION_HXX

#include "fw.hxx"


namespace Phusis
{
	enum class ApplicationMode
	{
		Performant,
		Quality
	};

	using Window = GLFWwindow*;

	class VkThreadData
	{
	public:
		VkCommandPool CommandPool;
		std::vector<VkCommandBuffer> CommandBuffer;
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
				ApplicationMode mode,
				uint32_t bufferCnt) noexcept;

		~Application() noexcept;

	private:
		bool VkValidateLayer() noexcept;

		bool VkInitializeInstance() noexcept;

		bool VkInitializePhysicalDevice() noexcept;

		bool VkInitializeDeviceQueue() noexcept;

		bool VkValidateExtension() noexcept;

		bool GLFWInitialize() noexcept;

		bool GLFWCreateWindow() noexcept;

		bool GLFWCreateSurface() noexcept;

		bool VkValidateSwapchain() noexcept;

		bool VkInitializeSwapchain() noexcept;

		bool VkInitializeCommandPool() noexcept;

		bool VkInitializeCommandBuffer() noexcept;

	public:
		int32_t InitializeComponents() noexcept;
	};
}

#endif //PHUSIS_APPLICATION_HXX
