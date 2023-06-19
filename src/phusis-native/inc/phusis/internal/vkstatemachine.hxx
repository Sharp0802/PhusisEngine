#ifndef PHUSIS_VKSTATEMACHINE_HXX
#define PHUSIS_VKSTATEMACHINE_HXX

#include "fw.hxx"
#include "phusis/engineobject.hxx"

namespace Phusis::Internal
{
	struct VkBoundData
	{
		uint32_t Width;
		uint32_t Height;
		glm::mat4 View;
		glm::mat4 Projection;

		const std::vector<EngineObjectData>& Objects;
	};

	struct VkRendererInheritance
	{
		VkDevice Device;

		VkQueue Queue;
		uint32_t QueueIdx;

		VkCommandPool Pool;
		VkCommandBuffer Buffer;
		VkRenderPass RenderPass;

		VkPipeline Pipeline;
		VkPipelineLayout PipelineLayout;

		VkClearColorValue ClearColor;
	};

	struct VkThreadData
	{
		uint32_t Index;
		VkCommandPool Pool;
		std::vector<VkCommandBuffer> Buffers;
	};

	struct VkFrameData
	{
		VkFramebuffer Framebuffer;
	};

	struct VkFrameDataLocal
	{
		VkCommandBufferInheritanceInfo Inheritance;
	};

	enum class BufferDistributionStrategy
	{
		Optimal,
		Uniform
	};

	class VkStateMachine
	{
	private:
		const VkBoundData* _bound;
		const VkFrameData* _frame;
		const VkRendererInheritance _inheritance;

		VkFrameDataLocal _local;

		std::vector<VkThreadData> _threads;
		std::vector<VkCommandBuffer> _buffer;

		uint32_t _knownTargetCount;
		clock_t _previousT;
		clock_t _deltaT;

		VkFence _renderFence;

	private:
		bool PrepareSecondaryPools();
		bool PrepareCommandBuffers(uint32_t idx, uint32_t n);
		bool DistributeBuffers(int32_t update, BufferDistributionStrategy strategy);

		bool BatchBuffer();
		bool BatchBufferLocal(uint32_t idx, uint32_t offset, uint32_t size);

		bool BeginDraw();
		bool EndDraw();

		bool Submit();

	public:
		void Start();
		void Update();
	};
}

#endif //PHUSIS_VKSTATEMACHINE_HXX
