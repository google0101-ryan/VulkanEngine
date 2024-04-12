#pragma once

#include <stdint.h>
#include <vulkan/vulkan.h>
#include <thirdparty/vma/vk_mem_alloc.h>

namespace Vulkan
{

class Image
{
public:
	Image(int width, int height, VkFormat format, VkImageUsageFlags usage, VkImageTiling tiling, VkImageAspectFlags aspectFlags);

	VkImage GetImage();
	VkImageView GetView();
private:
	VkImage image;
	VkImageView view;
	VmaAllocation alloc;
};

}