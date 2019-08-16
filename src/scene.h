#ifndef MOD_SCENE
#define MOD_SCENE

#include <vector>
#include <map>
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
};

struct Scene {
  std::vector<GameObjectH> gameObjects;
  std::map<short, GameObject> idToGameObjects;
  std::map<std::string, short> nameToId;
};

Scene loadScene(Mesh& columnSeatMesh, Mesh& boxMesh, Mesh& grassMesh);

std::string serializeScene(Scene& scene);
Scene deserializeScene(std::string content, Mesh& mesh, std::map<std::string, Mesh> meshes);

#endif
