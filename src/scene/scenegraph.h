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


enum ColliderShape { BOX, SPHERE };

struct physicsOpts {
  bool enabled;
  bool isStatic;
  bool hasCollisions;
  ColliderShape shape;
};

struct GameObject {
  short id;
  std::string name;
  glm::vec3 position;
  glm::vec3 scale;
  glm::quat rotation;
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

struct Token {
  std::string target;
  std::string attribute;
  std::string payload;
};

struct Field {
  char prefix;
  std::string type;
  std::vector<std::string> additionalFields;
};

std::string serializeScene(Scene& scene, std::function<std::vector<std::pair<std::string, std::string>>(short)> getAdditionalFields);
Scene deserializeScene(
  std::string content, 
  std::function<void(short, std::string, std::string, std::string)> addObject, 
  std::vector<Field> fields,
  short (*getNewObjectId)()
);
void addObjectToScene(
  Scene& scene, 
  std::string name, 
  std::string mesh,  
  glm::vec3 position, 
  short (*getNewObjectId)(),
  std::function<void(short, std::string, std::string, std::string)> addObject
);
void removeObjectFromScene(Scene& scene, short id);
std::vector<short> listObjInScene(Scene& scene);

// @TODO code these functions
void traverseScene(Scene& scene, std::function<void(short, glm::mat4)> onObject);  
glm::vec3 getFullPosition(short id);
glm::quat getFullRotation(short id);

#endif
