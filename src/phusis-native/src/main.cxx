#include "fw.hxx"
#include "phusis/application.hxx"
#include "sys/logger.hxx"
#include "sys/os.hxx"

int32_t vk_main();

int32_t main() noexcept
{
	try
	{
		return vk_main();
	}
	catch (std::exception& exception)
	{
		std::cerr << "====== EXCEPTION ======" << '\n'
				  << exception.what() << '\n'
				  << "=======================" << '\n';
		return -1;
	}
}

int32_t vk_main()
{
	const std::vector<std::string> layers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<std::string> exts = {};

	Phusis::Application app(layers, exts, Phusis::ApplicationMode::Quality);
	if (!app.VkValidateLayer())
		return 1;
	if (!app.VkValidateExtension())
		return 2;
	if (!app.VkInitializeInstance())
		return 3;
	if (!app.VkInitializePhysicalDevice())
		return 4;
	if (!app.VkInitializeDeviceQueue())
		return 5;
	if (!app.GLFWInitialize())
		return 6;
	if (!app.GLFWCreateWindow())
		return 7;
	if (!app.GLFWCreateSurface())
		return 8;
	if (!app.VkValidateSwapchain())
		return 9;
	if (!app.VkInitializeSwapchain())
		return 10;
	if (!app.VkInitializeCommandPool())
		return 11;

	sys::log.head(sys::INFO) << "complete operation successfully" << sys::EOM;

	return 0;
}
