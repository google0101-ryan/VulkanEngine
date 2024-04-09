#include "frontend.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

RenderFrontend::~RenderFrontend()
{
	backend.SubmitCommand(DRAW_SHUTDOWN, NULL);

	glfwDestroyWindow(window);
	glfwTerminate();
}

void RenderFrontend::InitFrontend(bool fullscreen)
{
	// API-agnostic stuff (GLFW, mostly)
	glfwInit();

	backend.InitGlfwFlags();

	int monitorCount;
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan Engine", fullscreen ? glfwGetMonitors(&monitorCount)[0] : NULL, NULL);

	DrawInitInfo initInfo;
	initInfo.screenHeight = WINDOW_HEIGHT;
	initInfo.screenWidth = WINDOW_WIDTH;
	initInfo.frontend = this;
	backend.SubmitCommand(DRAW_INIT, &initInfo);
}

void RenderFrontend::DrawView()
{
	// Check for exit commands from the user, as an easy out
	glfwPollEvents();

	if (glfwWindowShouldClose(window))
		return;
	
	backend.SubmitCommand(DRAW_RENDER_VIEW, NULL);
}

bool RenderFrontend::ShouldExit()
{
	return glfwWindowShouldClose(window);
}

GLFWwindow *RenderFrontend::GetWindow()
{
	return window;
}
