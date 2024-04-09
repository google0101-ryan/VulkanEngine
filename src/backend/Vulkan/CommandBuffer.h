#pragma once

#include <vulkan/vulkan.h>

namespace Vulkan
{

class Buffer;

enum CommandBufferType
{
	COMMAND_BUFFER_STATIC,
	COMMAND_BUFFER_DYNAMIC,
	COMMAND_BUFFER_RERECORDABLE
};

class CommandBuffer
{
public:
	CommandBuffer(CommandBufferType type, uint32_t queueIndex, bool isPrimary = true);
	void Destroy();

	void BeginFrame();
	void BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D size, VkClearValue clearColor = {{0.0f, 0.0f, 0.0f, 1.0f}});
	void BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint);
	void SetupViewportAndScissor(VkExtent2D offset, VkExtent2D size);
	void BindVertexBuffer(VkBuffer buffer, size_t offset);
	void Draw(int vertexCount, int instanceCount, int firstVertex, int firstInstance);
	void EndFrame();

	void Submit(VkQueue queue, VkSemaphore waitSemaphore, VkSemaphore finishedSema, VkFence fence);
private:
	VkCommandPool cmdPool;
	VkCommandBuffer buffer;
};

void DoBufferCopy(Buffer& dst, Buffer& src);

}