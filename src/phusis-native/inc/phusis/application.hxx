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

	class Application
	{
	private:
		std::vector<std::string> _requiredLayers;
		std::vector<std::string> _requiredExtensions;
		ApplicationMode _mode;

	private:
		VkPhysicalDevice _physicalDevice = nullptr;
		VkDevice _device = nullptr;
		VkQueue _queue = nullptr;
		VkInstance _instance = nullptr;

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
	};
}

#endif //PHUSIS_APPLICATION_HXX
