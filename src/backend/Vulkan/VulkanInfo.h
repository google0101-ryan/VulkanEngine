#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>
#include <backend/Vulkan/CommandBuffer.h>
#include <backend/Vulkan/Buffer.h>
#include <backend/Vulkan/DescriptorPool.h>
#include <backend/Vulkan/Image.h>

static constexpr int MAX_FRAME_DATA = 2;

struct shader_t
{
	VkShaderModule vertexShader, fragmentShader;
};

struct pipeline_t
{
	VkPipelineLayout layout;
	VkPipeline pipeline;
	std::vector<VkDescriptorSetLayout> setConstants;
};

struct VulkanBackendInfo
{
	int width, height;
	VkInstance instance;
	VkPhysicalDevice gpu;
	uint32_t graphicsIndex, presentIndex; // These are usually the same thing
	VkDevice device;
	VkQueue graphicsQueue, presentQueue;
	VkSurfaceKHR surface;
	VkSurfaceFormatKHR swapChainFormat;
	VkExtent2D swapChainExtent;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	shader_t colorShader;
	pipeline_t pipeline;
	VkRenderPass renderPass;
	std::vector<Vulkan::CommandBuffer*> renderCmdBuffers;
	std::vector<VkSemaphore> imageAvailSemas, renderFinishedSemas;
	std::vector<VkFence> inFlightFences;
	Vulkan::Buffer vertexBuffer, indexBuffer, vertexStagingBuffer, indexStagingBuffer;
	std::vector<Vulkan::Buffer> uniformBuffers;
	Vulkan::DescriptorPool pool;
	std::vector<VkDescriptorSet> descriptorSets;
	Vulkan::Image* depthBufferImage;
};

extern VulkanBackendInfo backendInfo;