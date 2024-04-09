#pragma once

#include <vulkan/vulkan.h>
#include <thirdparty/vma/vk_mem_alloc.h>

typedef size_t cacheHandle_t;

enum BufferUsage
{
	BUFFERUSAGE_VERTEX,
	BUFFERUSAGE_INDEX,
	BUFFERUSAGE_STAGING
};

namespace Vulkan
{

class Buffer
{
public:
	void Create(BufferUsage usage);
	void Destroy();

	void CopyData(void* data, int size);

	void BeginFrame();

	VkBuffer GetBuffer() {return buffer;}
private:
	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo vmaAllocInfo;
	size_t usedBytes = 0;
	uint32_t frame = 0;
	size_t size = 0;
};

}