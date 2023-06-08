#ifndef PHUSIS_SPINLOCK_HXX
#define PHUSIS_SPINLOCK_HXX

#include "fw.hxx"

namespace sys
{
	class spinlock
	{
	private:
		std::atomic<bool> _lock{false};

	public:
		void lock() noexcept;
		bool trylock() noexcept;
		void unlock() noexcept;
	};
}


#endif //PHUSIS_SPINLOCK_HXX
