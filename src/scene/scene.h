#ifndef MOD_SCENE
#define MOD_SCENE

#include <iostream>
#include <map>
#include "./scenegraph.h"
#include "./object_types.h"
#include "./common/mesh.h"

struct FullScene {
  Scene scene;
  std::map<std::string, Mesh> meshes;
  std::map<short, GameObjectObj> objectMapping;
};

FullScene deserializeFullScene(std::string content);
std::string serializeFullScene(Scene& scene, std::map<short, GameObjectObj> objectMapping);

#endif

