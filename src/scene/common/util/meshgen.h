#ifndef MOD_MESHGEN
#define MOD_MESHGEN

#include <vector>
#include <variant>
#include <iostream>
#include <glm/glm.hpp>
#include "./types.h"
#include "../../../translations.h"

MeshData generateMesh(std::vector<glm::vec3>& face, std::vector<glm::vec3>& points);

#endif