#ifndef PHUSIS_OS_HXX
#define PHUSIS_OS_HXX

#include "fw.hxx"

namespace sys
{
	using oskind = std::uint8_t;

	class os
	{
	public:
		enum : oskind
		{
			LINUX,
			OSX,
			WIN,
			OTHER,
			__N_ITEMS
		};

	public:
		static const oskind kind =
#if __linux__
				LINUX
#elif defined(__APPLE__) || defined(__MACH__)
				OSX
#elif defined(_WIN32) || defined(_WIN64)
				WIN
#else
				OTHER
#endif
		;

		static constexpr const char* surfaces[__N_ITEMS] = {
				"VK_KHR_xcb_surface",
				"VK_MVK_macos_surface",
				"VK_KHR_win32_surface",
				nullptr
		};

		static constexpr const char* surface = surfaces[kind];
	};
}

#endif //PHUSIS_OS_HXX
