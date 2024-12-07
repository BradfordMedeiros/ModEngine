#ifndef MOD_MESHGEN
#define MOD_MESHGEN

#include <vector>
#include <variant>
#include <iostream>
#include <glm/glm.hpp>
#include "./types.h"
#include "../../../translations.h"

Vertex createVertex(glm::vec3 position, glm::vec2 texCoords);
MeshData generateMesh(std::vector<glm::vec3>& face, std::vector<glm::vec3>& points);
MeshData generateMeshRaw(std::vector<glm::vec3>& verts, std::vector<unsigned int>& indexs);

#endif