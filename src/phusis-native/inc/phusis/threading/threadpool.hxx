#ifndef PHUSIS_THREADPOOL_HXX
#define PHUSIS_THREADPOOL_HXX

#include "fw.hxx"
#include "sys/spinlock.hxx"

namespace Phusis::Threading
{
	class ThreadPool
	{
	public:
		using callable = std::function<void(uint32_t tid)>;
		using batchable = std::function<void(uint32_t tid, size_t jid)>;

	private:
		inline static volatile int _idx;
		inline static volatile bool _exit;
		inline static std::vector<sys::spinlock> _local;
		inline static std::vector<std::thread> _workers;
		inline static std::vector<std::deque<callable>> _queue;

	public:
		CCTOR;

		static void Submit(const callable& callable) noexcept;
		static void Submit(const batchable& batchable, size_t batchSize);
	};
}

#endif //PHUSIS_THREADPOOL_HXX
