#include "Texture.h"
#include <backend/Vulkan/VulkanInfo.h>

#define STB_IMAGE_IMPLEMENTATION
#include <thirdparty/stb/stb_image.h>

extern VmaAllocator allocator;

Vulkan::Texture::Texture(std::string fileName)
{
	int width, height, channels;
	stbi_uc* pixels = stbi_load(fileName.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	VkDeviceSize imageSize = width*height*4;

	if (!pixels)
		throw std::runtime_error("Failed to create texture from filename " + fileName);
	
	Vulkan::Buffer stagingBuffer;
	stagingBuffer.Create(BUFFERUSAGE_STAGING, imageSize);
	stagingBuffer.CopyData(pixels, imageSize);

	stbi_image_free(pixels);

	VkImageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.extent.width = width;
	createInfo.extent.height = height;
	createInfo.extent.depth = 1;
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.flags = 0;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.flags = 0;
	allocInfo.priority = 1.0f;

	if (vmaCreateImage(allocator, &createInfo, &allocInfo, &image, &alloc, NULL) != VK_SUCCESS)
		throw std::runtime_error("Failed to create texture image");
	
	CommandBuffer cmdBuf(COMMAND_BUFFER_DONT_CARE, backendInfo.graphicsIndex);

	cmdBuf.StartSingleTimeCommands();
	cmdBuf.TransitionImage(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	cmdBuf.CopyBufferToImage(stagingBuffer.GetBuffer(), image, width, height);
	cmdBuf.TransitionImage(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	cmdBuf.EndSingleTimeCommands();
	cmdBuf.Destroy();


}