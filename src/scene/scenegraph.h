#ifndef MOD_SCENEGRAPH
#define MOD_SCENEGRAPH

#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <functional>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "./common/mesh.h"
#include "../common/util.h"
#include "./serialization.h"

struct GameObject {
  short id;
  std::string name;
  Transformation transformation;
  physicsOpts physicsOptions;
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

std::string serializeScene(Scene& scene, std::function<std::vector<std::pair<std::string, std::string>>(short)> getAdditionalFields);
Scene deserializeScene(
  std::string content, 
  std::function<void(short id, std::string type, std::map<std::string, std::string> additionalFields)> addObject, 
  std::vector<Field> fields,
  short (*getNewObjectId)()
);
void addObjectToScene(
  Scene& scene, 
  std::string name, 
  std::string mesh, 
  glm::vec3 position, 
  short (*getNewObjectId)(), 
  std::function<void(short, std::string, std::map<std::string, std::string> additionalFields)> addObject
);

void addSubsceneToRoot(std::map<short, short> childToParent, std::map<short, Transformation> gameobjTransforms, std::map<short, std::string> names);

void removeObjectFromScene(Scene& scene, short id);
std::vector<short> listObjInScene(Scene& scene);

// @TODO code these functions
void traverseScene(Scene& scene, std::function<void(short, glm::mat4)> onObject);  
glm::vec3 getFullPosition(short id);
glm::quat getFullRotation(short id);
glm::vec3 getFullScale(short id);

#endif
