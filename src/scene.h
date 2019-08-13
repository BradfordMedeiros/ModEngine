#ifndef MOD_SCENE
#define MOD_SCENE

#include <vector>
#include "./mesh.h"

struct GameObject {
  short id;
  glm::vec3 position;
  glm::vec3 scale;
  Mesh& mesh;
};
struct Scene {
  std::vector<GameObject> gameObjects;
  std::vector<GameObject> rotatingGameObjects;
};

Scene loadScene(Mesh& columnSeatMesh, Mesh& boxMesh, Mesh& grassMesh);

#endif
