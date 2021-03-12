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
  glm::vec3 collisionInfo;
  Transformation transformation;
};

struct GameobjAttributes {
  std::map<std::string, std::string> stringAttributes;
  std::map<std::string, double> numAttributes;
  std::map<std::string, glm::vec3> vecAttributes;
  
  // todo get rid of these fields
  std::map<std::string, std::string> additionalFields;
  std::vector<std::string> children;
};

Transformation getTransformationFromMatrix(glm::mat4 matrix);

#endif