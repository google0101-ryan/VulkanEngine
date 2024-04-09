#pragma once

#include <GLFW/glfw3.h>
#include <backend/Backend.h>

class RenderFrontend
{
public:
	~RenderFrontend();

	void InitFrontend(bool fullscreen = false);

	void DrawView();

	bool ShouldExit();

	GLFWwindow* GetWindow(); // Used by the Vulkan backend to create a window surface
private:
	GLFWwindow* window;
	RenderBackend backend;
};