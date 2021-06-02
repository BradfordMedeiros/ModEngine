#ifndef MOD_TYPES
#define MOD_TYPES

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <map>
#include <vector>
#include "./boundinfo.h"

struct Line {
  glm::vec3 fromPos;
  glm::vec3 toPos;
};

struct VoxelBody {
  glm::vec3 position;
  unsigned int textureId;
};

struct Transformation {
  glm::vec3 position;
  glm::vec3 scale;
  glm::quat rotation;
};

struct PhysicsInfo {
  BoundInfo boundInfo;
  Transformation transformation;
};

Transformation getTransformationFromMatrix(glm::mat4 matrix);

#endif