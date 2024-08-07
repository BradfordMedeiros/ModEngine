#ifndef MOD_SCENESANDBOX
#define MOD_SCENESANDBOX

#include <set>
#include <sstream>
#include <functional>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "./common/mesh.h"
#include "../common/util.h"
#include "./serialization.h"

struct GameObjectH {
  objid id;
  std::optional<objid> parentId;
  std::set<objid> children;
  objid groupId;       // grouping mechanism for nodes.  It is either its own id, or explicitly stated when created. 
  objid sceneId;
};

struct TransformCacheElement {
  Transformation transform;
};

struct Scene {
  objid rootId;
  std::map<objid, GameObject> idToGameObjects;
  std::map<objid, GameObjectH> idToGameObjectsH;
  std::map<objid, std::map<std::string, objid>> sceneToNameToId;
  std::unordered_map<objid, TransformCacheElement> absoluteTransforms; 
};

// no parent:
// add ot absolute transforms (as value = position)

// with parent 
// add to constraint
// add to absolute transforms (as value = position + contstaint)
// when update position of anything -> update absolute transform

struct SceneDeserialization {
  Scene scene;
  std::map<std::string, GameobjAttributes> additionalFields;
  std::map<std::string, GameobjAttributes> subelementAttributes;
};

struct SceneMetadata {
  std::string scenefile;
  std::optional<std::string> name;
  std::vector<std::string> tags;
};

struct SceneSandbox {
  std::map<objid, objid> sceneIdToRootObj;
  std::map<objid, SceneMetadata> sceneIdToSceneMetadata;
  Scene mainScene;
  std::vector<LayerInfo> layers;

  std::set<objid> updatedIds;
};

std::vector<std::string> childnames(SceneSandbox& sandbox, GameObjectH& gameobjecth);
void addGameObjectToScene(SceneSandbox& sandbox, objid sceneId, std::string name, GameObject& gameobjectObj, std::vector<std::string> children);

std::vector<objid> idsToRemoveFromScenegraph(SceneSandbox& sandbox, objid);
void removeObjectsFromScenegraph(SceneSandbox& sandbox, std::vector<objid> objects);
std::vector<objid> listObjInScene(SceneSandbox& sandbox, std::optional<objid> sceneId);
std::string serializeScene(SceneSandbox& sandbox, objid sceneId, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds);
///////////////////////////////

SceneSandbox createSceneSandbox(std::vector<LayerInfo> layers, std::function<std::set<std::string>(std::string&)> getObjautoserializerFields);
void forEveryGameobj(SceneSandbox& sandbox, std::function<void(objid id, GameObject& gameobj)> onElement);
std::vector<objid> allSceneIds(SceneSandbox& sandbox, std::optional<std::vector<std::string>> tags);
bool extractSceneIdFromName(std::string& name, objid* _id, std::string* _searchName);
std::optional<GameObject*> maybeGetGameObjectByName(SceneSandbox& sandbox, std::string name, objid sceneId, bool enablePrefixMatch);
objid getGroupId(SceneSandbox& sandbox, objid id);
objid getIdForName(SceneSandbox& sandbox, std::string name, objid sceneId);
std::vector<objid> getIdsInGroupByObjId(SceneSandbox& sandbox, objid index);
bool idExists(SceneSandbox& sandbox, objid id);
bool idExists(SceneSandbox& sandbox, std::string name, objid sceneId);
GameObject& getGameObject(SceneSandbox& sandbox, objid id);
GameObject& getGameObject(SceneSandbox& sandbox, std::string name, objid sceneId);
GameObjectH& getGameObjectH(SceneSandbox& sandbox, objid id);
GameObjectH& getGameObjectH(SceneSandbox& sandbox, std::string name, objid sceneId);

void traverseSandboxByLayer(SceneSandbox& sandbox, std::function<void(objid, glm::mat4, glm::mat4, LayerInfo&, std::string)> onObject);

glm::mat4 fullModelTransform(SceneSandbox& sandbox, objid id);
Transformation fullTransformation(SceneSandbox& sandbox, objid id);

struct GameobjAttributesWithId {
  objid id;
  GameobjAttributes attr;
};
struct AddSceneDataValues {
  std::map<std::string, GameobjAttributesWithId>  additionalFields;
  std::vector<objid> idsAdded;
  std::map<std::string, GameobjAttributes> subelementAttributes;
};
AddSceneDataValues addSceneDataToScenebox(SceneSandbox& sandbox, std::string sceneFileName, objid sceneId, std::string sceneData, std::optional<std::string> name, std::optional<std::vector<std::string>> tags, std::function<std::set<std::string>(std::string&)> getObjautoserializerFields, std::optional<objid> parentId);
void removeScene(SceneSandbox& sandbox, objid sceneId);
bool sceneExists(SceneSandbox& sandbox, objid sceneId);

std::map<std::string, GameobjAttributesWithId> multiObjAdd(
  SceneSandbox& sandbox,
  objid sceneId,
  objid rootId,
  objid rootIdNode, 
  std::map<objid, objid> childToParent, 
  std::map<objid, Transformation> gameobjTransforms, 
  std::map<objid, std::string> names, 
  std::map<objid, GameobjAttributes> additionalFields,
  std::function<objid()> getNewObjectId,
  std::function<std::set<std::string>(std::string&)> getObjautoserializerFields
);

void makeParent(SceneSandbox& sandbox, objid child, objid parent);
std::optional<objid> getParent(SceneSandbox& sandbox, objid id);

objid sceneId(SceneSandbox& sandbox, objid id);

std::vector<objid> getByName(SceneSandbox& sandbox, std::string name);
int getNumberOfObjects(SceneSandbox& sandbox);
int getNumberScenesLoaded(SceneSandbox& sandbox);
std::optional<objid> sceneIdByName(SceneSandbox& sandbox, std::string name);

void updateAbsoluteTransform(SceneSandbox& sandbox, objid id, Transformation transform);
void updateRelativeTransform(SceneSandbox& sandbox, objid id, Transformation transform);
void updateAbsolutePosition(SceneSandbox& sandbox, objid id, glm::vec3 position);
void updateRelativePosition(SceneSandbox& sandbox, objid id, glm::vec3 position);
void updateAbsoluteScale(SceneSandbox& sandbox, objid id, glm::vec3 scale);
void updateRelativeScale(SceneSandbox& sandbox, objid id, glm::vec3 scale);
void updateAbsoluteRotation(SceneSandbox& sandbox, objid id, glm::quat rotation);
void updateRelativeRotation(SceneSandbox& sandbox, objid id, glm::quat rotation);

std::set<objid> updateSandbox(SceneSandbox& sandbox);

Transformation calcRelativeTransform(Transformation& child, Transformation& parent);
Transformation calcRelativeTransform(SceneSandbox& sandbox, objid childId);

objid rootSceneId(SceneSandbox& sandbox);

#endif 