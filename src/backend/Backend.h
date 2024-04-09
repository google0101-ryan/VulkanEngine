#pragma once

#include <stdint.h>

class RenderFrontend;

enum DrawCommands
{
	DRAW_NOP = 0,
	DRAW_INIT = 1,
	DRAW_SHUTDOWN = 2,
	DRAW_RENDER_VIEW = 3,
};

struct DrawInitInfo
{
	int screenWidth, screenHeight;
	RenderFrontend* frontend;
};

class RenderBackend
{
public:
	void InitGlfwFlags();

	void SubmitCommand(DrawCommands cmd, void* data);
private:
	void DrawView();
	void InitBackend(DrawInitInfo* initInfo);
	void ShutdownBackend();
private:
	uint32_t frame = 0;
};