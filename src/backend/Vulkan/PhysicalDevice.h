#pragma once

#include <backend/Vulkan/VulkanInfo.h>

extern std::vector<const char*> requiredExtensions;

namespace Vulkan
{

VkPhysicalDevice PickPhysicalDevice();

}