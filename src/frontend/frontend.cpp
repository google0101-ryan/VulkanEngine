#include "frontend.h"
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

RenderModel defaultTri, defaultQuad, vikingHouse;

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

	defaultQuad.MakeDefaultQuad();
	defaultTri.MakeDefaultTriangle();

	vikingHouse.LoadModel("base/models/viking_room.obj");
}

void RenderFrontend::DrawView()
{
	static auto startTime = std::chrono::high_resolution_clock::now(); 
	auto curTime = std::chrono::high_resolution_clock::now();

	auto time = std::chrono::duration<float, std::chrono::seconds::period>(curTime - startTime).count();

	// Check for exit commands from the user, as an easy out
	glfwPollEvents();

	if (glfwWindowShouldClose(window))
		return;
	
	DrawUniformInfo info = {};
	
	glm::mat4 view = glm::lookAt(glm::vec3(2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	info.view = view;
	info.proj = glm::perspective(glm::radians(45.0f), WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 10.0f);
	info.proj[1][1] *= -1;
	backend.SubmitCommand(DRAW_UPDATE_UNIFORM, &info);

	vikingHouse.modelTransform = glm::mat4(1.0f);
	backend.SubmitCommand(DRAW_SUBMIT_GEOMETRY, (void*)&vikingHouse);

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
