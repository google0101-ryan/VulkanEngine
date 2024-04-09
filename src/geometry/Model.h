#pragma once

#include "DrawVert.h"

class RenderModel
{
private:
	renderTriangle_t* tris;
public:
	void MakeDefaultTriangle();
	void* GetData() {return (void*)tris;}
	size_t GetSize() {return tris->vertCount * sizeof(DrawVert);}
};