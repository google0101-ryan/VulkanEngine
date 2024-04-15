#include "Model.h"

#include <stdio.h>
#include <cstring>
#define TINYOBJLOADER_IMPLEMENTATION
#include <thirdparty/tiny_obj_loader/tiny_obj_loader.h>

#include <unordered_map>

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
	tris->indices = new uint32_t[3];
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
	tris->verts[0].texCoord[0] = 1.0f;
	tris->verts[0].texCoord[1] = 0.0f;

	tris->verts[1].position[0] = 0.5f;
	tris->verts[1].position[1] = -0.5f;
	tris->verts[1].position[2] = 0.0f;
	tris->verts[1].color[0] = 0.0f;
	tris->verts[1].color[1] = 1.0f;
	tris->verts[1].color[2] = 0.0f;
	tris->verts[1].texCoord[0] = 0.0f;
	tris->verts[1].texCoord[1] = 0.0f;

	tris->verts[2].position[0] = 0.5f;
	tris->verts[2].position[1] = 0.5f;
	tris->verts[2].position[2] = 0.0f;
	tris->verts[2].color[0] = 0.0f;
	tris->verts[2].color[1] = 0.0f;
	tris->verts[2].color[2] = 1.0f;
	tris->verts[2].texCoord[0] = 0.0f;
	tris->verts[2].texCoord[1] = 1.0f;

	tris->verts[3].position[0] = -0.5f;
	tris->verts[3].position[1] = 0.5f;
	tris->verts[3].position[2] = 0.0f;
	tris->verts[3].color[0] = 1.0f;
	tris->verts[3].color[1] = 1.0f;
	tris->verts[3].color[2] = 1.0f;
	tris->verts[3].texCoord[0] = 1.0f;
	tris->verts[3].texCoord[1] = 1.0f;

	tris->indexCount = 6;
	tris->indices = new uint32_t[6];
	tris->indices[0] = 0;
	tris->indices[1] = 1;
	tris->indices[2] = 2;
	tris->indices[3] = 2;
	tris->indices[4] = 3;
	tris->indices[5] = 0;

	AddAlbedo("base/textures/texture.jpg");
}

void RenderModel::LoadModel(std::string modelPath)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
        throw std::runtime_error(warn + err);
    }
	
	printf("%ld\n", attrib.vertices.size());

	std::unordered_map<DrawVert, uint32_t> uniqueVertices;
	std::vector<DrawVert> vertices;
	std::vector<uint32_t> indices;

	for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            DrawVert vertex{};

            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

	tris = new renderTriangle_t;
	tris->indexCount = indices.size();
	tris->vertCount = vertices.size();
	tris->indices = new uint32_t[indices.size()];
	tris->verts = new DrawVert[vertices.size()];

	printf("Created model with %ld indices (%ld bytes), %ld vertices (%ld bytes)\n", indices.size(), indices.size()*sizeof(uint32_t), vertices.size(), vertices.size()*sizeof(DrawVert));

	memcpy(tris->indices, indices.data(), indices.size()*sizeof(uint32_t));
	memcpy(tris->verts, vertices.data(), vertices.size()*sizeof(DrawVert));

	printf("Adding texture\n");

	texture = new Texture("base/textures/viking_room.png");
}

void RenderModel::AddAlbedo(std::string texName)
{
	texture = new Texture(texName);
}
