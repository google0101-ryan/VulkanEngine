#pragma once

#include "DrawVert.h"
#include "Texture.h"
#include <glm/glm.hpp>

class RenderModel
{
private:
	renderTriangle_t* tris;

	Texture* texture;
public:
	void MakeDefaultTriangle();
	void MakeDefaultQuad();

	void LoadModel(std::string modelPath);

	void AddAlbedo(std::string texName);

	Texture* GetTexture() {return texture;}
	renderTriangle_t* GetData() {return tris;}
	size_t GetSize() {return tris->vertCount * sizeof(DrawVert);}
	
	glm::mat4 modelTransform;
};