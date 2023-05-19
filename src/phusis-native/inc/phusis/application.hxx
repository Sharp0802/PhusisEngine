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
		uint32_t _bufferCnt;

	private:
		VkPhysicalDevice _physicalDevice = nullptr;
		VkDevice _device = nullptr;
		uint32_t _queueFamilyIdx = 0;
		VkPresentModeKHR _presentMode = VK_PRESENT_MODE_FIFO_KHR;
		VkQueue _queue = nullptr;
		VkInstance _instance = nullptr;
		VkSurfaceKHR _surface = nullptr;
		VkSwapchainKHR _swapchain = nullptr;
		std::vector<VkImage> _buffers{};
		VkCommandPool _primaryCommandPool = nullptr;
		std::vector<VkThreadData> _threads{std::thread::hardware_concurrency()};

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
