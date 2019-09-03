#ifndef MOD_OBJECTTYPES
#define MOD_OBJECTTYPES

#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>
#include <variant>
#include "./scene/common/mesh.h"
#include "./scene/scene.h"

struct GameObjectMesh {
  std::string meshName;
  Mesh mesh;
};
struct GameObjectCamera {
  int number = 0;
};

typedef std::variant<GameObjectMesh, GameObjectCamera> GameObjectObj;

static Field obj = {
  .prefix = '@', 
  .type = "default",
  .additionalFields = { "mesh" }
};

static Field camera = {
  .prefix = '>',
  .type = "camera",
  .additionalFields = { },
};
static std::vector fields = { obj, camera };

std::map<short, GameObjectObj> getObjectMapping();

void addObject(short id, std::string objectType, std::string field, std::string payload, 
  std::map<short, GameObjectObj>& mapping, 
  std::map<std::string, Mesh>& meshes, std::string defaultMesh, std::function<void(std::string)> ensureMeshLoaded);

void renderObject(short id, std::map<short, GameObjectObj>& mapping, Mesh& cameraMesh, bool showCameras);

std::vector<std::pair<std::string, std::string>> getAdditionalFields(short id, std::map<short, GameObjectObj>& mapping);

template<typename T>
std::vector<short> getGameObjectsIndex(std::map<short, GameObjectObj>& mapping){   // putting templates have to be in header?
  std::vector<short> indicies;
  for (auto [id, gameobject]: mapping){
    auto gameobjectP = std::get_if<T>(&gameobject);
    if (gameobjectP != NULL){
      indicies.push_back(id);
    }
  }
  return indicies;
}

#endif 