#ifndef MOD_SCENEGRAPH
#define MOD_SCENEGRAPH

#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <functional>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "./common/mesh.h"
#include "../common/util.h"
#include "./serialization.h"

struct GameObject {
  objid id;
  std::string name;
  Transformation transformation;
  physicsOpts physicsOptions;
  std::string lookat;
  std::string layer;
  std::string script;
  bool netsynchronize;
};
struct GameObjectH {
  objid id;
  objid parentId;
  std::set<objid> children;
  objid groupId;       // grouping mechanism for nodes.  It is either its own id, or explicitly stated when created. 
};

struct Scene {
  std::vector<objid> rootGameObjectsH;
  std::map<objid, GameObject> idToGameObjects;
  std::map<objid, GameObjectH> idToGameObjectsH;
  std::map<std::string, objid> nameToId;
  std::vector<LayerInfo> layers;
};

struct SceneDeserialization {
  Scene scene;
  std::map<std::string, SerializationObject> serialObjs;
};

std::string serializeObject(Scene& scene, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds, objid id);
std::string serializeScene(Scene& scene, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds);
SceneDeserialization deserializeScene(
  std::string content, 
  std::vector<Field> fields,
  std::function<objid()> getNewObjectId
);

SerializationObject serialObjectFromFields(
  std::string name, 
  glm::vec3 position, 
  std::string layer,
  std::vector<Field> fields,
  std::map<std::string, std::string> additionalFields
);

SerializationObject makeObjectInScene(
  Scene& scene, 
  std::string name, 
  glm::vec3 position, 
  std::string layer,
  std::function<objid()> getNewObjectId,
  std::vector<Field> fields,
  std::map<std::string, std::string> additionalFields
);

SerializationObject makeObjectInScene(
  Scene& scene,
  std::string serializedObj,
  std::function<objid()> getNewObjectId,
  std::vector<Field> fields
);


std::map<std::string, SerializationObject> addSubsceneToRoot(
  Scene& scene, 
  objid rootId,
  objid rootIdNode, 
  std::map<objid, objid> childToParent, 
  std::map<objid, Transformation> gameobjTransforms, 
  std::map<objid, std::string> names, 
  std::map<objid, std::map<std::string, std::string>> additionalFields,
  std::function<objid()> getNewObjectId
);

std::vector<objid> removeObjectFromScenegraph(Scene& scene, objid id);
std::vector<objid> listObjInScene(Scene& scene);

// @TODO code these functions
void traverseScene(Scene& scene, std::function<void(objid, glm::mat4, glm::mat4, bool)> onObject);  

Transformation getTransformationFromMatrix(glm::mat4 matrix);
Transformation fullTransformation(Scene& scene, objid id);
std::vector<objid> getIdsInGroup(Scene& scene, objid groupId);

std::map<std::string, std::string> scenegraphAttributes(Scene& scene, objid id);
void setScenegraphAttributes(Scene& scene, objid id, std::map<std::string, std::string> attributes);

#endif
