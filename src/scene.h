#ifndef MOD_SCENE
#define MOD_SCENE

#include <vector>
#include <map>
#include "./mesh.h"

struct GameObject {
  short id;
  glm::vec3 position;
  glm::vec3 scale;
  Mesh mesh;
};
struct GameObjectH {
  short id;
};

struct Scene {
  std::vector<GameObjectH> gameObjects;
  std::vector<GameObjectH> rotatingGameObjects;
  std::map<short, GameObject> idToGameObjects;
};

Scene loadScene(Mesh& columnSeatMesh, Mesh& boxMesh, Mesh& grassMesh);

std::string serializeScene(Scene& scene);
Scene deserializeScene(std::string content);

#endif
