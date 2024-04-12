#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <thirdparty/vma/vk_mem_alloc.h>

namespace Vulkan
{

class Texture
{
public:
	Texture(std::string fileName);
public:
	VkImage image;
	VkSampler sampler;
	VkImageView view;
	VmaAllocation alloc;
	VkDescriptorSet descriptorSet;
};

}