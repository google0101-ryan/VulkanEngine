#pragma once

#include <backend/Vulkan/VulkanInfo.h>

namespace Vulkan
{

class RenderPassBuilder
{
public:
	RenderPassBuilder& ConfigureColorAttachment();
	RenderPassBuilder& SetSubpass();
	VkRenderPass Build();
private:
	VkAttachmentDescription colorAttachment;
	VkAttachmentReference colorAttachmentRef;
	VkSubpassDescription subpassDescription;
};

}