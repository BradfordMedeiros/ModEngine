#ifndef MOD_LOADMODEL
#define MOD_LOADMODEL

#include <iostream>
#include <vector>
#include <map>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <functional>
#include <filesystem>
#include "./boundinfo.h"
#include "./types.h"
#include "../../../common/util.h"

#define NUM_BONES_PER_VERTEX 4

glm::mat4 aiKeysToGlm(aiVectorKey& positionKey, aiQuatKey& rotationKey, aiVectorKey& scalingKey);

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoords;
  short boneIndexes[NUM_BONES_PER_VERTEX]; // hardcoded limit of 4 per vertex
  float boneWeights[NUM_BONES_PER_VERTEX]; 
};


struct AnimationChannel {
  std::string nodeName;
  std::vector<aiVectorKey> positionKeys;    // @TODO decouple this from assimp 
  std::vector<aiVectorKey> scalingKeys;
  std::vector<aiQuatKey> rotationKeys;
};

struct Animation {
  std::string name;
  double duration;
  double ticksPerSecond;
  std::vector<AnimationChannel> channels;
};


struct Bone {
  std::string name;
  glm::mat4 offsetMatrix;
};

struct MeshData {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<std::string> texturePaths;
  BoundInfo boundInfo;
  std::vector<Bone> bones;
};

struct ModelData {
  std::map<short, MeshData> meshIdToMeshData;
  std::map<short, std::vector<int>> nodeToMeshId;
  std::map<short, short> childToParent;
  std::map<short, Transformation> nodeTransform;
  std::map<short, std::string> names;
  std::vector<Animation> animations;
};

// this really should be "load gameobject" --> since need children mesh
// but no representation for scene/children/objects yet so just flattening it to models
ModelData loadModel(std::string modelPath);  

#endif 

