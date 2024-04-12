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

	image = new Image(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR, VK_IMAGE_ASPECT_COLOR_BIT);
	
	CommandBuffer cmdBuf(COMMAND_BUFFER_DONT_CARE, backendInfo.graphicsIndex);

	cmdBuf.StartSingleTimeCommands();
	cmdBuf.TransitionImage(image->GetImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	cmdBuf.CopyBufferToImage(stagingBuffer.GetBuffer(), image->GetImage(), width, height);
	cmdBuf.TransitionImage(image->GetImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	cmdBuf.EndSingleTimeCommands();
	cmdBuf.Destroy();
	stagingBuffer.Destroy();
	
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
	imageInfo.imageView = image->GetView();
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