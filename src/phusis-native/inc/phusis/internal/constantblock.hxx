#ifndef PHUSIS_CONSTANTBLOCK_HXX
#define PHUSIS_CONSTANTBLOCK_HXX

#include "fw.hxx"

namespace Phusis::Internal
{
	struct ConstantBlock
	{
		const glm::mat4 MVP;
		const glm::vec4 Color;

		explicit ConstantBlock(glm::mat4 mvp, glm::vec4 color) noexcept
				: MVP(mvp), Color(color)
		{
		}
	};
}

#endif //PHUSIS_CONSTANTBLOCK_HXX
