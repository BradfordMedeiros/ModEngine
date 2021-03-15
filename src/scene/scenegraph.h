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
#include "./serialobject.h"

struct GameObjectH {
  objid id;
  objid parentId;
  std::set<objid> children;
  objid groupId;       // grouping mechanism for nodes.  It is either its own id, or explicitly stated when created. 
  bool linkOnly;
};

struct Scene {
  objid rootId;
  bool isNested;
  std::map<objid, GameObject> idToGameObjects;
  std::map<objid, GameObjectH> idToGameObjectsH;
  std::map<std::string, objid> nameToId;
  std::vector<LayerInfo> layers;
};

struct SceneDeserialization {
  Scene scene;
  std::map<std::string, std::map<std::string, std::string>>  additionalFields;
};

std::string serializeObject(Scene& scene, objid id, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds, std::string overrideName);
std::string serializeScene(Scene& scene, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds);
SceneDeserialization deserializeScene(std::string content, std::function<objid()> getNewObjectId, std::vector<LayerInfo> layers);

void addGameObjectToScene(Scene& scene, std::string name, GameObject& gameobjectObj, std::vector<std::string> children);
void addChildLink(Scene& scene, objid childId, objid parentId);

std::map<std::string,  std::map<std::string, std::string>> addSubsceneToRoot(
  Scene& scene, 
  objid rootId,
  objid rootIdNode, 
  std::map<objid, objid> childToParent, 
  std::map<objid, Transformation> gameobjTransforms, 
  std::map<objid, std::string> names, 
  std::map<objid, std::map<std::string, std::string>> additionalFields,
  std::function<objid()> getNewObjectId
);

std::vector<objid> idsToRemoveFromScenegraph(Scene& scene, objid);
void removeObjectsFromScenegraph(Scene& scene, std::vector<objid> objects);
std::vector<objid> listObjInScene(Scene& scene);

void traverseScene(Scene& scene, glm::mat4 initialModel, glm::vec3 totalScale, std::function<void(objid, glm::mat4, glm::mat4, bool, bool, std::string)> onObject, std::function<void(objid, glm::mat4, glm::vec3)> traverseLink);  

std::vector<objid> getIdsInGroup(Scene& scene, objid groupId);
GameObject& getGameObject(Scene& scene, objid id);
GameObject& getGameObject(Scene& scene, std::string name);
objid getGroupId(Scene& scene, objid id);
bool idExists(Scene& scene, objid id);
objid parentId(Scene& scene, objid id);

#endif

