#include "sys/spinlock.hxx"

/*
 * spinlock implementation by Erik Rigtorp
 * https://rigtorp.se/spinlock/
 */

void sys::spinlock::lock() noexcept
{
	while (true)
	{
		if (!_lock.exchange(true, std::memory_order_acquire))
			break;
		while (_lock.load(std::memory_order_relaxed))
			__builtin_ia32_pause();
	}
}

bool sys::spinlock::trylock() noexcept
{
	return !_lock.load(std::memory_order_relaxed) &&
		   !_lock.exchange(true, std::memory_order_acquire);
}

void sys::spinlock::unlock() noexcept
{
	_lock.store(false, std::memory_order_release);
}
