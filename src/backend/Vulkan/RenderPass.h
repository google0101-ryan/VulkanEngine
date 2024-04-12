#pragma once

#include <backend/Vulkan/VulkanInfo.h>

namespace Vulkan
{

class RenderPassBuilder
{
public:
	RenderPassBuilder& ConfigureColorAttachment();
	RenderPassBuilder& SetDepthAttachment(VkFormat depthFormat);
	RenderPassBuilder& SetSubpass();
	VkRenderPass Build();
private:
	VkAttachmentDescription colorAttachment;
	VkAttachmentDescription depthAttachment;
	VkAttachmentReference colorAttachmentRef;
	VkAttachmentReference depthAttachmentRef;
	VkSubpassDescription subpassDescription;
	bool hasDepthBuffer = false;
};

}