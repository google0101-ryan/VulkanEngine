#include "ShaderUnit.h"

#include <fstream>

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

VkShaderModule CreateShaderModule(std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = (uint32_t*)code.data();
	
	VkShaderModule ret;
	if (vkCreateShaderModule(backendInfo.device, &createInfo, NULL, &ret) != VK_SUCCESS)
		throw std::runtime_error("Failed to create shader module");
	
	return ret;
}

shader_t Vulkan::CreateShader(std::string baseName)
{
	auto vertShaderCode = readFile("base/shaders/" + baseName + ".vert.spv");
	auto fragShaderCode = readFile("base/shaders/" + baseName + ".frag.spv");

	shader_t shader;
	shader.vertexShader = CreateShaderModule(vertShaderCode);
	shader.fragmentShader = CreateShaderModule(fragShaderCode);

	return shader;
}