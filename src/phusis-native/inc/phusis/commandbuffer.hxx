#ifndef PHUSIS_COMMANDBUFFER_HXX
#define PHUSIS_COMMANDBUFFER_HXX

#include <utility>

#include "fw.hxx"
#include "application.hxx"
#include "buffer.hxx"
#include "mesh.hxx"
#include "engineobject.hxx"

namespace Phusis
{
	class CommandBuffer
	{
	private:
		const uint32_t _width, _height;
		const ObjectData& _data;
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
				VkCommandBufferInheritanceInfo inheritance,
				const ObjectData& data) noexcept
				: _width(app.Width),
				  _height(app.Height),
				  _data(data),
				  _projection(app.Projection),
				  _view(app.View),
				  _inheritance(inheritance),
				  _buffer(buffer),
				  _pipeline(app.Pipeline),
				  _layout(app.PipelineLayout)
		{
		}

		bool Update() noexcept;
	};
}

#endif //PHUSIS_COMMANDBUFFER_HXX
