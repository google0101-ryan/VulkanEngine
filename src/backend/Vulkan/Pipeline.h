#pragma once

#include <backend/Vulkan/VulkanInfo.h>

namespace Vulkan
{

enum ShaderType
{
	SHADER_VERTEX,
	SHADER_FRAGMENT,
};

enum DescriptorType
{
	DESCRIPTOR_UBO
};

class PipelineBuilder
{
public:
	PipelineBuilder& AddDescriptorLayout(ShaderType usage, DescriptorType type);

	PipelineBuilder& AttachShader(ShaderType type, VkShaderModule mod);
	PipelineBuilder& SetupDynamicState();
	PipelineBuilder& SetupVertexInput(size_t stride, int binding);
	PipelineBuilder& AddAttribute(int binding, int location, VkFormat format, size_t offset);
	PipelineBuilder& SetupInputAssembly(bool primitiveReset, VkPrimitiveTopology topology);
	PipelineBuilder& SetupViewportAndScisoor();
	PipelineBuilder& SetupRasterizer(VkCullModeFlagBits cullMode, VkFrontFace frontFace);
	PipelineBuilder& SetupMultisampling();
	PipelineBuilder& SetupColorBlending(VkBool32 blendEnabled, VkBlendFactor srcBlendFactor = VK_BLEND_FACTOR_ONE, VkBlendFactor dstBlendFactor = VK_BLEND_FACTOR_ONE, VkBlendOp op = VK_BLEND_OP_ADD);
	PipelineBuilder& SetLayout();
	pipeline_t Build();
private:
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkPipelineDynamicStateCreateInfo dynamicState;
	VkPipelineVertexInputStateCreateInfo vertexInput;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	VkPipelineViewportStateCreateInfo viewport;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineColorBlendAttachmentState blending;
	VkPipelineColorBlendStateCreateInfo blendState;
	std::vector<VkDescriptorSetLayout> setConstants;
	std::vector<VkPushConstantRange> pushConstants;
	VkPipelineLayoutCreateInfo pipelineCreateInfo;
	VkPipelineLayout layout;
	VkVertexInputBindingDescription inputBinding;
	std::vector<VkVertexInputAttributeDescription> attributeDesc;
};

}