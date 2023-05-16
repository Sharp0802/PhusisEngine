#include "sys/lockable.hxx"

/*
 * spinlock implementation by Erik Rigtorp
 * https://rigtorp.se/spinlock/
 */

void sys::lockable::lock() noexcept
{
	while (true)
	{
		if (!_lock.exchange(true, std::memory_order_acquire))
			break;
		while (_lock.load(std::memory_order_relaxed))
			__builtin_ia32_pause();
	}
}

bool sys::lockable::trylock() noexcept
{
	return !_lock.load(std::memory_order_relaxed) &&
		   !_lock.exchange(true, std::memory_order_acquire);
}

void sys::lockable::unlock() noexcept
{
	_lock.store(false, std::memory_order_release);
}
