#ifndef MOD_LOADMODEL
#define MOD_LOADMODEL

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <functional>
#include <filesystem>
#include "./loadmodel.h"

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoords;
};

struct BoundInfo {
  float xMin, xMax;
  float yMin, yMax;
  float zMin, zMax;
};

struct ModelData {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<std::string> texturePaths;
  BoundInfo boundInfo;
};


// this really should be "load gameobject" --> since need children mesh
// but no representation for scene/children/objects yet so just flattening it to models
std::vector<ModelData> loadModel(std::string modelPath);  

#endif 
