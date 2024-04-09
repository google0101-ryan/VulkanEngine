#include "Buffer.h"

#include <backend/Vulkan/VulkanInfo.h>
#include <geometry/DrawVert.h>

#define VMA_VULKAN_VERSION 1003000
#define VMA_IMPLEMENTATION
#include <thirdparty/vma/vk_mem_alloc.h>

#include <cstring>

VmaAllocator allocator;
static bool allocInited = false;

void Vulkan::Buffer::Create(BufferUsage usage)
{
	if (!allocInited)
	{
		VmaAllocatorCreateInfo createInfo = {};
		createInfo.physicalDevice = backendInfo.gpu;
		createInfo.device = backendInfo.device;
		createInfo.instance = backendInfo.instance;
		vmaCreateAllocator(&createInfo, &allocator);
		allocInited = true;
	}

	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = 65536; // Max buffer size
	createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo = {};

	if (usage == BUFFERUSAGE_VERTEX)
	{
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}
	else if (usage == BUFFERUSAGE_STAGING)
	{
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
		allocInfo.flags =  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	}

	if (vmaCreateBuffer(allocator, &createInfo, &allocInfo, &buffer, &allocation, &vmaAllocInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan buffer");
}

void Vulkan::Buffer::CopyData(void* data, int size)
{
	void* mapped;
	if (vmaMapMemory(allocator, allocation, &mapped) != VK_SUCCESS)
		throw std::runtime_error("Failed to map buffer");
	memcpy(mapped, data, size);
	vmaUnmapMemory(allocator, allocation);
}

void Vulkan::Buffer::Destroy()
{
	vmaDestroyBuffer(allocator, buffer, allocation);
}

void Vulkan::Buffer::BeginFrame()
{
	usedBytes = 0;
	frame++;
}
