#pragma once

#include <stdint.h>
#include <vector>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <geometry/Model.h>

typedef size_t cacheHandle_t;

class RenderFrontend;

enum DrawCommands
{
	DRAW_NOP = 0,
	DRAW_INIT = 1,
	DRAW_SHUTDOWN = 2,
	DRAW_RENDER_VIEW = 3,
	DRAW_UPDATE_UNIFORM = 4,
	DRAW_SUBMIT_GEOMETRY
};

struct DrawInitInfo
{
	int screenWidth, screenHeight;
	RenderFrontend* frontend;
};

class DrawUniformInfo
{
	friend class RenderBackend;
public:
	glm::mat4 view;
	glm::mat4 proj;
private:
	glm::mat4 model;
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
	class BackendModel
	{
	public:
		cacheHandle_t vertexHandle, indexHandle, uniformHandle;
		RenderModel model;

		renderTriangle_t* GetData() {return model.GetData();}
		size_t GetSize() {return model.GetSize();}
	};

	uint32_t frame = 0;
	DrawUniformInfo curUniformInfo;
	std::vector<BackendModel> models;
};