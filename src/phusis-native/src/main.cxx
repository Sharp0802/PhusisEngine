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

	int32_t r = app.InitializeComponents();
	if (r)
		return r;
	r = app.Run();
	return r;
}
