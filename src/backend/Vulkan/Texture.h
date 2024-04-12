#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <thirdparty/vma/vk_mem_alloc.h>
#include "Image.h"

namespace Vulkan
{

class Texture
{
public:
	Texture(std::string fileName);
public:
	Image* image;
	VkSampler sampler;
	VmaAllocation alloc;
	VkDescriptorSet descriptorSet;
};

}