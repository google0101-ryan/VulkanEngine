#include "Image.h"
#include "VulkanInfo.h"

extern VmaAllocator allocator;

Vulkan::Image::Image(int width, int height, VkFormat format, VkImageUsageFlags usage, VkImageTiling tiling, VkImageAspectFlags aspectFlags)
{
	VkImageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.extent.width = width;
	createInfo.extent.height = height;
	createInfo.extent.depth = 1;
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.format = format;
	createInfo.tiling = tiling;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.usage = usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.flags = 0;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.flags = 0;
	allocInfo.priority = 1.0f;

	if (vmaCreateImage(allocator, &createInfo, &allocInfo, &image, &alloc, NULL) != VK_SUCCESS)
		throw std::runtime_error("Failed to create texture image");
	
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.subresourceRange.levelCount = 1;

	if (vkCreateImageView(backendInfo.device, &viewInfo, NULL, &view) != VK_SUCCESS)
		throw std::runtime_error("Failed to create image view for texture");
}

VkImage Vulkan::Image::GetImage()
{
	return image;
}

VkImageView Vulkan::Image::GetView()
{
	return view;
}
