#include "Pipeline.h"

namespace Vulkan
{

std::vector<VkDynamicState> dynamicStates =
{
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR	
};

PipelineBuilder &PipelineBuilder::AddDescriptorLayout(ShaderType usage, DescriptorType type)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = setConstants.size();
	uboLayoutBinding.descriptorCount = 1;
	switch (type)
	{
	case DESCRIPTOR_UBO:
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		break;
	case DESCRIPTOR_UBO_DYNAMIC:
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		break;
	}

	switch (usage)
	{
	case ShaderType::SHADER_VERTEX:
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		break;
	}

	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = 1;
	createInfo.pBindings = &uboLayoutBinding;

	VkDescriptorSetLayout layout;

	if (vkCreateDescriptorSetLayout(backendInfo.device, &createInfo, NULL, &layout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout");

	setConstants.push_back(layout);

	return *this;
}

PipelineBuilder &PipelineBuilder::AttachShader(ShaderType type, VkShaderModule mod)
{
	VkPipelineShaderStageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	switch (type)
	{
	case SHADER_VERTEX:
		createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case SHADER_FRAGMENT:
		createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	}

	createInfo.module = mod;
	createInfo.pName = "main";

	shaderStages.push_back(createInfo);

	return *this;
}

PipelineBuilder &PipelineBuilder::SetupDynamicState()
{
	dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();

	return *this;
}
PipelineBuilder &PipelineBuilder::SetupVertexInput(size_t stride, int binding)
{
	inputBinding = {};
	inputBinding.binding = binding;
	inputBinding.stride = stride;
	inputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return *this;
}

PipelineBuilder &PipelineBuilder::AddAttribute(int binding, int location, VkFormat format, size_t offset)
{
	VkVertexInputAttributeDescription desc = {};
	desc.binding = binding;
	desc.location = location;
	desc.format = format;
	desc.offset = offset;

	attributeDesc.push_back(desc);

	return *this;
}

PipelineBuilder &PipelineBuilder::SetupInputAssembly(bool primitiveReset, VkPrimitiveTopology topology)
{
	inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = topology;
	inputAssembly.primitiveRestartEnable = primitiveReset;

	return *this;
}
PipelineBuilder &PipelineBuilder::SetupViewportAndScisoor()
{
	viewport = {};
	viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport.viewportCount = 1;
	viewport.scissorCount = 1;

	return *this;
}
PipelineBuilder &PipelineBuilder::SetupRasterizer(VkCullModeFlagBits cullMode, VkFrontFace frontFace)
{
	rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.frontFace = frontFace;
	rasterizer.cullMode = cullMode;
	rasterizer.depthBiasEnable = VK_FALSE;

	return *this;
}
PipelineBuilder &PipelineBuilder::SetupMultisampling()
{
	// TODO: Make this configurable
	multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	
	return *this;
}
PipelineBuilder &PipelineBuilder::SetupColorBlending(VkBool32 blendEnabled, VkBlendFactor srcBlendFactor, VkBlendFactor dstBlendFactor, VkBlendOp op)
{
	blending = {};
	blending.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blending.blendEnable = blendEnabled;
	blending.srcColorBlendFactor = srcBlendFactor;
	blending.dstColorBlendFactor = dstBlendFactor;
	blending.colorBlendOp = op;
	blending.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blending.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blending.alphaBlendOp = VK_BLEND_OP_ADD;

	blendState = {};
	blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendState.logicOpEnable = VK_FALSE;
	blendState.attachmentCount = 1;
	blendState.pAttachments = &blending;

	return *this;
}
PipelineBuilder &PipelineBuilder::SetLayout()
{
	uint32_t setConstantSize = setConstants.size();
	uint32_t pushConstanstSize = pushConstants.size();

	pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineCreateInfo.setLayoutCount = setConstantSize;
	pipelineCreateInfo.pSetLayouts = setConstantSize ? setConstants.data() : NULL;
	pipelineCreateInfo.pushConstantRangeCount = pushConstanstSize;
	pipelineCreateInfo.pPushConstantRanges = pushConstanstSize ? pushConstants.data() : NULL;

	return *this;
}

pipeline_t PipelineBuilder::Build()
{
	pipeline_t ret;

	ret.setConstants = setConstants;

	// We start by creating the pipeline layout
	if (vkCreatePipelineLayout(backendInfo.device, &pipelineCreateInfo, NULL, &layout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline layout");

	ret.layout = layout;

	vertexInput = {};
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.vertexAttributeDescriptionCount = attributeDesc.size();
	vertexInput.pVertexAttributeDescriptions = attributeDesc.data();
	vertexInput.vertexBindingDescriptionCount = 1;
	vertexInput.pVertexBindingDescriptions = &inputBinding;

	VkGraphicsPipelineCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.stageCount = shaderStages.size();
	createInfo.pStages = shaderStages.data();
	createInfo.pVertexInputState = &vertexInput;
	createInfo.pInputAssemblyState = &inputAssembly;
	createInfo.pViewportState = &viewport;
	createInfo.pRasterizationState = &rasterizer;
	createInfo.pMultisampleState = &multisampling;
	createInfo.pColorBlendState = &blendState;
	createInfo.pDynamicState = &dynamicState;
	createInfo.layout = layout;
	createInfo.renderPass = backendInfo.renderPass;
	createInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(backendInfo.device, VK_NULL_HANDLE, 1, &createInfo, NULL, &ret.pipeline) != VK_SUCCESS)
		throw std::runtime_error("Failed to create graphics pipeline");

	return ret;
}
}