#include "CommandBuffer.h"

#include <backend/Vulkan/VulkanInfo.h>

Vulkan::CommandBuffer::CommandBuffer(CommandBufferType type, uint32_t queueIndex, bool isPrimary)
{
	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	switch (type)
	{
	case COMMAND_BUFFER_STATIC:
		poolCreateInfo.flags = 0;
		break;
	case COMMAND_BUFFER_DYNAMIC:
		poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		break;
	case COMMAND_BUFFER_DONT_CARE:
	case COMMAND_BUFFER_RERECORDABLE:
		poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		break;
	}
	poolCreateInfo.queueFamilyIndex = queueIndex;

	if (vkCreateCommandPool(backendInfo.device, &poolCreateInfo, NULL, &cmdPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create command buffer pool");
	
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandPool = cmdPool;
	allocInfo.commandBufferCount = 1;
	
	if (vkAllocateCommandBuffers(backendInfo.device, &allocInfo, &buffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create command buffer");
}

void Vulkan::CommandBuffer::Destroy()
{
	vkFreeCommandBuffers(backendInfo.device, cmdPool, 1, &buffer);
	vkDestroyCommandPool(backendInfo.device, cmdPool, NULL);
}

void Vulkan::CommandBuffer::BeginFrame()
{
	vkResetCommandBuffer(buffer, 0);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = NULL;

	if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begine command buffer");
}

void Vulkan::CommandBuffer::BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, VkExtent2D size, VkClearValue clearColor, VkClearValue depthColor)
{
	VkClearValue values[2] = {clearColor, depthColor};

	VkRenderPassBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = renderPass;
	beginInfo.framebuffer = framebuffer;
	beginInfo.renderArea.offset = {0, 0};
	beginInfo.renderArea.extent = size;
	beginInfo.clearValueCount = 2;
	beginInfo.pClearValues = values;

	vkCmdBeginRenderPass(buffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Vulkan::CommandBuffer::BindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint)
{
	vkCmdBindPipeline(buffer, bindPoint, pipeline);
}

void Vulkan::CommandBuffer::SetupViewportAndScissor(VkExtent2D offset, VkExtent2D size)
{
	VkViewport viewport = {};
	viewport.x = offset.width;
	viewport.y = offset.height;
	viewport.width = size.width;
	viewport.height = size.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {(int)offset.width, (int)offset.height};
	scissor.extent = size;

	vkCmdSetViewport(buffer, 0, 1, &viewport);
	vkCmdSetScissor(buffer, 0, 1, &scissor);
}

void Vulkan::CommandBuffer::BindVertexBuffer(VkBuffer buffer, size_t offset)
{
	vkCmdBindVertexBuffers(this->buffer, 0, 1, &buffer, &offset);
}

void Vulkan::CommandBuffer::BindIndexBuffer(VkBuffer buffer, size_t offset)
{
	vkCmdBindIndexBuffer(this->buffer, buffer, offset, VK_INDEX_TYPE_UINT16);
}

void Vulkan::CommandBuffer::BindDescriptorSet(VkDescriptorSet set, int setNum, int offset)
{
	descriptorOffset = offset;
	if (offset >= 0)
		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backendInfo.pipeline.layout, setNum, 1, &set, 1, &descriptorOffset);
	else
		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backendInfo.pipeline.layout, setNum, 1, &set, 0, NULL);
}

void Vulkan::CommandBuffer::Draw(int vertexCount, int instanceCount, int firstVertex, int firstInstance)
{
	vkCmdDraw(buffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void Vulkan::CommandBuffer::DrawIndexed(int indices, int instanceCount, int startOffs, int indexOffs, int firstInstance)
{
	vkCmdDrawIndexed(buffer, indices, instanceCount, startOffs, indexOffs, firstInstance);
}

void Vulkan::CommandBuffer::EndFrame()
{
	vkCmdEndRenderPass(buffer);
	vkEndCommandBuffer(buffer);
}

void Vulkan::CommandBuffer::StartSingleTimeCommands()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(buffer, &beginInfo);
}

void Vulkan::CommandBuffer::TransitionImage(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
		throw std::invalid_argument("unsupported layout transition!");


	vkCmdPipelineBarrier(this->buffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Vulkan::CommandBuffer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = {0, 0};
	region.imageExtent = {width, height, 1};

	vkCmdCopyBufferToImage(this->buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void Vulkan::CommandBuffer::EndSingleTimeCommands()
{
	vkEndCommandBuffer(buffer);

	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.commandBufferCount = 1;
	info.pCommandBuffers = &buffer;

	vkQueueSubmit(backendInfo.graphicsQueue, 1, &info, VK_NULL_HANDLE);
	vkQueueWaitIdle(backendInfo.graphicsQueue);
}

void Vulkan::CommandBuffer::Submit(VkQueue queue, VkSemaphore waitSemaphore, VkSemaphore finishedSema, VkFence fence)
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	
	VkSemaphore waitSemaphores[] = {waitSemaphore};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buffer;

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &finishedSema;

	if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit command buffer");
}

void Vulkan::DoBufferCopy(Buffer &dst, Buffer &src, uint32_t offset)
{
	VkCommandPoolCreateInfo poolInfo = {};
	VkCommandPool cmdPool;
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = backendInfo.graphicsIndex;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	if (vkCreateCommandPool(backendInfo.device, &poolInfo, NULL, &cmdPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create temporary command pool\n");

	VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = cmdPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(backendInfo.device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = offset; // Optional
	copyRegion.size = 65536;
	vkCmdCopyBuffer(commandBuffer, src.GetBuffer(), dst.GetBuffer(), 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.commandBufferCount = 1;
	info.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(backendInfo.graphicsQueue, 1, &info, VK_NULL_HANDLE);
	vkQueueWaitIdle(backendInfo.graphicsQueue);

	vkFreeCommandBuffers(backendInfo.device, cmdPool, 1, &commandBuffer);
	vkDestroyCommandPool(backendInfo.device, cmdPool, NULL);
}
