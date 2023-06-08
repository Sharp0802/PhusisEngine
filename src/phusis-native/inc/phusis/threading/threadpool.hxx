#ifndef PHUSIS_THREADPOOL_HXX
#define PHUSIS_THREADPOOL_HXX

#include "fw.hxx"
#include "sys/spinlock.hxx"

namespace Phusis::Threading
{
	class ThreadPool
	{
	public:
		using callable = std::function<void()>;
		using batchable = std::function<void(size_t)>;

	private:
		static volatile int _idx;
		static volatile bool _exit;
		static std::vector<sys::spinlock> _local;
		static std::vector<std::thread> _workers;
		static std::vector<std::deque<callable>> _queue;

	public:
		CCTOR;

		static void Submit(const callable& callable) noexcept;
		static void Submit(const batchable& batchable, size_t batchSize);
	};
}

#endif //PHUSIS_THREADPOOL_HXX
