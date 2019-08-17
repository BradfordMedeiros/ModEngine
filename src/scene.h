#ifndef MOD_SCENE
#define MOD_SCENE

#include <vector>
#include <map>
#include <set>
#include "./mesh.h"

struct GameObject {
  short id;
  short parentId;
  std::string name;
  glm::vec3 position;
  glm::vec3 scale;
  Mesh mesh;
  bool isRotating;
};
struct GameObjectH {
  short id;
  short parentId;
  std::set<short> children;
};

struct Scene {
  std::vector<short> rootGameObjectsH;
  std::map<short, GameObject> idToGameObjects;
  std::map<short, GameObjectH> idToGameObjectsH;
  std::map<std::string, short> nameToId;
};

std::string serializeScene(Scene& scene);
Scene deserializeScene(std::string content, Mesh& mesh, std::map<std::string, Mesh> meshes);

#endif
