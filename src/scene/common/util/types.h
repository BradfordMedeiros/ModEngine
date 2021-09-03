#ifndef MOD_TYPES
#define MOD_TYPES

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <map>
#include <vector>
#include "./boundinfo.h"

#define NUM_BONES_PER_VERTEX 4

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoords;
  int32_t boneIndexes[NUM_BONES_PER_VERTEX]; // hardcoded limit of 4 per vertex
  float boneWeights[NUM_BONES_PER_VERTEX]; 
};

struct Bone {
  std::string name;
  glm::mat4 offsetMatrix;
  glm::mat4 initialBonePose;
};

struct MeshData {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::string diffuseTexturePath;
  bool hasDiffuseTexture;
  std::string emissionTexturePath;
  bool hasEmissionTexture;
  std::string opacityTexturePath;
  bool hasOpacityTexture;
  BoundInfo boundInfo;
  std::vector<Bone> bones;
};

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
void printMatrixInformation(glm::mat4 transform, std::string label);
BoundInfo getBounds(std::vector<Vertex>& vertices);
glm::mat4 matrixFromComponents(glm::mat4 initialModel, glm::vec3 position, glm::vec3 scale, glm::quat rotation);
glm::mat4 matrixFromComponents(Transformation transformation);

#endif