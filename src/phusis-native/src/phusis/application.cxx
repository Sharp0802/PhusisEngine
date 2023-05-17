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
	vkDestroyCommandPool(_device, _commandPool, nullptr);
	vkDestroySwapchainKHR(_device, _swapchain, nullptr);
	vkDestroySurfaceKHR(_instance, _surface, nullptr);
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

	static const std::array<const char*, 1> ext = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	uint32_t cExt;
	vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &cExt, nullptr);

	std::vector<VkExtensionProperties> extensions(cExt);
	vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &cExt, &extensions[0]);

	bool failed = false;
	for (const auto& e : ext)
	{
		uint32_t i;
		for (i = 0; i < extensions.size(); ++i)
			if (strcmp(extensions[i].extensionName, e) == 0)
				break;
		if (i == extensions.size())
		{
			sys::log.head(sys::FAIL) << "D-EXT not matched: " << e << sys::EOM;
			failed = true;
		}
		else
		{
			sys::log.head(sys::VERB) << "D-EXT matched: " << e << sys::EOM;
		}
	}
	if (failed)
	{
		sys::log.head(sys::CRIT) << "missing vulkan device extension detected" << sys::EOM;
		return false;
	}

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(ext.size());
	deviceCreateInfo.ppEnabledExtensionNames = &ext[0];
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
	_queueFamilyIdx = queueFamilyIdx;

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
	for (const auto& required: _requiredExtensions)
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

bool Phusis::Application::GLFWInitialize() noexcept
{
	if (!glfwInit())
		return false;
	return glfwVulkanSupported();
}

bool Phusis::Application::GLFWCreateWindow() noexcept
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	_window = glfwCreateWindow(640, 480, "Example Title", nullptr, nullptr);

	sys::log.head(sys::INFO) << "GLFW window created" << sys::EOM;

	return true;
}

bool Phusis::Application::GLFWCreateSurface() noexcept
{
	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(_instance, _window, nullptr, &surface);
	if (err)
	{
		sys::log.head(sys::CRIT) << "cannot create vulkan surface: " << err << sys::EOM;
		return false;
	}

	sys::log.head(sys::INFO) << "vulkan surface created" << sys::EOM;

	_surface = surface;
	return true;
}

bool Phusis::Application::VkValidateSwapchain() noexcept
{
	VkBool32 supported;
	vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, _queueFamilyIdx, _surface, &supported);
	if (!supported)
	{
		sys::log.head(sys::CRIT) << "selected physical device doesn't support surface" << sys::EOM;
		return false;
	}

	uint32_t cModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &cModes, nullptr);

	std::vector<VkPresentModeKHR> modes(cModes);
	vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &cModes, &modes[0]);

	static const std::array<std::string, 7> modeMap = {
			"VK_PRESENT_MODE_IMMEDIATE_KHR",
			"VK_PRESENT_MODE_MAILBOX_KHR",
			"VK_PRESENT_MODE_FIFO_KHR",
			"VK_PRESENT_MODE_FIFO_RELAXED_KHR",
			"VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR",
			"VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR",
			"VK_PRESENT_MODE_MAX_ENUM_KHR"
	};

	bool matched = false;
	for (const auto& mode : modes)
	{
		int idx;
		switch (mode)
		{
		case 1000111000:
			idx = 4;
			break;
		case 1000111001:
			idx = 5;
			break;
		case 0x7FFFFFFF:
			idx = 6;
			break;
		default:
			idx = mode;
			break;
		}

		if (mode == 2)
		{
			_presentMode = mode;
			matched = true;
		}

		sys::log.head(sys::VERB) << "SRF found: " << modeMap[idx] << sys::EOM;
	}

	if (!matched)
	{
		sys::log.head(sys::CRIT) << "SRF not matched: " << modeMap[2] << sys::EOM;
		return false;
	}
	else
	{
		sys::log.head(sys::INFO) << "SRF matched: " << modeMap[2] << sys::EOM;
	}

	return true;
}

bool Phusis::Application::VkInitializeSwapchain() noexcept
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &capabilities);

	uint32_t cFormat;
	vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &cFormat, nullptr);

	std::vector<VkSurfaceFormatKHR> formats(cFormat);
	vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &cFormat, &formats[0]);

	bool matched;
	VkSurfaceFormatKHR fmt;
	for (const auto& format : formats)
	{
		sys::log.head(sys::VERB) << "SRF-FMT found: " << format.format << sys::EOM;
		if (VK_FORMAT_B8G8R8A8_UNORM == format.format)
		{
			fmt = format;
			matched = true;
		}
	}
	if (matched)
	{
		sys::log.head(sys::INFO) << "SRF-FMT matched: " << fmt.format << sys::EOM;
	}
	else
	{
		sys::log.head(sys::CRIT) << "SRF-FMT not matched: " << fmt.format << sys::EOM;
		return false;
	}

	VkSwapchainCreateInfoKHR info{};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = _surface;
	info.minImageCount = 2;
	info.imageFormat = fmt.format;
	info.imageColorSpace = fmt.colorSpace;
	info.imageExtent = capabilities.currentExtent;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.preTransform = capabilities.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = _presentMode;

	VkSwapchainKHR swapchain;
	VkResult result = vkCreateSwapchainKHR(_device, &info, nullptr, &swapchain);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "cannot create vulkan swapchain" << sys::EOM;
		return false;
	}

	uint32_t cBuffer;
	vkGetSwapchainImagesKHR(_device, swapchain, &cBuffer, nullptr);

	if (cBuffer < 2)
	{
		sys::log.head(sys::CRIT) << "cannot retrieve swapchain buffers" << sys::EOM;
		return false;
	}

	std::vector<VkImage> buffers(cBuffer);
	vkGetSwapchainImagesKHR(_device, swapchain, &cBuffer, &buffers[0]);

	_swapchain = swapchain;
	_buffers = buffers;

	sys::log.head(sys::INFO) << "swapchain initialized" << sys::EOM;

	return true;
}

bool Phusis::Application::VkInitializeCommandPool() noexcept
{
	VkCommandPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	info.queueFamilyIndex = _queueFamilyIdx;

	VkCommandPool pool;
	VkResult result = vkCreateCommandPool(_device, &info, nullptr, &pool);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "cannot create vulkan command pool" << sys::EOM;
		return false;
	}

	_commandPool = pool;

	sys::log.head(sys::INFO) << "vulkan command pool created" << sys::EOM;

	return true;
}
