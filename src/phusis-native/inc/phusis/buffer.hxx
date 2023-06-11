#ifndef PHUSIS_BUFFER_HXX
#define PHUSIS_BUFFER_HXX

#include "fw.hxx"

namespace Phusis
{
	struct Buffer
	{
		const VkBuffer Array;
		const uint32_t Count;

		explicit Buffer(VkBuffer arr, uint32_t cnt) noexcept
				: Array(arr), Count(cnt)
		{
		}
	};
}

#endif //PHUSIS_BUFFER_HXX
