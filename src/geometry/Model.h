#pragma once

#include "DrawVert.h"

class RenderModel
{
private:
	renderTriangle_t* tris;
public:
	void MakeDefaultTriangle();
	void MakeDefaultQuad();

	renderTriangle_t* GetData() {return tris;}
	size_t GetSize() {return tris->vertCount * sizeof(DrawVert);}
};