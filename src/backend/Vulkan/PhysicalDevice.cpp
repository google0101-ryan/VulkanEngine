#include "PhysicalDevice.h"

std::vector<const char*> requiredExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool SupportsExtensions(VkPhysicalDevice device, std::vector<const char*> requiredExtensions)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
	std::vector<VkExtensionProperties> properties(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, properties.data());

	std::vector<bool> foundExtensions;

	for (const auto& extension : properties)
	{
		for (const auto& reqExtension : requiredExtensions)
		{
			if (std::string(reqExtension) == std::string(extension.extensionName))
				foundExtensions.push_back(true);
		}
	}

	return foundExtensions.size() == requiredExtensions.size();
}

VkPhysicalDevice Vulkan::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(backendInfo.instance, &deviceCount, NULL);

	if (deviceCount == 0)
		throw std::runtime_error("No suitable Vulkan devices in system");
	
	std::vector<VkPhysicalDevice> devices;
	devices.resize(deviceCount);
	vkEnumeratePhysicalDevices(backendInfo.instance, &deviceCount, devices.data());

	// Pick the first GPU that supports both graphics and present queues
	// The user can swap out GPUs later from settings

	VkPhysicalDevice* optimal = NULL;

	for (auto& device : devices)
	{
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
		std::vector<VkQueueFamilyProperties> properties(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, properties.data());

		bool foundGraphics = false, foundPresent = false;
		int i = 0;
		for (const auto& queueFamilies : properties)
		{
			if (queueFamilies.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				backendInfo.graphicsIndex = i;
				foundGraphics = true;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, backendInfo.surface, &presentSupport);
			if (presentSupport)
			{
				backendInfo.presentIndex = i;
				foundPresent = true;
			}

			i++;
		}

		if (foundGraphics && foundPresent && SupportsExtensions(device, requiredExtensions))
			optimal = &device;
	}

	if (!optimal)
		throw std::runtime_error("Could not select suitable Vulkan-compatible GPU\n");
	
	return *optimal;
}