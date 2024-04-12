#pragma once

#include <vulkan/vulkan.h>

namespace Vulkan
{

class Buffer;

enum CommandBufferType
{
	COMMAND_BUFFER_DONT_CARE,
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
	void BindIndexBuffer(VkBuffer buffer, size_t offset);
	void BindDescriptorSet(VkDescriptorSet set, uint32_t offset = 0);
	void Draw(int vertexCount, int instanceCount, int firstVertex, int firstInstance);
	void DrawIndexed(int indices, int instanceCount, int startOffs, int indexOffs, int firstInstance);
	void EndFrame();

	void StartSingleTimeCommands();
	void TransitionImage(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void EndSingleTimeCommands();

	void Submit(VkQueue queue, VkSemaphore waitSemaphore, VkSemaphore finishedSema, VkFence fence);
private:
	VkCommandPool cmdPool;
	VkCommandBuffer buffer;
	uint32_t descriptorOffset;
};

void DoBufferCopy(Buffer& dst, Buffer& src, uint32_t offset = 0);

}