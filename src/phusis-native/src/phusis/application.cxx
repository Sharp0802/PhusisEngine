#include <cstring>
#include "phusis/application.hxx"
#include "sys/logger.hxx"
#include "sys/os.hxx"

Phusis::Application::Application(
		const std::vector<std::string>& requiredLayers,
		const std::vector<std::string>& requiredExtensions,
		ApplicationMode mode) noexcept:
		_requiredLayers(requiredLayers),
		_mode(mode)
{
	// do NOT use glfwGetRequiredInstanceExtensions : it doesn't work
	_requiredExtensions = std::vector<std::string>(requiredExtensions.size() + 3);
	_requiredExtensions.assign(requiredExtensions.begin(), requiredExtensions.end());
	_requiredExtensions.emplace_back("VK_KHR_surface");
	_requiredExtensions.emplace_back(sys::os::surface);
	_requiredExtensions.emplace_back("VK_EXT_swapchain_colorspace");

	for (const auto& i: _requiredExtensions)
		sys::log.head(sys::INFO) << "EXT required: " << i << sys::EOM;
}

Phusis::Application::~Application() noexcept
{
	vkDestroyDevice(_device, nullptr);
	vkDestroyInstance(_instance, nullptr);
}

bool Phusis::Application::VkValidateLayer() noexcept
{
	uint32_t cProperty = 0;
	vkEnumerateInstanceLayerProperties(&cProperty, nullptr);

	std::vector<VkLayerProperties> properties(cProperty);
	vkEnumerateInstanceLayerProperties(&cProperty, &properties[0]);

	auto* layers = new std::string[cProperty];
	for (int i = 0; i < cProperty; ++i)
		layers[i] = std::string(properties[i].layerName);

	bool failed = false;
	for (const auto& required: _requiredLayers)
	{
		uint32_t i;
		for (i = 0; i < cProperty; ++i)
			if (layers[i] == required)
				break;
		if (i == cProperty)
		{
			failed = true;
			sys::log.head(sys::FAIL) << "vulkan layer '" << required << "' not matched" << sys::EOM;
		}
		else
		{
			sys::log.head(sys::INFO) << "vulkan layer '" << required << "' matched" << sys::EOM;
		}
	}

	delete[] layers;

	if (failed)
	{
		sys::log.head(sys::CRIT) << "missing vulkan layer detected" << sys::EOM;
	}

	return !failed;
}

bool Phusis::Application::VkInitializeInstance() noexcept
{
	size_t cLayer = _requiredLayers.size();
	const char** layers = new const char* [cLayer];
	for (int i = 0; i < cLayer; ++i)
		layers[i] = _requiredLayers[i].c_str();

	size_t cExt = _requiredExtensions.size();
	const char** exts = new const char* [cExt];
	for (int i = 0; i < cExt; ++i)
		exts[i] = _requiredExtensions[i].c_str();

	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.enabledLayerCount = cLayer;
	create_info.ppEnabledLayerNames = layers;
	create_info.enabledExtensionCount = cExt;
	create_info.ppEnabledExtensionNames = exts;

	VkInstance instance;
	VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
	_instance = instance;

	return result == VK_SUCCESS;
}

bool Phusis::Application::VkInitializePhysicalDevice() noexcept
{
	uint32_t cDevice = 0;
	vkEnumeratePhysicalDevices(_instance, &cDevice, nullptr);

	std::vector<VkPhysicalDevice> physicalDevices(cDevice);
	vkEnumeratePhysicalDevices(_instance, &cDevice, &physicalDevices[0]);

	for (const auto& device: physicalDevices)
	{
		VkPhysicalDeviceProperties prop{};
		vkGetPhysicalDeviceProperties(device, &prop);

		sys::log.head(sys::VERB) << "GPU found: " << prop.deviceName << sys::EOM;
	}

	VkPhysicalDevice physicalDevice = physicalDevices[0];
	_physicalDevice = physicalDevice;

	VkPhysicalDeviceProperties physicalProperties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalProperties);

	sys::log.head(sys::INFO) << "GPU using: " << physicalProperties.deviceName << sys::EOM;

	return true;
}

bool Phusis::Application::VkInitializeDeviceQueue() noexcept
{
	uint32_t cProperty = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &cProperty, nullptr);

	std::vector<VkQueueFamilyProperties> properties(cProperty);
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &cProperty, &properties[0]);

	uint32_t queueFamilyIdx = UINT32_MAX;
	for (uint32_t i = 0; i < properties.size(); ++i)
	{
		if (properties[i].queueCount == 0)
			continue;

		if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueFamilyIdx = i;
			break;
		}
	}
	if (queueFamilyIdx == UINT32_MAX)
	{
		sys::log.head(sys::CRIT) << "queue family not found" << sys::EOM;
		return false;
	}

	constexpr float priority = 1.f;

	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = queueFamilyIdx;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &priority;

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

	VkDevice device;
	VkResult result = vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &device);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "failed to create device" << sys::EOM;
		return false;
	}

	VkQueue queue;
	vkGetDeviceQueue(device, queueFamilyIdx, 0, &queue);

	_device = device;
	_queue = queue;

	sys::log.head(sys::INFO) << "vulkan device & queue has been ready" << sys::EOM;

	return true;
}

bool Phusis::Application::VkValidateExtension() noexcept
{
	uint32_t cExtension = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &cExtension, nullptr);

	std::vector<VkExtensionProperties> extensions(cExtension);
	vkEnumerateInstanceExtensionProperties(nullptr, &cExtension, &extensions[0]);

	for (const auto& extension: extensions)
	{
		sys::log.head(sys::VERB) << "EXT found: " << std::string(extension.extensionName) << sys::EOM;
	}

	auto* extensionNames = new std::string[cExtension];
	for (uint32_t i = 0; i < cExtension; ++i)
		extensionNames[i] = std::string(extensions[i].extensionName);

	bool failed = false;
	for (const auto& required : _requiredExtensions)
	{
		uint32_t i;
		for (i = 0; i < cExtension; ++i)
			if (extensionNames[i] == required)
				break;
		if (i == cExtension)
		{
			sys::log.head(sys::FAIL) << "EXT not matched: " << required << sys::EOM;
			failed = true;
		}
		else
		{
			sys::log.head(sys::INFO) << "EXT matched: " << required << sys::EOM;
		}
	}

	delete[] extensionNames;

	if (failed)
	{
		sys::log.head(sys::CRIT) << "missing vulkan extension detected" << sys::EOM;
	}

	return !failed;
}
