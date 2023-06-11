#ifndef PHUSIS_RENDERJOB_HXX
#define PHUSIS_RENDERJOB_HXX

#include <utility>

#include "fw.hxx"
#include "phusis/internal/commandbuffer.hxx"
#include "threadpool.hxx"

namespace Phusis::Threading
{
	class RenderJob
	{
	public:
		using callable = std::function<void(const Phusis::Internal::CommandBuffer& buffer, size_t jid)>;

	private:
		std::vector<Phusis::Internal::CommandBuffer> _buffers;
		callable _callable;
		sys::spinlock _lock;

	public:
		explicit RenderJob(callable callable) : _callable(std::move(callable))
		{
		}

		bool Dispatch(size_t size) noexcept;

		void Wait() noexcept;
	};
}

#endif //PHUSIS_RENDERJOB_HXX
