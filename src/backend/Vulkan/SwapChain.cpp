#include "SwapChain.h"

#include <algorithm>

VkSwapchainKHR Vulkan::CreateSwapchain()
{
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(backendInfo.gpu, backendInfo.surface, &formatCount, NULL);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(backendInfo.gpu, backendInfo.surface, &formatCount, formats.data());

	uint32_t presentCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(backendInfo.gpu, backendInfo.surface, &presentCount, NULL);
	std::vector<VkPresentModeKHR> presentModes(presentCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(backendInfo.gpu, backendInfo.surface, &presentCount, presentModes.data());

	VkSurfaceFormatKHR swapChainFormat;
	bool foundDesired = false;
	for (auto& availableFormats : formats)
	{
		if (availableFormats.format == VK_FORMAT_B8G8R8_SRGB && availableFormats.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			foundDesired = true;
			swapChainFormat = availableFormats;
			break;
		}
	}

	if (!foundDesired)
		swapChainFormat = formats[0];

	VkPresentModeKHR presentMode;
	foundDesired = false;
	for (auto& availablePresentModes : presentModes)
	{
		if (availablePresentModes == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			foundDesired = true;
			break;
		}
	}

	if (!foundDesired)
		presentMode = VK_PRESENT_MODE_FIFO_KHR;
	
	VkExtent2D extent;
	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(backendInfo.gpu, backendInfo.surface, &caps);

	if (caps.currentExtent.width == UINT32_MAX)
	{
		extent.width = std::clamp((uint32_t)backendInfo.width, caps.minImageExtent.width, caps.maxImageExtent.width);
		extent.height = std::clamp((uint32_t)backendInfo.height, caps.minImageExtent.height, caps.maxImageExtent.height);
	}
	else
	{
		extent = caps.currentExtent;
	}

	uint32_t imageCount = caps.minImageCount+1;
	if (caps.maxImageCount > 0 && caps.maxImageCount < imageCount)
		imageCount = caps.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = backendInfo.surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = swapChainFormat.format;
	createInfo.imageColorSpace = swapChainFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	std::vector<uint32_t> queueFamilyIndices = {backendInfo.graphicsIndex, backendInfo.presentIndex};
	
	if (backendInfo.graphicsIndex != backendInfo.presentIndex)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 1;
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}
	createInfo.preTransform = caps.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkSwapchainKHR ret;
	if (vkCreateSwapchainKHR(backendInfo.device, &createInfo, NULL, &ret) != VK_SUCCESS)
		throw std::runtime_error("Failed to create swapchain");
	
	// Go ahead and grab the swapchain images and place them in backendInfo
	vkGetSwapchainImagesKHR(backendInfo.device, ret, &imageCount, NULL);
	backendInfo.swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(backendInfo.device, ret, &imageCount, backendInfo.swapChainImages.data());
	
	backendInfo.swapChainFormat = swapChainFormat;
	backendInfo.swapChainExtent = extent;

	backendInfo.swapChainImageViews.resize(backendInfo.swapChainImages.size());

	for (size_t i = 0; i < backendInfo.swapChainImageViews.size(); i++)
	{
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = backendInfo.swapChainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = backendInfo.swapChainFormat.format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		if (vkCreateImageView(backendInfo.device, &imageViewCreateInfo, NULL, &backendInfo.swapChainImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create swapchain image view");
	}

	return ret;
}