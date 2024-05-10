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

Transformation aiKeysToTransform(aiVectorKey& positionKey, aiQuatKey& rotationKey, aiVectorKey& scalingKey);
glm::mat4 transformToGlm(Transformation transform);

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

struct ModelData {
  std::map<int32_t, MeshData> meshIdToMeshData;
  std::map<int32_t, std::vector<int>> nodeToMeshId;
  std::map<int32_t, int32_t> childToParent;
  std::map<int32_t, Transformation> nodeTransform;
  std::map<int32_t, std::string> names;
  std::vector<Animation> animations;
};

// this really should be "load gameobject" --> since need children mesh
// but no representation for scene/children/objects yet so just flattening it to models
ModelData loadModel(std::string rootname, std::string modelPath);
std::vector<glm::vec3> getVertexsFromModelData(ModelData& data);

std::string nameForMeshId(std::string& rootmesh, int32_t meshId);
std::vector<std::string> meshNamesForNode(ModelData& modelData, std::string& rootmesh, std::string nodeName);

#endif 

