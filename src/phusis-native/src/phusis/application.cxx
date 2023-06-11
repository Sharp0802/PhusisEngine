#include <cstring>
#include "phusis/internal/constantblock.hxx"
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
	// do NOT use glfwGetRequiredInstanceExtensions : it's buggy and doesn't work
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
	ReleaseDeviceDependents();
	ReleaseDeviceIndependents();

	sys::log.head(sys::INFO) << "closing application; see you next time..." << sys::EOM;
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
	if (!_window)
	{
		sys::log.head(sys::CRIT) << "could not create GLFW window" << sys::EOM;
		return false;
	}

	sys::log.head(sys::INFO) << "GLFW window created" << sys::EOM;

	return true;
}

bool Phusis::Application::VkValidateLayer() noexcept
{
	uint32_t cProperty = 0;
	vkEnumerateInstanceLayerProperties(&cProperty, nullptr);

	std::vector<VkLayerProperties> properties(cProperty);
	vkEnumerateInstanceLayerProperties(&cProperty, &properties[0]);

	auto* layers = new std::string[cProperty];
	for (uint32_t i = 0; i < cProperty; ++i)
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

bool Phusis::Application::VkInitializeInstance() noexcept
{
	size_t cLayer = _requiredLayers.size();
	const char** layers = new const char* [cLayer];
	for (size_t i = 0; i < cLayer; ++i)
		layers[i] = _requiredLayers[i].c_str();

	size_t cExt = _requiredExtensions.size();
	const char** exts = new const char* [cExt];
	for (size_t i = 0; i < cExt; ++i)
		exts[i] = _requiredExtensions[i].c_str();

	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.enabledLayerCount = cLayer;
	create_info.ppEnabledLayerNames = layers;
	create_info.enabledExtensionCount = cExt;
	create_info.ppEnabledExtensionNames = exts;

	VkInstance instance;
	VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
	Instance = instance;

	return result == VK_SUCCESS;
}

bool Phusis::Application::VkInitializePhysicalDevice() noexcept
{
	uint32_t cDevice = 0;
	vkEnumeratePhysicalDevices(Instance, &cDevice, nullptr);

	std::vector<VkPhysicalDevice> physicalDevices(cDevice);
	vkEnumeratePhysicalDevices(Instance, &cDevice, &physicalDevices[0]);

	for (const auto& device: physicalDevices)
	{
		VkPhysicalDeviceProperties prop{};
		vkGetPhysicalDeviceProperties(device, &prop);

		sys::log.head(sys::VERB) << "GPU found: " << prop.deviceName << sys::EOM;
	}

	VkPhysicalDevice physicalDevice = physicalDevices[0];
	PhysicalDevice = physicalDevice;

	VkPhysicalDeviceProperties physicalProperties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalProperties);

	sys::log.head(sys::INFO) << "GPU using: " << physicalProperties.deviceName << sys::EOM;

	return true;
}

bool Phusis::Application::VkInitializeLogicalDevice() noexcept
{
	uint32_t cProperty = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &cProperty, nullptr);

	std::vector<VkQueueFamilyProperties> properties(cProperty);
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &cProperty, &properties[0]);

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
	vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &cExt, nullptr);

	std::vector<VkExtensionProperties> extensions(cExt);
	vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &cExt, &extensions[0]);

	bool failed = false;
	for (const auto& e: ext)
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
	VkResult result = vkCreateDevice(PhysicalDevice, &deviceCreateInfo, nullptr, &device);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "failed to create device" << sys::EOM;
		return false;
	}

	VkQueue queue;
	vkGetDeviceQueue(device, queueFamilyIdx, 0, &queue);

	Device = device;
	Queue = queue;
	_queueFamilyIdx = queueFamilyIdx;

	sys::log.head(sys::INFO) << "vulkan device & queue has been ready" << sys::EOM;

	return true;
}

bool Phusis::Application::VkCreateSurface() noexcept
{
	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(Instance, _window, nullptr, &surface);
	if (err)
	{
		sys::log.head(sys::CRIT) << "cannot create vulkan surface: " << err << sys::EOM;
		return false;
	}

	sys::log.head(sys::INFO) << "vulkan surface created" << sys::EOM;

	Surface = surface;
	return true;
}

bool Phusis::Application::VkValidateSwapchain() noexcept
{
	VkBool32 supported;
	vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, _queueFamilyIdx, Surface, &supported);
	if (!supported)
	{
		sys::log.head(sys::CRIT) << "selected physical device doesn't support surface" << sys::EOM;
		return false;
	}

	uint32_t cModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &cModes, nullptr);

	std::vector<VkPresentModeKHR> modes(cModes);
	vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &cModes, &modes[0]);

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
	for (const auto& mode: modes)
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

bool Phusis::Application::VkInitializeSurface() noexcept
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &capabilities);

	uint32_t cFormat;
	vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &cFormat, nullptr);

	std::vector<VkSurfaceFormatKHR> formats(cFormat);
	vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &cFormat, &formats[0]);

	bool matched;
	VkSurfaceFormatKHR fmt;
	for (const auto& format: formats)
	{
		sys::log.head(sys::VERB) << "SRF-FMT found: " << format.format << sys::EOM;
		if (VK_FORMAT_B8G8R8A8_UNORM == format.format)
		{
			fmt = format;
			matched = true;
		}
	}
	if (!matched)
	{
		sys::log.head(sys::CRIT) << "SRF-FMT not matched: " << fmt.format << sys::EOM;
		return false;
	}

	sys::log.head(sys::INFO) << "SRF-FMT matched: " << fmt.format << sys::EOM;
	_surfaceFormat = fmt;
	_surfaceCapabilities = capabilities;

	return true;
}

bool Phusis::Application::VkInitializeSwapchain() noexcept
{
	VkSwapchainCreateInfoKHR info{};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = Surface;
	info.minImageCount = 2;
	info.imageFormat = _surfaceFormat.format;
	info.imageColorSpace = _surfaceFormat.colorSpace;
	info.imageExtent = _surfaceCapabilities.currentExtent;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.preTransform = _surfaceCapabilities.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = _presentMode;

	_swapchainInfo = info;

	VkSwapchainKHR swapchain;
	VkResult result = vkCreateSwapchainKHR(Device, &info, nullptr, &swapchain);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "cannot create vulkan swapchain" << sys::EOM;
		return false;
	}

	Swapchain = swapchain;

	sys::log.head(sys::INFO) << "swapchain initialized" << sys::EOM;

	return true;
}

bool Phusis::Application::VkInitializeImageViews() noexcept
{
	uint32_t cBuffer;
	vkGetSwapchainImagesKHR(Device, Swapchain, &cBuffer, nullptr);

	if (cBuffer < 2)
	{
		sys::log.head(sys::CRIT) << "cannot retrieve swapchain buffers" << sys::EOM;
		return false;
	}

	std::vector<VkImage> buffers(cBuffer);
	vkGetSwapchainImagesKHR(Device, Swapchain, &cBuffer, &buffers[0]);

	std::vector<VkImageView> views(cBuffer);

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.format = _surfaceFormat.format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = _requiredLayers.size();
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	bool failed = false;
	for (uint32_t i = 0; i < cBuffer; ++i)
	{
		viewInfo.image = buffers[i];
		VkResult result = vkCreateImageView(Device, &viewInfo, nullptr, &views[i]);
		if (result != VK_SUCCESS)
		{
			failed = true;
			continue;
		}
	}
	if (failed)
	{
		sys::log.head(sys::CRIT) << "could not create image-view from image" << sys::EOM;
		return false;
	}

	sys::log.head(sys::INFO) << "image-view created from swapchain" << sys::EOM;

	_swapchainBuffers = buffers;
	_swapchainViews = views;

	return true;
}

bool Phusis::Application::VkInitializeCommandPool() noexcept
{
	VkCommandPoolCreateInfo primaryInfo{};
	primaryInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	primaryInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	primaryInfo.queueFamilyIndex = _queueFamilyIdx;

	VkCommandPool pool;
	VkResult result = vkCreateCommandPool(Device, &primaryInfo, nullptr, &pool);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "could not create primary command-pool" << sys::EOM;
		return false;
	}

	PrimaryCommandPool = pool;

	sys::log.head(sys::INFO) << "primary command-pool created" << sys::EOM;

	bool failed = false;
	for (auto& data: _threads)
	{
		VkCommandPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		info.queueFamilyIndex = _queueFamilyIdx;

		VkCommandPool localPool;
		VkResult localResult = vkCreateCommandPool(Device, &info, nullptr, &localPool);
		if (localResult != VK_SUCCESS)
		{
			sys::log.head(sys::FAIL) << "could not create secondary command-pool" << sys::EOM;
			failed = true;
			continue;
		}

		data.CommandPool = localPool;
	}
	if (failed)
	{
		sys::log.head(sys::CRIT) << "could not initialize secondary command-pool set" << sys::EOM;
		return false;
	}

	sys::log.head(sys::INFO) << "secondary command-pool created" << sys::EOM;

	return true;
}

bool Phusis::Application::VkInitializeCommandBuffer() noexcept
{
	VkCommandBufferAllocateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandBufferCount = 1;
	info.commandPool = PrimaryCommandPool;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer buffer;
	VkResult result = vkAllocateCommandBuffers(Device, &info, &buffer);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "could not create primary command-buffer" << sys::EOM;
		return false;
	}
	PrimaryCommandBuffer = buffer;

	bool failed = false;
	for (auto& data: _threads)
	{
		info.commandPool = data.CommandPool;
		info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

		result = vkAllocateCommandBuffers(Device, &info, &buffer);
		if (result != VK_SUCCESS)
		{
			sys::log.head(sys::FAIL) << "could not create secondary command-buffer" << sys::EOM;
			failed = true;
			continue;
		}

		data.CommandBuffer = buffer;
	}
	if (failed)
	{
		sys::log.head(sys::CRIT) << "could not initialize command-buffer set" << sys::EOM;
		return false;
	}

	sys::log.head(sys::INFO) << "command-buffer set initialized" << sys::EOM;

	return true;
}

bool Phusis::Application::VkInitializeRenderPass() noexcept
{
	std::array<VkFormat, 3> formats = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT
	};

	VkFormat depthFormat = VK_FORMAT_UNDEFINED;
	for (const auto& format : formats)
	{
		VkFormatProperties prop;
		vkGetPhysicalDeviceFormatProperties(PhysicalDevice, format, &prop);
		if (prop.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			depthFormat = format;
			sys::log.head(sys::INFO) << "stencil-depth format found: " << format << sys::EOM;
			break;
		}
	}
	if (depthFormat == VK_FORMAT_UNDEFINED)
	{
		sys::log.head(sys::CRIT) << "supported stencil-depth format not found" << sys::EOM;
		return false;
	}

	std::array<VkAttachmentDescription, 2> desc{};

	// color desc
	desc[0].format = _swapchainInfo.imageFormat;
	desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
	desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	desc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	desc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	desc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	desc[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// depth desc
	desc[0].format = depthFormat;
	desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
	desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	desc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	desc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	desc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	desc[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::array<VkAttachmentReference, 2> ref{};

	// color ref
	ref[0].attachment = 0;
	ref[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// depth ref
	ref[1].attachment = 0;
	ref[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &ref[0];
	subpass.pDepthStencilAttachment = &ref[1];
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;
	subpass.pResolveAttachments = nullptr;

	// for layout transitions
	std::array<VkSubpassDependency, 2> deps{};

	deps[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	deps[0].dstSubpass = 0;
	deps[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	deps[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	deps[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	deps[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	deps[0].dependencyFlags = 0;

	deps[1].srcSubpass = VK_SUBPASS_EXTERNAL;
	deps[1].dstSubpass = 0;
	deps[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	deps[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	deps[1].srcAccessMask = 0;
	deps[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	deps[1].dependencyFlags = 0;

	VkRenderPassCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = desc.size();
	info.pAttachments = desc.data();
	info.subpassCount = 1;
	info.pSubpasses = &subpass;
	info.dependencyCount = deps.size();
	info.pDependencies = deps.data();

	_defaultRenderPassInfo = info;

	VkRenderPass renderpass;
	VkResult result = vkCreateRenderPass(Device, &info, nullptr, &renderpass);
	RenderPass = renderpass;

	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "could not create default render-pass" << sys::EOM;
		return false;
	}

	sys::log.head(sys::INFO) << "default render-pass created" << sys::EOM;

	return true;
}

bool Phusis::Application::VkInitializeFramebuffers() noexcept
{
	int32_t w, h;
	glfwGetWindowSize(_window, &w, &h);

	VkFramebufferCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.pNext = nullptr;
	info.renderPass = RenderPass;
	info.attachmentCount = _defaultRenderPassInfo.attachmentCount;
	info.width = w;
	info.height = h;
	info.layers = _requiredLayers.size();

	std::vector<VkFramebuffer> framebuffers{_swapchainBuffers.size()};

	bool failed = false;
	for (uint32_t i = 0; i < _swapchainBuffers.size(); ++i)
	{
		info.pAttachments = &_swapchainViews[i];
		VkResult result = vkCreateFramebuffer(Device, &info, nullptr, &framebuffers[i]);
		if (result != VK_SUCCESS)
		{
			sys::log.head(sys::FAIL) << "could not create framebuffer" << sys::EOM;
			failed = true;
		}
	}
	if (failed)
	{
		sys::log.head(sys::CRIT) << "could not create framebuffer set" << sys::EOM;
		return false;
	}

	_framebuffers = framebuffers;

	sys::log.head(sys::INFO) << "framebuffer set created" << sys::EOM;

	return true;
}

bool Phusis::Application::UpdateCommandBuffers(VkFramebuffer buffer) noexcept
{
	int32_t w, h;
	glfwGetWindowSize(_window, &w, &h);

	VkCommandBufferBeginInfo bufInfo{};
	bufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkClearColorValue color{};
	color.uint32[0] = 1; // B
	color.uint32[1] = 1; // G
	color.uint32[2] = 1; // R
	color.uint32[3] = 0; // A

	VkClearValue clear[2];
	clear[0].color = color;
	clear[1].depthStencil = {1.0f, 0};

	VkRenderPassBeginInfo renderpassInfo{};
	renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassInfo.renderPass = RenderPass;
	renderpassInfo.renderArea.offset.x = 0;
	renderpassInfo.renderArea.offset.y = 0;
	renderpassInfo.renderArea.extent.width = w;
	renderpassInfo.renderArea.extent.height = h;
	renderpassInfo.clearValueCount = 2;
	renderpassInfo.pClearValues = clear;
	renderpassInfo.framebuffer = buffer;

	vkBeginCommandBuffer(PrimaryCommandBuffer, &bufInfo);
	vkCmdBeginRenderPass(PrimaryCommandBuffer, &renderpassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	VkCommandBufferInheritanceInfo info{};
	info.renderPass = RenderPass;
	info.framebuffer = buffer;

	// update secondary
	// render thread

	std::vector<VkCommandBuffer> buffers{_threads.size()};
	for (uint32_t i = 0; i < _threads.size(); ++i)
		buffers[i] = _threads[i].CommandBuffer;

	vkCmdExecuteCommands(PrimaryCommandBuffer, buffers.size(), buffers.data());
	vkCmdEndRenderPass(PrimaryCommandBuffer);

	VkResult result = vkEndCommandBuffer(PrimaryCommandBuffer);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::WARN) << "could not end primary command-buffer; output can be incorrect" << sys::EOM;
		return false;
	}

	sys::log.head(sys::INFO) << "command-buffer updated" << sys::EOM;
	return false;
}

bool Phusis::Application::VkInitializePipelineLayout() noexcept
{
	VkPushConstantRange range{};
	range.size = sizeof(Phusis::Internal::ConstantBlock);
	range.offset = 0;
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.setLayoutCount = 0;
	info.pSetLayouts = nullptr;
	info.pushConstantRangeCount = 1;
	info.pPushConstantRanges = &range;

	VkPipelineLayout layout;
	VkResult result = vkCreatePipelineLayout(Device, &info, nullptr, &layout);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "could not create vulkan pipeline layout" << sys::EOM; 
	}

	PipelineLayout = layout;

	return false;
}

int32_t Phusis::Application::InitializeDeviceDependents() noexcept
{
	sys::log.head(sys::INFO) << "initializing device-dependants" << sys::EOM;

	if (!VkCreateSurface())
		return 8;
	if (!VkValidateSwapchain())
		return 9;
	if (!VkInitializeSurface())
		return 10;
	if (!VkInitializeSwapchain())
		return 11;
	if (!VkInitializeImageViews())
		return 12;
	if (!VkInitializeCommandPool())
		return 13;
	if (!VkInitializeCommandBuffer())
		return 14;
	if (!VkInitializeRenderPass())
		return 15;
	if (!VkInitializeFramebuffers())
		return 16;
	if (!VkInitializePipelineLayout())
		return 17;

	sys::log.head(sys::INFO) << "" << sys::EOM;

	return 0;
}

void Phusis::Application::ReleaseDeviceDependents() noexcept
{
	sys::log.head(sys::INFO) << "clean up device-dependent resources..." << sys::EOM;

	for (const auto& buffer : _framebuffers)
		vkDestroyFramebuffer(Device, buffer, nullptr);
	vkDestroyRenderPass(Device, RenderPass, nullptr);
	for (const auto& data: _threads)
		vkDestroyCommandPool(Device, data.CommandPool, nullptr);
	vkDestroyCommandPool(Device, PrimaryCommandPool, nullptr);
	for (const auto& view : _swapchainViews)
		vkDestroyImageView(Device, view, nullptr);
	vkDestroySwapchainKHR(Device, Swapchain, nullptr);
	vkDestroySurfaceKHR(Instance, Surface, nullptr);
}

void Phusis::Application::ReleaseDeviceIndependents() noexcept
{
	sys::log.head(sys::INFO) << "clean up device-independent resources..." << sys::EOM;

	vkDestroyDevice(Device, nullptr);
	vkDestroyInstance(Instance, nullptr);

	glfwDestroyWindow(_window);
	glfwTerminate();
}

int32_t Phusis::Application::InitializeComponents() noexcept
{
	sys::log.head(sys::DBUG) << "\n=== SYSTEM CONFIGURATION ===\n"
							 << "Hardware Concurrency : " << _threads.size() << "\n"
							 << sys::EOM;

	if (!GLFWInitialize())
		return 1;
	if (!GLFWCreateWindow())
		return 2;
	if (!VkValidateLayer())
		return 3;
	if (!VkValidateExtension())
		return 4;
	if (!VkInitializeInstance())
		return 5;
	if (!VkInitializePhysicalDevice())
		return 6;
	if (!VkInitializeLogicalDevice())
		return 7;

	int32_t r = InitializeDeviceDependents();
	if (r)
		return r;

	sys::log.head(sys::INFO) << "complete operation successfully" << sys::EOM;

	return 0;
}

int32_t Phusis::Application::Run() noexcept
{
	while (!glfwWindowShouldClose(_window))
	{
		glfwPollEvents();
	}

	return 0;
}


