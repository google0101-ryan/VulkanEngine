#pragma once

#include "DrawVert.h"
#include <glm/glm.hpp>

class RenderModel
{
private:
	renderTriangle_t* tris;
public:
	void MakeDefaultTriangle();
	void MakeDefaultQuad();

	renderTriangle_t* GetData() {return tris;}
	size_t GetSize() {return tris->vertCount * sizeof(DrawVert);}
	
	glm::mat4 modelTransform;
};