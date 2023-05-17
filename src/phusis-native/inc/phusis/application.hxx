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

	class Application
	{
	private:
		std::vector<std::string> _requiredLayers;
		std::vector<std::string> _requiredExtensions;
		ApplicationMode _mode;

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
		VkCommandPool _commandPool = nullptr;

	private:
		Window _window = nullptr;

	public:
		Application(
				const std::vector<std::string>& requiredLayers,
				const std::vector<std::string>& requiredExtensions,
				ApplicationMode mode) noexcept;

		~Application() noexcept;

	public:
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
	};
}

#endif //PHUSIS_APPLICATION_HXX
