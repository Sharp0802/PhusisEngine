#ifndef PHUSIS_COMMANDBUFFER_HXX
#define PHUSIS_COMMANDBUFFER_HXX

#include <utility>

#include "fw.hxx"
#include "phusis/application.hxx"
#include "phusis/buffer.hxx"
#include "phusis/mesh.hxx"
#include "phusis/engineobject.hxx"

namespace Phusis::Internal
{
	class CommandBuffer
	{
	private:
		EngineObjectData* _data = nullptr;

		const uint32_t *_width, *_height;
		const glm::mat4& _projection;
		const glm::mat4& _view;

		const VkCommandBufferInheritanceInfo _inheritance;

		VkCommandBuffer _buffer;
		VkPipeline _pipeline;
		VkPipelineLayout _layout;

	public:
		explicit CommandBuffer(
				const Application& app,
				VkCommandBuffer buffer,
				VkCommandBufferInheritanceInfo inheritance) noexcept
				: _width(&app.Width),
				  _height(&app.Height),
				  _projection(app.Projection),
				  _view(app.View),
				  _inheritance(inheritance),
				  _buffer(buffer),
				  _pipeline(app.Pipeline),
				  _layout(app.PipelineLayout)
		{
		}

	public:
		void Load(EngineObjectData* data) noexcept;
		bool Update() noexcept;
	};
}

#endif //PHUSIS_COMMANDBUFFER_HXX
