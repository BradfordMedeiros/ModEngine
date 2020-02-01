#ifndef MOD_TYPES
#define MOD_TYPES

#include <glm/glm.hpp>

struct Line {
  glm::vec3 fromPos;
  glm::vec3 toPos;
};

struct VoxelBody {
  glm::vec3 position;
};

#endif