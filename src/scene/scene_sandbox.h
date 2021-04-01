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

////////////////////////

struct GameObjectH {
  objid id;
  objid parentId;
  std::set<objid> children;
  objid groupId;       // grouping mechanism for nodes.  It is either its own id, or explicitly stated when created. 
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
SceneDeserialization deserializeScene(std::string content, std::function<objid()> getNewObjectId, std::vector<LayerInfo> layers);
void addGameObjectToScene(Scene& scene, std::string name, GameObject& gameobjectObj, std::vector<std::string> children);

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

void traverseScene(Scene& scene, glm::mat4 initialModel, glm::vec3 totalScale, std::function<void(objid, glm::mat4, glm::mat4, bool, bool, std::string)> onObject);  

std::vector<objid> getIdsInGroup(Scene& scene, objid groupId);

/////////////////////////////
struct SceneSandbox {
  std::map<objid, Scene> scenes;
  std::map<objid, objid> idToScene;
};

//////////////////////////////

std::string serializeScene(SceneSandbox& sandbox, objid sceneId, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds);
///////////////////////////////



SceneSandbox createSceneSandbox();
void forEveryGameobj(SceneSandbox& sandbox, std::function<void(objid id, Scene& scene, GameObject& gameobj)> onElement);
std::vector<objid> allSceneIds(SceneSandbox& sandbox);
Scene& sceneForId(SceneSandbox& sandbox, objid id);
std::optional<GameObject*> maybeGetGameObjectByName(SceneSandbox& sandbox, std::string name);
objid getGroupId(SceneSandbox& sandbox, objid id);
std::vector<objid> getIdsInGroup(SceneSandbox& sandbox, objid index);
bool idExists(SceneSandbox& sandbox, objid id);
GameObject& getGameObject(SceneSandbox& sandbox, objid id);
GameObject& getGameObject(SceneSandbox& sandbox, std::string name);

void traverseScene(SceneSandbox& sandbox, Scene& scene, std::function<void(objid, glm::mat4, glm::mat4, bool, std::string)> onObject);
void traverseSandbox(SceneSandbox& sandbox, std::function<void(objid, glm::mat4, glm::mat4, bool, bool, std::string)> onObject);

glm::mat4 fullModelTransform(SceneSandbox& sandbox, objid id);
glm::mat4 armatureTransform(SceneSandbox& sandbox, objid id, std::string skeleton);
Transformation fullTransformation(SceneSandbox& sandbox, objid id);

struct AddSceneDataValues {
  SceneDeserialization deserializedScene;
  std::vector<objid> idsAdded;
};
AddSceneDataValues addSceneDataToScenebox(SceneSandbox& sandbox, objid sceneId, std::string sceneData, std::vector<LayerInfo> layers);

std::map<std::string,  std::map<std::string, std::string>> multiObjAdd(SceneSandbox& sandbox,
  objid rootId,
  objid rootIdNode, 
  std::map<objid, objid> childToParent, 
  std::map<objid, Transformation> gameobjTransforms, 
  std::map<objid, std::string> names, 
  std::map<objid, std::map<std::string, std::string>> additionalFields,
  std::function<objid()> getNewObjectId
);

#endif 