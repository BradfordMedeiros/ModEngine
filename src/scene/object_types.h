#ifndef MOD_OBJECTTYPES
#define MOD_OBJECTTYPES

#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>
#include <variant>
#include "./common/mesh.h"
#include "./scenegraph.h"

struct GameObjectMesh {
  std::string meshName;
  Mesh mesh;
  bool isDisabled;
};
struct GameObjectCamera {};
struct GameObjectLight {
  /*!light:type:directional
  !light:type:directional 
  !light:maxangle:50
  !light:color:10 10 20 */
};

typedef std::variant<GameObjectMesh, GameObjectCamera, GameObjectLight> GameObjectObj;

static Field obj = {
  .prefix = '@', 
  .type = "default",
  .additionalFields = { "mesh", "disabled" }
};

static Field camera = {
  .prefix = '>',
  .type = "camera",
  .additionalFields = { },
};

static Field light = {
  .prefix = '!',
  .type = "light",
  .additionalFields = { },
};

static std::vector fields = { obj, camera, light };

std::map<short, GameObjectObj> getObjectMapping();

void addObject(short id, std::string objectType, std::string field, std::string payload, 
  std::map<short, GameObjectObj>& mapping, 
  std::map<std::string, Mesh>& meshes, std::string defaultMesh, std::function<void(std::string)> ensureMeshLoaded);

void removeObject(std::map<short, GameObjectObj>& mapping, short id);
void renderObject(short id, std::map<short, GameObjectObj>& mapping, Mesh& cameraMesh, bool showBoundingBoxForMesh,  Mesh& boundingboxMesh, bool showCameras);

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

std::vector<short> getGameObjectsIndex(std::map<short, GameObjectObj>& mapping);

#endif 
