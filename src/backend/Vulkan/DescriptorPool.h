#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Vulkan
{

class Buffer;

enum DescriptorPoolType
{
	DESCRIPTOR_POOL_UBO,
	DESCRIPTOR_POOL_UBO_DYNAMIC
};

class DescriptorPool
{
	friend class DescriptorPoolBuilder;
public:
	std::vector<VkDescriptorSet> AllocSets(std::vector<VkDescriptorSetLayout>& layouts, int count);

	VkDescriptorPool GetPool() {return pool;}
private:
	VkDescriptorPool pool;
	int allocs = 0;
};

class DescriptorPoolBuilder
{
public:
	DescriptorPoolBuilder();

	DescriptorPoolBuilder& AddSize(DescriptorPoolType type, uint32_t count);
	DescriptorPoolBuilder& SetMaxSize(uint32_t size);
	DescriptorPool Build();


private:
	VkDescriptorPoolCreateInfo createInfo;
	std::vector<VkDescriptorPoolSize> sizes;
};

}