#include "phusis/threading/renderjob.hxx"

bool Phusis::Threading::RenderJob::Dispatch(size_t size) noexcept
{
	if (!_lock.trylock())
		return false;

	auto* pc = new size_t;
	*pc = 0;
	ThreadPool::Submit([this, pc, size](uint32_t tid, size_t jid)
	{
		Phusis::Internal::CommandBuffer buffer = _buffers[tid];
		// TODO : load object & submit buffer
		_callable(buffer, jid);
		if (__sync_add_and_fetch(pc, 1) == size)
		{
			_lock.unlock();
			delete pc;
		}
	}, size);

	return true;
}

void Phusis::Threading::RenderJob::Wait() noexcept
{
	_lock.lock();
	_lock.unlock();
}
