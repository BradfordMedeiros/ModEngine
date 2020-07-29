#ifndef MOD_TYPES
#define MOD_TYPES

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct Line {
  glm::vec3 fromPos;
  glm::vec3 toPos;
};

struct VoxelBody {
  glm::vec3 position;
};

struct Transformation {
  glm::vec3 position;
  glm::vec3 scale;
  glm::quat rotation;
};

struct Properties {
  Transformation transformation;
};

#endif