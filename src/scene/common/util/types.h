#ifndef MOD_TYPES
#define MOD_TYPES

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/compatibility.hpp>
#include <map>
#include <vector>
#include "./boundinfo.h"

#define NUM_BONES_PER_VERTEX 4

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 tangent;
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
  std::string roughnessTexturePath;
  bool hasRoughnessTexture;
  std::string normalTexturePath;
  bool hasNormalTexture;
  BoundInfo boundInfo;
  std::vector<Bone> bones;
};

struct Line {
  glm::vec3 fromPos;
  glm::vec3 toPos;
};

enum LineColor { RED, GREEN, BLUE };

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

enum AlignType { NEGATIVE_ALIGN, CENTER_ALIGN, POSITIVE_ALIGN };
enum TextWrapType { WRAP_NONE, WRAP_CHARACTERS };
struct TextWrap {
  TextWrapType type;
  float wrapamount;
};

Transformation getTransformationFromMatrix(glm::mat4 matrix);
glm::vec3 distanceToSecondFromFirst(glm::mat4 y, glm::mat4 x);
void printTransformInformation(Transformation transform);
void printMatrixInformation(glm::mat4 transform, std::string label);
BoundInfo getBounds(std::vector<Vertex>& vertices);
glm::mat4 matrixFromComponents(glm::mat4 initialModel, glm::vec3 position, glm::vec3 scale, glm::quat rotation);
glm::mat4 matrixFromComponents(Transformation transformation);
Transformation interpolate(Transformation transform1, Transformation transform2, float posamount, float scaleamount, float rotamount);

#endif