#include "VulkanInstance.h"

#include <GLFW/glfw3.h>
#include <vector>
#include <cstdio>

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

Vulkan::Instance::Instance(const char *appName, const char *engineName)
: appName(appName),
engineName(engineName)
{
}

Vulkan::Instance &Vulkan::Instance::SetApiVersion(uint32_t apiVersion)
{
	this->apiVersion = apiVersion;
	return *this;
}

Vulkan::Instance &Vulkan::Instance::SetVersion(uint32_t appVer, uint32_t engineVer)
{
	this->appVersion = appVer;
	this->engineVersion = engineVer;
	return *this;
}

Vulkan::Instance &Vulkan::Instance::EnableValidation()
{
	printf("DEBUG: Enabling validation layers\n");
	this->validationEnabled = true;
	return *this;
}

VkInstance Vulkan::Instance::Build() const
{
	VkInstance ret;

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName;
	appInfo.applicationVersion = appVersion;
	appInfo.pEngineName = engineName;
	appInfo.engineVersion = engineVersion;
	appInfo.apiVersion = apiVersion;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	if (!validationEnabled)
	{
		createInfo.enabledLayerCount = 0;
	}
	else
	{
		createInfo.enabledLayerCount = validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	VkResult result = vkCreateInstance(&createInfo, NULL, &ret);

	if (result != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return ret;
}
