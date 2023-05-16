#ifndef PHUSIS_LOCKABLE_HXX
#define PHUSIS_LOCKABLE_HXX

#include "fw.hxx"

namespace sys
{
	class lockable
	{
	private:
		std::atomic<bool> _lock{false};

	public:
		void lock() noexcept;
		bool trylock() noexcept;
		void unlock() noexcept;
	};
}


#endif //PHUSIS_LOCKABLE_HXX
