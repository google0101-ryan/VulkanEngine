#include "Framebuffer.h"

VkFramebuffer Vulkan::CreateFramebuffer(VkExtent2D size, std::vector<VkImageView> attachments)
{
	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.attachmentCount = attachments.size();
	createInfo.pAttachments = attachments.data();
	createInfo.renderPass = backendInfo.renderPass;
	createInfo.width = size.width;
	createInfo.height = size.height;
	createInfo.layers = 1;

	VkFramebuffer ret;
	if (vkCreateFramebuffer(backendInfo.device, &createInfo, NULL, &ret) != VK_SUCCESS)
		throw std::runtime_error("Failed to create framebuffer");
	return ret;
}