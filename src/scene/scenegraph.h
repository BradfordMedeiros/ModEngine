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
  std::string lookat;
  std::string layer;
};
struct GameObjectH {
  short id;
  short parentId;
  std::set<short> children;
  short groupId;       // grouping mechanism for nodes.  It is either its own id, or explicitly stated when created. 
};

struct Scene {
  std::vector<short> rootGameObjectsH;
  std::map<short, GameObject> idToGameObjects;
  std::map<short, GameObjectH> idToGameObjectsH;
  std::map<std::string, short> nameToId;
  std::vector<LayerInfo> layers;
};

struct SceneDeserialization {
  Scene scene;
  std::map<std::string, SerializationObject> serialObjs;
};

std::string serializeScene(Scene& scene, std::function<std::vector<std::pair<std::string, std::string>>(short)> getAdditionalFields);
SceneDeserialization deserializeScene(
  std::string content, 
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

std::map<std::string, SerializationObject> addSubsceneToRoot(
  Scene& scene, 
  short rootId,
  short rootIdNode, 
  std::map<short, short> childToParent, 
  std::map<short, Transformation> gameobjTransforms, 
  std::map<short, std::string> names, 
  std::map<short, std::map<std::string, std::string>> additionalFields,
  short (*getNewObjectId)()
);

void removeObjectFromScene(Scene& scene, short id);
std::vector<short> listObjInScene(Scene& scene);

// @TODO code these functions
void traverseScene(Scene& scene, std::function<void(short, glm::mat4, glm::mat4, bool)> onObject);  
glm::vec3 getFullPosition(short id);
glm::quat getFullRotation(short id);
glm::vec3 getFullScale(short id);
std::vector<short> getIdsInGroup(Scene& scene, short groupId);

#endif
