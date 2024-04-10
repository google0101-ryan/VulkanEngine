#pragma once

#include <glm/glm.hpp>

struct [[gnu::packed]] DrawVert
{
	float position[3];
	float color[3];
	float texCoord[2];
};

struct renderTriangle_t
{
	DrawVert* verts;
	int vertCount;
	uint16_t* indices;
	int indexCount;
};