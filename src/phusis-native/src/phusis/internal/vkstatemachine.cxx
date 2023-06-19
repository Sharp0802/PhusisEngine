#include "phusis/internal/vkstatemachine.hxx"
#include "sys/logger.hxx"
#include "phusis/internal/constantblock.hxx"

bool Phusis::Internal::VkStateMachine::PrepareSecondaryPools()
{
	bool failed = false;
	for (auto& data: _threads)
	{
		VkCommandPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		info.queueFamilyIndex = _inheritance.QueueIdx;

		VkCommandPool pool;
		VkResult result = vkCreateCommandPool(_inheritance.Device, &info, nullptr, &pool);
		if (result != VK_SUCCESS)
		{
			sys::log.head(sys::FAIL) << "could not create secondary command pool" << sys::EOM;
			data.Pool = nullptr;
			failed = true;
		}
		else
		{
			data.Pool = pool;
		}
	}

	if (failed)
	{
		sys::log.head(sys::CRIT) << "could not create secondary command pool" << sys::EOM;
	}

	return !failed;
}

bool Phusis::Internal::VkStateMachine::PrepareCommandBuffers(uint32_t idx, uint32_t n)
{
	VkResult result;

	VkThreadData& data = _threads[idx];
	if (data.Buffers.size() == n)
		return true;

	result = vkResetCommandPool(_inheritance.Device, data.Pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "could not reset command pool on thread " << idx << sys::EOM;
		return false;
	}

	VkCommandBufferAllocateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandBufferCount = n;
	info.commandPool = data.Pool;
	info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

	std::vector<VkCommandBuffer> buffers{ n };
	result = vkAllocateCommandBuffers(_inheritance.Device, &info, buffers.data());
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "could not create buffer on thread " << idx << sys::EOM;
		return false;
	}

	return true;
}

bool Phusis::Internal::VkStateMachine::DistributeBuffers(
		int32_t update,
		Phusis::Internal::BufferDistributionStrategy strategy)
{
	bool result = true;
	switch (strategy)
	{
	case Phusis::Internal::BufferDistributionStrategy::Optimal:
	{
		if (update > 0)
		{
			// find minimal
			size_t x = -1, xi;
			for (size_t i = 0; i < _threads.size(); ++i)
			{
				size_t loc = _threads[i].Buffers.size();
				if (x >= loc)
					continue;
				x = loc;
				xi = i;
			}
			result = PrepareCommandBuffers(xi, x + update);
		}
		else if (update < 0)
		{
			// find maximal
			size_t x = -1, xi;
			for (size_t i = 0; i < _threads.size(); ++i)
			{
				size_t loc = _threads[i].Buffers.size();
				if (x > loc)
					continue;
				x = loc;
				xi = i;
			}
			result = PrepareCommandBuffers(xi, x + update);
		}
		break;
	}

	case Phusis::Internal::BufferDistributionStrategy::Uniform:
	{
		uint32_t total = _knownTargetCount + update;
		uint32_t count = total / _threads.size();
		for (size_t i = 0; i < _threads.size(); ++i)
		{
			uint32_t local = count;
			if (i < total % _threads.size())
				local++;
			result &= PrepareCommandBuffers(i, local);
		}
		break;
	}

	default:
		sys::log.head(sys::CRIT) << "could not distribute buffers" << sys::EOM;
		result = false;
		break;
	}

	if (result)
		_knownTargetCount += update;

	return result;
}

bool Phusis::Internal::VkStateMachine::BatchBuffer()
{
	std::vector<uint32_t> indices(_threads.size());
	for (size_t i = 0; i < indices.size(); ++i)
		indices[i] = i;

	bool result = true;
	std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [this, &result](uint32_t i)
	{
		uint32_t local = _bound->Objects.size() / _threads.size();
		uint32_t remain = _bound->Objects.size() % _threads.size();
		uint32_t offset = local * i + std::min(i, remain);
		if (i < remain)
			local++;

		bool loc = BatchBufferLocal(i, offset, local);
		__sync_and_and_fetch(&result, loc);
	});
	return result;
}

bool Phusis::Internal::VkStateMachine::BatchBufferLocal(uint32_t idx, uint32_t offset, uint32_t size)
{
	bool result = true;
	for (uint32_t i = 0; i < size; ++i)
	{
		EngineObjectData object = _bound->Objects[offset + i];
		if (!object.enabled)
			continue;

		VkCommandBuffer buffer = _threads[idx].Buffers[i];

		VkResult vkr;

		VkCommandBufferBeginInfo begin{};
		begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		begin.pInheritanceInfo = &_local.Inheritance;

		vkr = vkBeginCommandBuffer(buffer, &begin);
		if (vkr != VK_SUCCESS)
		{
			sys::log.head(sys::FAIL) << "could not begin command-buffer; render vkr may be wrong" << sys::EOM;
			result = false;
			continue;
		}

		VkViewport viewport{};
		viewport.width = static_cast<float>(_bound->Width);
		viewport.height = static_cast<float>(_bound->Height);
		viewport.maxDepth = 1.f;
		viewport.minDepth = 0.f;
		viewport.x = 0.f;
		viewport.y = 0.f;

		VkRect2D scissor{};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = _bound->Width;
		scissor.extent.height = _bound->Height;

		vkCmdSetViewport(buffer, 0, 1, &viewport);
		vkCmdSetScissor(buffer, 0, 1, &scissor);
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _inheritance.Pipeline);

		ConstantBlock blk(_bound->Projection * _bound->View * object.Rotation, object.Color);
		vkCmdPushConstants(buffer, _inheritance.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ConstantBlock), &blk);

		std::vector<VkDeviceSize> offsets(object.Mesh.Vertices.size());
		offsets.assign(offsets.size(), 0);

		std::vector<VkBuffer> buffers(object.Mesh.Vertices.size());
		for (size_t j = 0; j < buffers.size(); ++j)
			buffers[j] = object.Mesh.Vertices[j].Array;

		vkCmdBindVertexBuffers(buffer, 0, buffers.size(), buffers.data(), offsets.data());

		for (size_t j = 0; j < buffers.size(); ++j)
		{
			vkCmdBindIndexBuffer(buffer, buffers[j], 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(buffer, object.Mesh.Indices[j].Count, 1, 0, 0, 0);
		}

		result = vkEndCommandBuffer(buffer);
		if (result != VK_SUCCESS)
		{
			sys::log.head(sys::FAIL) << "could not end command-buffer; render result may be wrong" << sys::EOM;
			result = false;
			continue;
		}
	}
	return result;
}

bool Phusis::Internal::VkStateMachine::BeginDraw()
{
	VkResult result;

	VkClearValue clears[2];
	clears[0].color = _inheritance.ClearColor;
	clears[1].depthStencil = {1.f, 0};

	VkRenderPassBeginInfo pass{};
	pass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	pass.renderPass = _inheritance.RenderPass;
	pass.renderArea.offset.x = 0;
	pass.renderArea.offset.y = 0;
	pass.renderArea.extent.width = _bound->Width;
	pass.renderArea.extent.height = _bound->Height;
	pass.clearValueCount = 2;
	pass.pClearValues = clears;
	pass.framebuffer = _frame->Framebuffer;

	VkCommandBufferBeginInfo buffer{};
	buffer.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkCommandBufferInheritanceInfo inherit{};
	inherit.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inherit.renderPass = pass.renderPass;
	inherit.framebuffer = pass.framebuffer;

	result = vkBeginCommandBuffer(_inheritance.Buffer, &buffer);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "could not begin command-buffer" << sys::EOM;
		return false;
	}

	vkCmdBeginRenderPass(_inheritance.Buffer, &pass, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	return true;
}

bool Phusis::Internal::VkStateMachine::EndDraw()
{
	for (const auto& thread : _threads)
		vkCmdExecuteCommands(_inheritance.Buffer, thread.Buffers.size(), thread.Buffers.data());
	vkCmdEndRenderPass(_inheritance.Buffer);

	VkResult result = vkEndCommandBuffer(_inheritance.Buffer);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "could not end command-buffer" << sys::EOM;
		return false;
	}

	return true;
}

bool Phusis::Internal::VkStateMachine::Submit()
{
	VkResult fence;
	do
	{
		fence = vkWaitForFences(_inheritance.Device, 1, &_renderFence, VK_TRUE, 100000000);
	} while (fence == VK_TIMEOUT);
	if (fence != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "could not submit command-buffer" << sys::EOM;
		return false;
	}
	vkResetFences(_inheritance.Device, 1, &_renderFence);

	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &_inheritance.Buffer;

	VkResult result = vkQueueSubmit(_inheritance.Queue, 1, &submit, _renderFence);
	if (result != VK_SUCCESS)
	{
		sys::log.head(sys::CRIT) << "could not submit command-buffer" << sys::EOM;
		return false;
	}

	return true;
}

void Phusis::Internal::VkStateMachine::Start()
{
	PrepareSecondaryPools();
}

void Phusis::Internal::VkStateMachine::Update()
{
	clock_t curT = std::clock();

	BeginDraw();
	BatchBuffer();
	EndDraw();
	Submit();

	_deltaT = curT - _previousT;
	_previousT = curT;
}
