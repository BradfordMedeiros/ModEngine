#ifndef MOD_SCENESANDBOX
#define MOD_SCENESANDBOX

#include <set>
#include <sstream>
#include <functional>
#include <unordered_set>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "./common/mesh.h"
#include "../common/util.h"
#include "./serialization.h"

struct GameObjectH {
  objid id;
  objid parentId;
  int parentDirectIndex;
  std::set<objid> children;
  std::set<int> childrenDirectIndex;
  objid groupId;       // grouping mechanism for nodes.  It is either its own id, or explicitly stated when created. 
  objid sceneId;
  objid depth = 0;
  std::optional<objid> prefabId;

  int updateFrame = -1;
  
  int updateAbsoluteFrame = -1;
  int updateAbsolutePosition = -1;
  int updateAbsoluteScale = -1;
  int updateAbsoluteRotation = -1;

};

struct TransformCacheElement {
  Transformation transform;
  glm::mat4 matrix;
};

struct GameObjectBuffer {
  bool inUse;
  TransformCacheElement absoluteTransform;
  GameObject gameobj;
  GameObjectH gameobjh;
  ObjTypeLookup lookup;
};

struct Scene {
  std::vector<GameObjectBuffer> gameobjects;
  std::unordered_map<objid, int> idToDirectIndex;

  std::unordered_map<objid, std::unordered_map<std::string, objid>> sceneToNameToId;
};

struct SceneDeserialization {
  Scene scene;
  std::unordered_map<std::string, GameobjAttributes> additionalFields;
  std::unordered_map<std::string, GameobjAttributes> subelementAttributes;
};

struct SceneMetadata {
  std::string scenefile;
  std::optional<std::string> name;
  std::vector<std::string> tags;
};

struct SceneSandbox {
  std::unordered_map<objid, objid> sceneIdToRootObj;
  std::unordered_map<objid, SceneMetadata> sceneIdToSceneMetadata;
  Scene mainScene;
  std::vector<LayerInfo> layers;

  std::set<objid> updatedIds;
};

TransformCacheElement& getAbsoluteById(SceneSandbox& sandbox, objid id);
TransformCacheElement& getAbsoluteByDirectIndex(SceneSandbox& sandbox, int directIndex);

std::vector<std::string> childnamesNoPrefabs(SceneSandbox& sandbox, GameObjectH& gameobjecth);
void addGameObjectToScene(SceneSandbox& sandbox, objid sceneId, std::string name, GameObject& gameobjectObj, std::vector<std::string> children, std::optional<objid> prefabId);

std::set<objid> idsToRemoveFromScenegraph(SceneSandbox& sandbox, objid);
void removeObjectsFromScenegraph(SceneSandbox& sandbox, std::set<objid> objects);
std::vector<objid> listObjInScene(SceneSandbox& sandbox, std::optional<objid> sceneId);
std::set<objid> listObjAndDescInScene(SceneSandbox& sandbox, objid sceneId);
std::set<objid> getChildrenIdsAndParent(Scene& scene,  objid id);
std::string serializeScene(SceneSandbox& sandbox, objid sceneId, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds);
///////////////////////////////

SceneSandbox createSceneSandbox(std::vector<LayerInfo> layers, std::function<std::set<std::string>(std::string&)> getObjautoserializerFields);
void forEveryGameobj(SceneSandbox& sandbox, std::function<void(objid id, GameObject& gameobj)> onElement);
std::vector<objid> allSceneIds(SceneSandbox& sandbox, std::optional<std::vector<std::string>> tags);
bool extractSceneIdFromName(std::string& name, objid* _id, std::string* _searchName);
std::optional<GameObject*> maybeGetGameObjectByName(SceneSandbox& sandbox, std::string& name, objid sceneId);
objid getGroupId(SceneSandbox& sandbox, objid id);
objid getIdForName(SceneSandbox& sandbox, std::string name, objid sceneId);
std::vector<objid> getIdsInGroupByObjId(SceneSandbox& sandbox, objid index);
bool idExists(SceneSandbox& sandbox, objid id);
bool idExists(SceneSandbox& sandbox, std::string name, objid sceneId);
GameObject& getGameObject(SceneSandbox& sandbox, objid id);
GameObject& getGameObject(SceneSandbox& sandbox, std::string name, objid sceneId);
GameObject& getGameObjectDirectIndex(SceneSandbox& sandbox, objid id);
ObjTypeLookup& getObjTypeLookup(SceneSandbox& sandbox, objid id);
int getDirectIndexForId(SceneSandbox& sandbox, objid id);

GameObjectH& getGameObjectH(Scene& scene, objid id);
GameObjectH& getGameObjectH(SceneSandbox& sandbox, objid id);
GameObjectH& getGameObjectH(SceneSandbox& sandbox, std::string name, objid sceneId);

glm::mat4 fullModelTransform(SceneSandbox& sandbox, objid id, const char* hint = NULL);
glm::mat4 fullModelTransformDirect(SceneSandbox& sandbox, objid directIndex, const char* hint = NULL);
Transformation& fullTransformation(SceneSandbox& sandbox, objid id, const char* hint = NULL);

void updateAllChildrenPositions(SceneSandbox& sandbox, objid updatedId);

struct GameobjAttributesWithId {
  objid id;
  GameobjAttributes attr;
};
struct AddSceneDataValues {
  std::unordered_map<std::string, GameobjAttributesWithId>  additionalFields;
  std::vector<objid> idsAdded;
  std::unordered_map<std::string, GameobjAttributes> subelementAttributes;
};
AddSceneDataValues addSceneDataToScenebox(SceneSandbox& sandbox, std::string sceneFileName, objid sceneId, std::string sceneData, std::optional<std::string> name, std::optional<std::vector<std::string>> tags, std::function<std::set<std::string>(std::string&)> getObjautoserializerFields, std::optional<objid> parentId, std::optional<objid> prefabId);
void removeScene(SceneSandbox& sandbox, objid sceneId);
bool sceneExists(SceneSandbox& sandbox, objid sceneId);

std::unordered_map<std::string, GameobjAttributesWithId> multiObjAdd(
  SceneSandbox& sandbox,
  objid sceneId,
  objid rootId,
  objid rootIdNode, 
  std::unordered_map<objid, objid> childToParent, 
  std::unordered_map<objid, Transformation> gameobjTransforms, 
  std::unordered_map<objid, std::string> names, 
  std::unordered_map<objid, GameobjAttributes> additionalFields,
  std::function<objid()> getNewObjectId,
  std::function<std::set<std::string>(std::string&)> getObjautoserializerFields,
  std::set<objid> boneIds,
  std::optional<objid> prefabId
);

void makeParent(SceneSandbox& sandbox, objid child, objid parent);
std::optional<objid> getParent(SceneSandbox& sandbox, objid id);

objid sceneId(SceneSandbox& sandbox, objid id);

std::vector<objid> getByName(SceneSandbox& sandbox, std::string name);
int getNumberOfObjects(SceneSandbox& sandbox);
int getNumberScenesLoaded(SceneSandbox& sandbox);
std::optional<objid> sceneIdByName(SceneSandbox& sandbox, std::string name);


void updateAbsoluteTransform(SceneSandbox& sandbox, objid id, Transformation transform, Hint hint);
void updateRelativeTransform(SceneSandbox& sandbox, objid id, Transformation transform, Hint hint, std::optional<int> directIndex);
void updateAbsolutePosition(SceneSandbox& sandbox, objid id, glm::vec3 position, Hint hint);
void updateRelativePosition(SceneSandbox& sandbox, objid id, glm::vec3 position, Hint hint);
void updateAbsoluteScale(SceneSandbox& sandbox, objid id, glm::vec3 scale, Hint hint);
void updateAbsoluteRotation(SceneSandbox& sandbox, objid id, glm::quat rotation, Hint hint);
void updateRelativeRotation(SceneSandbox& sandbox, objid id, glm::quat rotation, Hint hint);

std::set<objid> updateSandbox(SceneSandbox& sandbox);

Transformation calcRelativeTransform(Transformation& child, Transformation& parent);
Transformation calcRelativeTransform(SceneSandbox& sandbox, objid childId);

objid rootSceneId(SceneSandbox& sandbox);
bool transformLoggingEnabled();

#endif 