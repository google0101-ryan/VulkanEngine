#include <backend/Backend.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <cassert>
#include <chrono>

#include <backend/Vulkan/VulkanInfo.h>
#include <backend/Vulkan/VulkanInstance.h>
#include <backend/Vulkan/PhysicalDevice.h>
#include <backend/Vulkan/SwapChain.h>
#include <backend/Vulkan/ShaderUnit.h>
#include <backend/Vulkan/Pipeline.h>
#include <backend/Vulkan/RenderPass.h>
#include <backend/Vulkan/Framebuffer.h>
#include <backend/Vulkan/DescriptorPool.h>
#include <geometry/Model.h>
#include <frontend/frontend.h>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

VulkanBackendInfo backendInfo;

namespace Vulkan
{

VkDevice CreateDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = backendInfo.graphicsIndex;
	queueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;
	queueCreateInfos.push_back(queueCreateInfo);
	if (backendInfo.presentIndex != backendInfo.graphicsIndex)
	{
		VkDeviceQueueCreateInfo presentQueueCreateInfo = {};
		presentQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		presentQueueCreateInfo.queueFamilyIndex = backendInfo.presentIndex;
		presentQueueCreateInfo.queueCount = 1;
		presentQueueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(presentQueueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = queueCreateInfos.size();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = requiredExtensions.size();
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();
	createInfo.enabledLayerCount = 0;

	VkDevice ret;
	if (vkCreateDevice(backendInfo.gpu, &createInfo, NULL, &ret) != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical GPU device");
	
	return ret;
}

VkSemaphore CreateSemaphore()
{
	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VkSemaphore ret;
	if (vkCreateSemaphore(backendInfo.device, &createInfo, NULL, &ret) != VK_SUCCESS)
		throw std::runtime_error("Failed to create semaphore");
	return ret;
}

VkFence CreateFence(bool signalled)
{
	VkFenceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	if (signalled)
		createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	
	VkFence ret;
	if (vkCreateFence(backendInfo.device, &createInfo, NULL, &ret) != VK_SUCCESS)
		throw std::runtime_error("Failed to create fence\n");
	return ret;
}

}

void RenderBackend::InitGlfwFlags()
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO: Handle window resizing properly
}

void RenderBackend::SubmitCommand(DrawCommands cmd, void *data)
{
	switch (cmd)
	{
	case DRAW_INIT:
	{
		DrawInitInfo* initInfo = (DrawInitInfo*)data;
		InitBackend(initInfo);
		break;
	}
	case DRAW_SHUTDOWN:
		ShutdownBackend();
		break;
	case DRAW_RENDER_VIEW:
		DrawView();
		break;
	case DRAW_UPDATE_UNIFORM:
		curUniformInfo = *(DrawUniformInfo*)data;
		break;
	case DRAW_SUBMIT_GEOMETRY:
	{
		BackendModel model;
		model.model = (RenderModel*)data;
		models.push_back(model);
		break;
	}
	default:
		printf("ERROR: Couldn't execute unknown backend command %d\n", cmd);
		exit(1);
	}
}

void RenderBackend::InitBackend(DrawInitInfo *initInfo)
{
	backendInfo.width = initInfo->screenWidth;
	backendInfo.height = initInfo->screenHeight;

	backendInfo.instance = Vulkan::Instance("VULKAN", "VULKAN ENGINE")
							.SetApiVersion(VK_API_VERSION_1_3)
							.SetVersion(VK_MAKE_API_VERSION(0, 1, 0, 0), VK_MAKE_API_VERSION(0, 1, 0, 0))
#ifndef NDEBUG
							.EnableValidation()
#endif							
							.Build();
	
	if (backendInfo.instance == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to create vulkan instance");
	}
	
	glfwCreateWindowSurface(backendInfo.instance, initInfo->frontend->GetWindow(), NULL, &backendInfo.surface);
	backendInfo.gpu = Vulkan::PickPhysicalDevice();
	backendInfo.device = Vulkan::CreateDevice();
	vkGetDeviceQueue(backendInfo.device, backendInfo.graphicsIndex, 0, &backendInfo.graphicsQueue);
	vkGetDeviceQueue(backendInfo.device, backendInfo.presentIndex, 0, &backendInfo.presentQueue);
	backendInfo.swapChain = Vulkan::CreateSwapchain();

	backendInfo.colorShader = Vulkan::CreateShader("color");

	backendInfo.renderPass = Vulkan::RenderPassBuilder()
										.ConfigureColorAttachment()
										.SetSubpass()
										.Build();

	backendInfo.pipeline = Vulkan::PipelineBuilder()
		.AttachShader(Vulkan::SHADER_VERTEX, backendInfo.colorShader.vertexShader)
		.AttachShader(Vulkan::SHADER_FRAGMENT, backendInfo.colorShader.fragmentShader)
		.AddDescriptorLayout(Vulkan::ShaderType::SHADER_VERTEX, Vulkan::DescriptorType::DESCRIPTOR_UBO)
		.SetupDynamicState()
		.SetupVertexInput(sizeof(DrawVert), 0)
		.AddAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(DrawVert, position))
		.AddAttribute(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(DrawVert, color))
		.AddAttribute(0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(DrawVert, texCoord))
		.SetupInputAssembly(VK_FALSE, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		.SetupViewportAndScisoor()
		.SetupRasterizer(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
		.SetupMultisampling()
		.SetupColorBlending(VK_FALSE)
		.SetLayout()
		.Build();
	
	backendInfo.swapChainFramebuffers.resize(backendInfo.swapChainImageViews.size());
	for (size_t i = 0; i < backendInfo.swapChainImageViews.size(); i++)
		backendInfo.swapChainFramebuffers[i] = Vulkan::CreateFramebuffer(backendInfo.swapChainExtent, {backendInfo.swapChainImageViews[i]});
	
	backendInfo.renderCmdBuffers.resize(MAX_FRAME_DATA);
	backendInfo.imageAvailSemas.resize(MAX_FRAME_DATA);
	backendInfo.renderFinishedSemas.resize(MAX_FRAME_DATA);
	backendInfo.inFlightFences.resize(MAX_FRAME_DATA);
	for (int i = 0; i < MAX_FRAME_DATA; i++)
	{
		backendInfo.renderCmdBuffers[i] = new Vulkan::CommandBuffer(Vulkan::COMMAND_BUFFER_RERECORDABLE, backendInfo.graphicsIndex);
		backendInfo.imageAvailSemas[i] = Vulkan::CreateSemaphore();
		backendInfo.renderFinishedSemas[i] = Vulkan::CreateSemaphore();
		backendInfo.inFlightFences[i] = Vulkan::CreateFence(true);
	}

	backendInfo.vertexBuffer.Create(BUFFERUSAGE_VERTEX);
	backendInfo.indexBuffer.Create(BUFFERUSAGE_INDEX);
	backendInfo.vertexStagingBuffer.Create(BUFFERUSAGE_STAGING);
	backendInfo.indexStagingBuffer.Create(BUFFERUSAGE_STAGING);
	backendInfo.uniformBuffers.resize(MAX_FRAME_DATA);
	for (int i = 0; i < MAX_FRAME_DATA; i++)
	{
		backendInfo.uniformBuffers[i].Create(BUFFERUSAGE_UNIFORM, sizeof(DrawUniformInfo));
	}

	backendInfo.pool = Vulkan::DescriptorPoolBuilder()
						.AddSize(Vulkan::DescriptorPoolType::DESCRIPTOR_POOL_UBO,
								MAX_FRAME_DATA)
						.SetMaxSize(MAX_FRAME_DATA)
						.Build();
	
	backendInfo.descriptorSets = backendInfo.pool.AllocSets(backendInfo.pipeline.setConstants, MAX_FRAME_DATA);

	for (int i = 0; i < MAX_FRAME_DATA; i++)
	{
		VkDescriptorBufferInfo bufInfo = {};
		bufInfo.buffer = backendInfo.uniformBuffers[i].GetBuffer();
		bufInfo.offset = 0;
		bufInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet descWrite = {};
		descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descWrite.dstSet = backendInfo.descriptorSets[i];
		descWrite.dstBinding = 0;
		descWrite.dstArrayElement = 0;
		descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descWrite.descriptorCount = 1;
		descWrite.pBufferInfo = &bufInfo;
		
		vkUpdateDescriptorSets(backendInfo.device, 1, &descWrite, 0, NULL);
	}
}

void RenderBackend::ShutdownBackend()
{
	vkDeviceWaitIdle(backendInfo.device);
	backendInfo.vertexStagingBuffer.Destroy();
	backendInfo.indexStagingBuffer.Destroy();
	backendInfo.indexBuffer.Destroy();
	backendInfo.vertexBuffer.Destroy();
	for (int i = 0; i < MAX_FRAME_DATA; i++)
	{
		vkDestroyFence(backendInfo.device, backendInfo.inFlightFences[i], NULL);
		vkDestroySemaphore(backendInfo.device, backendInfo.imageAvailSemas[i], NULL);
		vkDestroySemaphore(backendInfo.device, backendInfo.renderFinishedSemas[i], NULL);
		backendInfo.renderCmdBuffers[i]->Destroy();
		backendInfo.uniformBuffers[i].Destroy();
		delete backendInfo.renderCmdBuffers[i];
	}
	vkDestroyDescriptorPool(backendInfo.device, backendInfo.pool.GetPool(), NULL);
	for (auto framebuffer : backendInfo.swapChainFramebuffers)
		vkDestroyFramebuffer(backendInfo.device, framebuffer, NULL);
	vkDestroyPipeline(backendInfo.device, backendInfo.pipeline.pipeline, NULL);
	vkDestroyPipelineLayout(backendInfo.device, backendInfo.pipeline.layout, NULL);
	for (auto setConst : backendInfo.pipeline.setConstants)
		vkDestroyDescriptorSetLayout(backendInfo.device, setConst, NULL);
	vkDestroyRenderPass(backendInfo.device, backendInfo.renderPass, NULL);
	vkDestroyShaderModule(backendInfo.device, backendInfo.colorShader.vertexShader, NULL);
	vkDestroyShaderModule(backendInfo.device, backendInfo.colorShader.fragmentShader, NULL);
	for (auto imageView : backendInfo.swapChainImageViews)
		vkDestroyImageView(backendInfo.device, imageView, NULL);
	vkDestroySwapchainKHR(backendInfo.device, backendInfo.swapChain, NULL);
	vkDestroyDevice(backendInfo.device, NULL);
	vkDestroySurfaceKHR(backendInfo.instance, backendInfo.surface, NULL);
	vkDestroyInstance(backendInfo.instance, NULL);
}

std::chrono::_V2::high_resolution_clock::time_point startTime;

void RenderBackend::DrawView()
{
	uint32_t index = frame % MAX_FRAME_DATA;

	if (!startTime.time_since_epoch().count())
		startTime = std::chrono::high_resolution_clock::now(); 
	auto curTime = std::chrono::high_resolution_clock::now();

	auto time = std::chrono::duration<float, std::chrono::seconds::period>(curTime - startTime).count();

	curUniformInfo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	// Make sure previous frame is done
	vkWaitForFences(backendInfo.device, 1, &backendInfo.inFlightFences[index], VK_TRUE, UINT64_MAX);
	vkResetFences(backendInfo.device, 1, &backendInfo.inFlightFences[index]);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(backendInfo.device, backendInfo.swapChain, UINT64_MAX, backendInfo.imageAvailSemas[index], VK_NULL_HANDLE, &imageIndex);

	// Reset all allocations in the buffer
	backendInfo.vertexStagingBuffer.BeginFrame();
	backendInfo.indexStagingBuffer.BeginFrame();

	for (auto& model : models)
	{
		model.vertexHandle = backendInfo.vertexStagingBuffer.Alloc(model.GetData()->verts, model.GetSize());
		model.indexHandle = backendInfo.indexStagingBuffer.Alloc(model.GetData()->indices, model.GetData()->indexCount * sizeof(uint16_t));
	}

	Vulkan::DoBufferCopy(backendInfo.vertexBuffer, backendInfo.vertexStagingBuffer);
	Vulkan::DoBufferCopy(backendInfo.indexBuffer, backendInfo.indexStagingBuffer);

	backendInfo.uniformBuffers[index].CopyData((void*)&curUniformInfo, sizeof(curUniformInfo));

	auto& cmdBuf = backendInfo.renderCmdBuffers[index];
	
	for (auto& model : models)
	{
		cacheHandle_t vertexHandle = model.vertexHandle;
		cacheHandle_t indexHandle = model.indexHandle;

		size_t vtxOffset = (vertexHandle >> 24) & 0x1ffffff;
		size_t idxOffset = (indexHandle >> 24) & 0x1ffffff;

		cmdBuf->BeginFrame();
		cmdBuf->BeginRenderPass(backendInfo.renderPass, backendInfo.swapChainFramebuffers[imageIndex], backendInfo.swapChainExtent);
		cmdBuf->BindPipeline(backendInfo.pipeline.pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
		cmdBuf->SetupViewportAndScissor({0, 0}, backendInfo.swapChainExtent);
		cmdBuf->BindVertexBuffer(backendInfo.vertexBuffer.GetBuffer(), vtxOffset);
		cmdBuf->BindIndexBuffer(backendInfo.indexBuffer.GetBuffer(), idxOffset);
		cmdBuf->BindDescriptorSet(backendInfo.descriptorSets[index]);
		cmdBuf->DrawIndexed(model.GetData()->indexCount, 1, 0, 0, 0);
		cmdBuf->EndFrame();
	}

	cmdBuf->Submit(backendInfo.graphicsQueue, backendInfo.imageAvailSemas[index], backendInfo.renderFinishedSemas[index], backendInfo.inFlightFences[index]);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &backendInfo.renderFinishedSemas[index];
	VkSwapchainKHR swapChains[] = {backendInfo.swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	vkQueuePresentKHR(backendInfo.presentQueue, &presentInfo);

	models.clear();

	frame++;
}
