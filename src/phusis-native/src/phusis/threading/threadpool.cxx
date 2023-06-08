#include "phusis/threading/threadpool.hxx"
#include "sys/logger.hxx"

Phusis::Threading::ThreadPool::cctor::cctor()
{
	size_t size = std::thread::hardware_concurrency();
	size -= size != 0;

	_queue = std::vector<std::deque<callable>>(size);
	_workers = std::vector<std::thread>(size);
	for (size_t i = 0; i < size; ++i)
	{
		_queue[i] = std::deque<callable>(std::max(512 / size, static_cast<size_t>(16)));
		_workers[i] = std::thread([i] {
			sys::log.head(sys::INFO) << "Job worker #" << i << " ready";
			while (!_exit)
			{
				_local[i].lock();

				callable* lambda = nullptr;
				if (!_queue[i].empty())
				{
					lambda = &_queue[i].front();
					_queue[i].pop_front();
				}

				_local[i].unlock();

				if (lambda != nullptr)
					(*lambda)();
			}
		});
	}
}

void Phusis::Threading::ThreadPool::Submit(const callable& callable) noexcept
{
	size_t id = __sync_fetch_and_add(&_idx, 1) % _workers.size();

	_local[id].lock();
	_queue[id].push_back(callable);
	_local[id].unlock();
}

void Phusis::Threading::ThreadPool::Submit(
		const Phusis::Threading::ThreadPool::batchable& batchable,
		size_t batchSize)
{
	if (batchSize <= _workers.size())
	{
		for (size_t i = 0; i < batchSize; ++i)
		{
			_local[i].lock();
			_queue[i].emplace_back([batchable, i] {
				batchable(i);
			});
			_local[i].unlock();
		}
	}
	else
	{
		for (size_t i = 0; i < _workers.size(); ++i)
		{
			_local[i].lock();
			_queue[i].emplace_back([batchable, batchSize, i] {
				size_t size = _workers.size();
				size_t cnt = batchSize / size + (batchSize % size ? 1 : 0);
				size_t offset = batchSize / size * i + std::min(batchSize % size, i);
				for (size_t j = 0; j < cnt; ++j)
					batchable(offset + j);
			});
			_local[i].unlock();
		}
	}
}
