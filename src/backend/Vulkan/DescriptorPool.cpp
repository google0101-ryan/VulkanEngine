#include "DescriptorPool.h"
#include <backend/Vulkan/VulkanInfo.h>

Vulkan::DescriptorPoolBuilder::DescriptorPoolBuilder()
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
}

Vulkan::DescriptorPoolBuilder &Vulkan::DescriptorPoolBuilder::AddSize(DescriptorPoolType type, uint32_t count)
{
	VkDescriptorPoolSize poolSize = {};
	switch (type)
	{
	case DESCRIPTOR_POOL_UBO:
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		break;
	case DESCRIPTOR_POOL_UBO_DYNAMIC:
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		break;
	case DESCRIPTOR_POOL_SAMPLER:
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		break;
	}
	poolSize.descriptorCount = count;

	sizes.push_back(poolSize);

	return *this;
}

Vulkan::DescriptorPoolBuilder &Vulkan::DescriptorPoolBuilder::SetMaxSize(uint32_t size)
{
	createInfo.maxSets = size;
	return *this;
}

Vulkan::DescriptorPool Vulkan::DescriptorPoolBuilder::Build()
{
	createInfo.poolSizeCount = sizes.size();
	createInfo.pPoolSizes = sizes.data();

	VkDescriptorPool pool;
	if (vkCreateDescriptorPool(backendInfo.device, &createInfo, NULL, &pool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor pool");
	
	DescriptorPool ret;
	ret.pool = pool;

	return ret;
}

std::vector<VkDescriptorSet> Vulkan::DescriptorPool::AllocSets(VkDescriptorSetLayout layout, int count)
{
	std::vector<VkDescriptorSet> sets;
	sets.resize(count);

	std::vector<VkDescriptorSetLayout> setLayout(count, layout);
	VkDescriptorSetAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.descriptorPool = pool;
	info.descriptorSetCount = count;
	info.pSetLayouts = setLayout.data();

	if (vkAllocateDescriptorSets(backendInfo.device, &info, sets.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate descriptor sets");
	
	return sets;
}
