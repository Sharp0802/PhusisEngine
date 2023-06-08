#ifndef PHUSIS_LOGGER_HXX
#define PHUSIS_LOGGER_HXX

#include "fw.hxx"
#include "spinlock.hxx"

namespace sys
{
	enum loggerctrl : char
	{
		CRIT = 1,
		FAIL = 2,
		WARN = 3,
		INFO = 4,
		VERB = 5,
		DBUG = 6,
		EOM = 0
	};

	class loggerctx
	{
	private:
		bool _disposed;
		std::stringstream _buffer;

	public:
		loggerctx() noexcept;

	private:
		void flush() noexcept;

	public:
		loggerctx& operator<<(const std::string& str) noexcept;

		loggerctx& operator<<(const char* cstr) noexcept;

		loggerctx& operator<<(char ch) noexcept;

		loggerctx& operator<<(int64_t i) noexcept;

		loggerctx& operator<<(int32_t i) noexcept;

		loggerctx& operator<<(int16_t i) noexcept;

		loggerctx& operator<<(uint64_t i) noexcept;

		loggerctx& operator<<(uint32_t i) noexcept;

		loggerctx& operator<<(uint16_t i) noexcept;

		loggerctx& operator<<(long double f) noexcept;

		loggerctx& operator<<(double f) noexcept;

		loggerctx& operator<<(float f) noexcept;

		loggerctx& operator<<(bool b) noexcept;
	};

	class logger
	{
	public:
		loggerctx __head(loggerctrl ctrl, const char* file, int32_t line);
	};

	[[maybe_unused]]
	static logger log;
}

#define head(ctrl) __head(ctrl, __FILE__, __LINE__)

#endif //PHUSIS_LOGGER_HXX
