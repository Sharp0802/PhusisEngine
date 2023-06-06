#include "phusis/commandbuffer.hxx"
#include "sys/logger.hxx"
#include "phusis/constantblock.hxx"

bool Phusis::CommandBuffer::Update() noexcept
{
	if (!_data.enabled)
		return true;

	VkResult result;

	VkCommandBufferBeginInfo begin{};
	begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	begin.pInheritanceInfo = &_inheritance;

	result = vkBeginCommandBuffer(_buffer, &begin);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::FAIL) << "could not begin command-buffer; render result may be wrong" << sys::EOM;
		return false;
	}

	VkViewport viewport{};
	viewport.width = static_cast<decltype(viewport.width)>(_width);
	viewport.height = static_cast<decltype(viewport.height)>(_height);
	viewport.maxDepth = 1.f;
	viewport.minDepth = 0.f;
	viewport.x = 0.f;
	viewport.y = 0.f;

	VkRect2D scissor{};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = _width;
	scissor.extent.height = _height;

	vkCmdSetViewport(_buffer, 0, 1, &viewport);
	vkCmdSetScissor(_buffer, 0, 1, &scissor);
	vkCmdBindPipeline(_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

	ConstantBlock blk(_projection * _view * _data.Rotation, _data.Color);
	vkCmdPushConstants(_buffer, _layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ConstantBlock), &blk);

	std::vector<VkDeviceSize> offsets(_data.Mesh.Vertices.size());
	offsets.assign(offsets.size(), 0);

	std::vector<VkBuffer> buffers(_data.Mesh.Vertices.size());
	for (size_t i = 0; i < buffers.size(); ++i)
		buffers[i] = _data.Mesh.Vertices[i].Array;

	vkCmdBindVertexBuffers(_buffer, 0, buffers.size(), buffers.data(), offsets.data());

	for (size_t i = 0; i < buffers.size(); ++i)
	{
		vkCmdBindIndexBuffer(_buffer, buffers[i], 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(_buffer, _data.Mesh.Indices[i].Count, 1, 0, 0, 0);
	}

	result = vkEndCommandBuffer(_buffer);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::FAIL) << "could not end command-buffer; render result may be wrong" << sys::EOM;
		return false;
	}

	return true;
}
