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

	tris->verts[1].position[0] = -1.0f;
	tris->verts[1].position[1] = 1.0f;
	tris->verts[1].position[2] = 0.0f;
	tris->verts[1].color[0] = 0.0f;
	tris->verts[1].color[1] = 1.0f;
	tris->verts[1].color[2] = 0.0f;

	tris->verts[2].position[0] = 0.0f;
	tris->verts[2].position[1] = -1.0f;
	tris->verts[2].position[2] = 0.0f;
	tris->verts[2].color[0] = 0.0f;
	tris->verts[2].color[1] = 0.0f;
	tris->verts[2].color[2] = 1.0f;
}