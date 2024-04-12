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
	stagingBuffer.Destroy();

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.subresourceRange.levelCount = 1;

	if (vkCreateImageView(backendInfo.device, &viewInfo, NULL, &view) != VK_SUCCESS)
		throw std::runtime_error("Failed to create image view for texture");
	
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(backendInfo.gpu, &properties);

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	
	if (vkCreateSampler(backendInfo.device, &samplerInfo, NULL, &sampler) != VK_SUCCESS)
		throw std::runtime_error("Failed to create sampler for texture");
	
	descriptorSet = backendInfo.pool.AllocSets(backendInfo.pipeline.setConstants[1], 1)[0];

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = view;
	imageInfo.sampler = sampler;

	VkWriteDescriptorSet writeInfo = {};
	writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo.dstSet = descriptorSet;
	writeInfo.dstBinding = 0;
	writeInfo.dstArrayElement = 0;
	writeInfo.descriptorCount = 1;
	writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeInfo.pImageInfo = &imageInfo;
		
	vkUpdateDescriptorSets(backendInfo.device, 1, &writeInfo, 0, NULL);
}