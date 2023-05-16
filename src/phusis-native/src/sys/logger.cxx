#include "sys/logger.hxx"

static sys::lockable _lock;

sys::loggerctx sys::logger::__head(loggerctrl ctrl, const char* file, int32_t line)
{
	constexpr const char* const head[] = {
			"NONE",
			"CRIT",
			"FAIL",
			"WARN",
			"INFO",
			"DBUG",
	};

	constexpr const char* const color[] = {
			"\033[37m",
			"\033[1m\033[31m",
			"\033[31m",
			"\033[33m",
			"\033[37m",
			"\033[36m"
	};

	time_t now = time(nullptr);
	struct tm tm = *localtime(&now);
	char buf[128];
	strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%S%z", &tm);

	const std::filesystem::path base(SRCDIR);
	const std::filesystem::path dir(file);

	sys::loggerctx ctx{};
	ctx << color[ctrl]
		<< "[" << buf
		<< "|" << getpid()
		<< "|" << head[ctrl]
		<< "|" << std::filesystem::relative(dir, base).generic_string()
		<< "," << line
		<< "] ";

	return ctx;
}

sys::loggerctx::loggerctx() noexcept : _disposed(false), _buffer()
{

}

void sys::loggerctx::flush() noexcept
{
	_buffer << "\033[0m";
	_lock.lock();
	std::cout << _buffer.str();
	_lock.unlock();
	_buffer.str(std::string());
}

sys::loggerctx& sys::loggerctx::operator<<(const std::string& str) noexcept
{
	if (!_disposed)
		_buffer << str;
	return *this;
}

sys::loggerctx& sys::loggerctx::operator<<(const char* cstr) noexcept
{
	if (!_disposed)
		_buffer << cstr;
	return *this;
}

sys::loggerctx& sys::loggerctx::operator<<(char ch) noexcept
{
	if (ch == 0)
	{
		_buffer << '\n';
		flush();
		_buffer.str(std::string());
		_disposed = true;
		return *this;
	}

	if (!_disposed)
		_buffer << ch;
	return *this;
}

sys::loggerctx& sys::loggerctx::operator<<(int64_t i) noexcept
{
	if (!_disposed)
		_buffer << i;
	return *this;
}

sys::loggerctx& sys::loggerctx::operator<<(int32_t i) noexcept
{
	*this << (int64_t)i;
	return *this;
}

sys::loggerctx& sys::loggerctx::operator<<(int16_t i) noexcept
{
	*this << (int64_t)i;
	return *this;
}

sys::loggerctx& sys::loggerctx::operator<<(uint64_t i) noexcept
{
	if (!_disposed)
		_buffer << i;
	return *this;
}

sys::loggerctx& sys::loggerctx::operator<<(uint32_t i) noexcept
{
	*this << (uint64_t)i;
	return *this;
}

sys::loggerctx& sys::loggerctx::operator<<(uint16_t i) noexcept
{
	*this << (uint64_t)i;
	return *this;
}

sys::loggerctx& sys::loggerctx::operator<<(long double f) noexcept
{
	if (!_disposed)
		_buffer << f;
	return *this;
}

sys::loggerctx& sys::loggerctx::operator<<(double f) noexcept
{
	*this << (long double)f;
	return *this;
}

sys::loggerctx& sys::loggerctx::operator<<(float f) noexcept
{
	*this << (long double)f;
	return *this;
}

sys::loggerctx& sys::loggerctx::operator<<(bool b) noexcept
{
	if (!_disposed)
		_buffer << b;
	return *this;
}
