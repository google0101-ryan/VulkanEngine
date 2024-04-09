#include <backend/Backend.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <cassert>

#include <backend/Vulkan/VulkanInfo.h>
#include <backend/Vulkan/VulkanInstance.h>
#include <backend/Vulkan/PhysicalDevice.h>
#include <backend/Vulkan/SwapChain.h>
#include <backend/Vulkan/ShaderUnit.h>
#include <backend/Vulkan/Pipeline.h>
#include <backend/Vulkan/RenderPass.h>
#include <backend/Vulkan/Framebuffer.h>
#include <geometry/Model.h>
#include <frontend/frontend.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

VulkanBackendInfo backendInfo;

RenderModel defaultTri;

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
		.SetupDynamicState()
		.SetupVertexInput(sizeof(DrawVert), 0)
		.AddAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(DrawVert, position))
		.AddAttribute(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(DrawVert, color))
		.SetupInputAssembly(VK_FALSE, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		.SetupViewportAndScisoor()
		.SetupRasterizer(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE)
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
	backendInfo.stagingBuffer.Create(BUFFERUSAGE_STAGING);

	defaultTri.MakeDefaultTriangle();
}

void RenderBackend::ShutdownBackend()
{
	vkDeviceWaitIdle(backendInfo.device);
	backendInfo.stagingBuffer.Destroy();
	backendInfo.vertexBuffer.Destroy();
	for (int i = 0; i < MAX_FRAME_DATA; i++)
	{
		vkDestroyFence(backendInfo.device, backendInfo.inFlightFences[i], NULL);
		vkDestroySemaphore(backendInfo.device, backendInfo.imageAvailSemas[i], NULL);
		vkDestroySemaphore(backendInfo.device, backendInfo.renderFinishedSemas[i], NULL);
		backendInfo.renderCmdBuffers[i]->Destroy();
		delete backendInfo.renderCmdBuffers[i];
	}
	for (auto framebuffer : backendInfo.swapChainFramebuffers)
		vkDestroyFramebuffer(backendInfo.device, framebuffer, NULL);
	vkDestroyPipeline(backendInfo.device, backendInfo.pipeline.pipeline, NULL);
	vkDestroyPipelineLayout(backendInfo.device, backendInfo.pipeline.layout, NULL);
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

void RenderBackend::DrawView()
{
	uint32_t index = frame % MAX_FRAME_DATA;

	// Make sure previous frame is done
	vkWaitForFences(backendInfo.device, 1, &backendInfo.inFlightFences[index], VK_TRUE, UINT64_MAX);
	vkResetFences(backendInfo.device, 1, &backendInfo.inFlightFences[index]);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(backendInfo.device, backendInfo.swapChain, UINT64_MAX, backendInfo.imageAvailSemas[index], VK_NULL_HANDLE, &imageIndex);

	backendInfo.stagingBuffer.CopyData(((renderTriangle_t*)(defaultTri.GetData()))->verts, defaultTri.GetSize());

	// Copy the staging buffer into the vertex buffer
	Vulkan::DoBufferCopy(backendInfo.vertexBuffer, backendInfo.stagingBuffer);

	auto& cmdBuf = backendInfo.renderCmdBuffers[index];
	cmdBuf->BeginFrame();
	cmdBuf->BeginRenderPass(backendInfo.renderPass, backendInfo.swapChainFramebuffers[imageIndex], backendInfo.swapChainExtent);
	cmdBuf->BindPipeline(backendInfo.pipeline.pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
	cmdBuf->SetupViewportAndScissor({0, 0}, backendInfo.swapChainExtent);
	cmdBuf->BindVertexBuffer(backendInfo.vertexBuffer.GetBuffer(), 0);
	cmdBuf->Draw(((renderTriangle_t*)(defaultTri.GetData()))->vertCount, 1, 0, 0);
	cmdBuf->EndFrame();

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

	frame++;
}
