#pragma once

#include <backend/Vulkan/VulkanInfo.h>

namespace Vulkan
{

VkFramebuffer CreateFramebuffer(VkExtent2D size, std::vector<VkImageView> attachments);

}