#pragma once

#include <RenderDefines.h>

#ifdef RENDERER_VULKAN
#include <backend/Vulkan/Texture.h>
#endif

#include <string>

class Texture
#ifdef RENDERER_VULKAN
: Vulkan::Texture
#endif
{
private:
	using BaseClass = 
#ifdef RENDERER_VULKAN
	Vulkan::Texture;
#endif

public:
	Texture(std::string path)
	: BaseClass(path) {}
};