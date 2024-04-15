#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct DrawVert
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;

	bool operator==(const DrawVert& other) const 
	{
    	return position == other.position && color == other.color && texCoord == other.texCoord;
	}
};

namespace std {
    template<> struct hash<DrawVert> {
        size_t operator()(DrawVert const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position) ^
                   (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

struct renderTriangle_t
{
	DrawVert* verts;
	uint32_t vertCount;
	uint32_t* indices;
	uint32_t indexCount;
};