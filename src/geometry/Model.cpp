#include "Model.h"

#include <stdio.h>
#include <cstring>

void RenderModel::MakeDefaultTriangle()
{
	tris = new renderTriangle_t;
	tris->vertCount = 3;
	tris->verts = new DrawVert[3];
	memset(tris->verts, 0, tris->vertCount*sizeof(DrawVert));

	tris->verts[0].position[0] = 1.0f;
	tris->verts[0].position[1] = 1.0f;
	tris->verts[0].position[2] = 0.0f;
	tris->verts[0].color[0] = 1.0f;
	tris->verts[0].color[1] = 0.0f;
	tris->verts[0].color[2] = 0.0f;
	tris->verts[0].texCoord[0] = 1.0f;
	tris->verts[0].texCoord[1] = 1.0f;

	tris->verts[1].position[0] = -1.0f;
	tris->verts[1].position[1] = 1.0f;
	tris->verts[1].position[2] = 0.0f;
	tris->verts[1].color[0] = 0.0f;
	tris->verts[1].color[1] = 1.0f;
	tris->verts[1].color[2] = 0.0f;
	tris->verts[0].texCoord[0] = 0.0f;
	tris->verts[0].texCoord[1] = 1.0f;

	tris->verts[2].position[0] = 0.0f;
	tris->verts[2].position[1] = -1.0f;
	tris->verts[2].position[2] = 0.0f;
	tris->verts[2].color[0] = 0.0f;
	tris->verts[2].color[1] = 0.0f;
	tris->verts[2].color[2] = 1.0f;
	tris->verts[0].texCoord[0] = 0.5f;
	tris->verts[0].texCoord[1] = 0.0f;

	tris->indexCount = 3;
	tris->indices = new uint16_t[3];
	tris->indices[0] = 0;
	tris->indices[1] = 1;
	tris->indices[2] = 2;
}

void RenderModel::MakeDefaultQuad()
{
	tris = new renderTriangle_t;
	tris->vertCount = 4;
	tris->verts = new DrawVert[4];
	memset(tris->verts, 0, tris->vertCount*sizeof(DrawVert));

	tris->verts[0].position[0] = -0.5f;
	tris->verts[0].position[1] = -0.5f;
	tris->verts[0].position[2] = 0.0f;
	tris->verts[0].color[0] = 1.0f;
	tris->verts[0].color[1] = 0.0f;
	tris->verts[0].color[2] = 0.0f;
	tris->verts[0].texCoord[0] = 0.0f;
	tris->verts[0].texCoord[1] = 1.0f;

	tris->verts[1].position[0] = 0.5f;
	tris->verts[1].position[1] = -0.5f;
	tris->verts[1].position[2] = 0.0f;
	tris->verts[1].color[0] = 0.0f;
	tris->verts[1].color[1] = 1.0f;
	tris->verts[1].color[2] = 0.0f;
	tris->verts[0].texCoord[0] = 1.0f;
	tris->verts[0].texCoord[1] = 1.0f;

	tris->verts[2].position[0] = 0.5f;
	tris->verts[2].position[1] = 0.5f;
	tris->verts[2].position[2] = 0.0f;
	tris->verts[2].color[0] = 0.0f;
	tris->verts[2].color[1] = 0.0f;
	tris->verts[2].color[2] = 1.0f;
	tris->verts[0].texCoord[0] = 1.0f;
	tris->verts[0].texCoord[1] = 0.0f;

	tris->verts[3].position[0] = -0.5f;
	tris->verts[3].position[1] = 0.5f;
	tris->verts[3].position[2] = 0.0f;
	tris->verts[3].color[0] = 1.0f;
	tris->verts[3].color[1] = 1.0f;
	tris->verts[3].color[2] = 1.0f;
	tris->verts[0].texCoord[0] = 0.0f;
	tris->verts[0].texCoord[1] = 0.0f;

	tris->indexCount = 6;
	tris->indices = new uint16_t[6];
	tris->indices[0] = 0;
	tris->indices[1] = 1;
	tris->indices[2] = 2;
	tris->indices[3] = 2;
	tris->indices[4] = 3;
	tris->indices[5] = 0;

	AddAlbedo("base/textures/texture.jpg");
}

void RenderModel::AddAlbedo(std::string texName)
{
	texture = new Texture(texName);
}
