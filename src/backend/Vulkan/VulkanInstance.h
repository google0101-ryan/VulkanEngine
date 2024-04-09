#pragma once

#include <vulkan/vulkan.h>

namespace Vulkan
{

class Instance
{
public:
	Instance(const char* appName, const char* engineName);
	Instance& SetApiVersion(uint32_t apiVersion);
	Instance& SetVersion(uint32_t appVer, uint32_t engineVer);
	Instance& EnableValidation();

	VkInstance Build() const;
private:
	const char* appName;
	const char* engineName;
	uint32_t apiVersion;
	uint32_t appVersion, engineVersion;
	bool validationEnabled = false;
};

}