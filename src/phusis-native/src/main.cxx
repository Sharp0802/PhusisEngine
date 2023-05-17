#include "fw.hxx"
#include "phusis/application.hxx"
#include "sys/logger.hxx"
#include "sys/os.hxx"

void vk_main();

int32_t main() noexcept
{
	try
	{
		vk_main();
		return 0;
	}
	catch (std::exception& exception)
	{
		std::cerr << "====== EXCEPTION ======" << '\n'
				  << exception.what() << '\n'
				  << "=======================" << '\n';
		return -1;
	}
}

void vk_main()
{
	const std::vector<std::string> layers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<std::string> exts = {};

	Phusis::Application app(layers, exts, Phusis::ApplicationMode::Quality);
	if (!app.VkValidateLayer())
		return;
	if (!app.VkValidateExtension())
		return;
	if (!app.VkInitializeInstance())
		return;
	if (!app.VkInitializePhysicalDevice())
		return;
	if (!app.VkInitializeDeviceQueue())
		return;

	app.~Application();

	sys::log.head(sys::INFO) << "complete operation successfully" << sys::EOM;
}
