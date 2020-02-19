#ifndef MOD_LOADMODEL
#define MOD_LOADMODEL

#include <iostream>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <functional>
#include <filesystem>
#include "./loadmodel.h"
#include "./boundinfo.h"
#include "./types.h"

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoords;
};

struct Animation {
  std::string name;
  double duration;
  double ticksPerSecond;
};


struct Bone {
  std::string name;
  aiMatrix4x4t<float> offsetMatrix;
  std::vector<aiVertexWeight> vertexWeights;  //  weight.mVertexId,  weight.mWeight
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

