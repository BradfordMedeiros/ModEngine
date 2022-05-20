#ifndef MOD_SCENESANDBOX
#define MOD_SCENESANDBOX

#include <map>
#include <vector>
#include <functional>

////////////
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
#include "./styles.h"

struct GameObjectH {
  objid id;
  objid parentId;
  std::set<objid> children;
  objid groupId;       // grouping mechanism for nodes.  It is either its own id, or explicitly stated when created. 
  objid sceneId;
};

enum TransformUpdateType { UPDATE_NONE, UPDATE_ABSOLUTE, UPDATE_RECALC_RELATIVE };
struct TransformCacheElement {
  Transformation transform;
  TransformUpdateType updateType;
};

struct Scene {
  objid rootId;
  std::map<objid, GameObject> idToGameObjects;
  std::map<objid, GameObjectH> idToGameObjectsH;
  std::map<objid, std::map<std::string, objid>> sceneToNameToId;

  std::map<objid, TransformCacheElement> absoluteTransforms;
};

struct SceneDeserialization {
  Scene scene;
  std::map<std::string, GameobjAttributes> additionalFields;
  std::map<std::string, GameobjAttributes> subelementAttributes;
};

struct SceneMetadata {
  std::string scenefile;
  std::optional<std::string> name;
};

struct SceneSandbox {
  std::map<objid, objid> sceneIdToRootObj;
  std::map<objid, SceneMetadata> sceneIdToSceneMetadata;
  Scene mainScene;
  std::vector<LayerInfo> layers;
};

std::vector<std::string> childnames(SceneSandbox& sandbox, GameObjectH& gameobjecth);
std::string serializeObjectSandbox(GameObject& gameobj, objid id, objid groupId, std::vector<std::pair<std::string, std::string>> getAdditionalFields, std::vector<std::string>& children, bool includeIds, std::string overrideName);
void addGameObjectToScene(SceneSandbox& sandbox, objid sceneId, std::string name, GameObject& gameobjectObj, std::vector<std::string> children);

std::vector<objid> idsToRemoveFromScenegraph(SceneSandbox& sandbox, objid);
void maybePruneScenes(SceneSandbox& sandbox);
void removeObjectsFromScenegraph(SceneSandbox& sandbox, std::vector<objid> objects);
std::vector<objid> listObjInScene(SceneSandbox& sandbox, objid sceneId);
std::string serializeScene(SceneSandbox& sandbox, objid sceneId, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds);
///////////////////////////////

SceneSandbox createSceneSandbox(std::vector<LayerInfo> layers);
void forEveryGameobj(SceneSandbox& sandbox, std::function<void(objid id, GameObject& gameobj)> onElement);
std::vector<objid> allSceneIds(SceneSandbox& sandbox);
bool extractSceneIdFromName(std::string& name, objid* _id, std::string* _searchName);
std::optional<GameObject*> maybeGetGameObjectByName(SceneSandbox& sandbox, std::string name, objid sceneId, bool enablePrefixMatch);
objid getGroupId(SceneSandbox& sandbox, objid id);
objid getIdForName(SceneSandbox& sandbox, std::string name, objid sceneId);
std::vector<objid> getIdsInGroup(SceneSandbox& sandbox, objid index);
bool idExists(SceneSandbox& sandbox, objid id);
bool idExists(SceneSandbox& sandbox, std::string name, objid sceneId);
GameObject& getGameObject(SceneSandbox& sandbox, objid id);
GameObject& getGameObject(SceneSandbox& sandbox, std::string name, objid sceneId);
GameObjectH& getGameObjectH(SceneSandbox& sandbox, objid id);
GameObjectH& getGameObjectH(SceneSandbox& sandbox, std::string name, objid sceneId);

void traverseSandbox(SceneSandbox& sandbox, std::function<void(objid, glm::mat4, glm::mat4, LayerInfo&, std::string)> onObject);

glm::mat4 fullModelTransform(SceneSandbox& sandbox, objid id);
Transformation fullTransformation(SceneSandbox& sandbox, objid id);
glm::mat4 armatureTransform(SceneSandbox& sandbox, objid id, std::string skeleton, objid sceneId);

struct GameobjAttributesWithId {
  objid id;
  GameobjAttributes attr;
};
struct AddSceneDataValues {
  std::map<std::string, GameobjAttributesWithId>  additionalFields;
  std::vector<objid> idsAdded;
  std::map<std::string, GameobjAttributes> subelementAttributes;
};
AddSceneDataValues addSceneDataToScenebox(SceneSandbox& sandbox, std::string sceneFileName, objid sceneId, std::string sceneData, std::vector<Style>& styles, std::optional<std::string> name);
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
  std::function<objid()> getNewObjectId
);

void makeParent(SceneSandbox& sandbox, objid child, objid parent);
objid rootIdForScene(SceneSandbox& sandbox, objid sceneId);
objid sceneId(SceneSandbox& sandbox, objid id);
bool parentSceneId(SceneSandbox& sandbox, objid sceneId, objid* _parentSceneId);
std::vector<objid> childSceneIds(SceneSandbox& sandbox, objid sceneId);

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

void updateSandbox(SceneSandbox& sandbox);

#endif 