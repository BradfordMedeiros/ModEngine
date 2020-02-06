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

struct MeshData {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<std::string> texturePaths;
  BoundInfo boundInfo;
};

struct ModelData {
  std::vector<MeshData> meshData;
  /*std::map<short, short> parentToChild;
  std::map<short, Transformation> nodeTransform;
  std::map<short, std::string> names;*/
};




// this really should be "load gameobject" --> since need children mesh
// but no representation for scene/children/objects yet so just flattening it to models
std::vector<MeshData> loadModel(std::string modelPath);  

#endif 

