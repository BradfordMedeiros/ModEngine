#ifndef MOD_SCENESANDBOX
#define MOD_SCENESANDBOX

#include <map>
#include <vector>
#include <functional>
#include "./scenegraph.h"

struct SceneSandbox {
  std::map<objid, Scene> scenes;
  std::map<objid, objid> idToScene;
};

SceneSandbox createSceneSandbox();
void forEveryGameobj(SceneSandbox& sandbox, std::function<void(objid id, Scene& scene, GameObject& gameobj)> onElement);
std::vector<objid> allSceneIds(SceneSandbox& sandbox);
Scene& sceneForId(SceneSandbox& sandbox, objid id);
std::optional<GameObject*> maybeGetGameObjectByName(SceneSandbox& sandbox, std::string name);
objid getGroupId(SceneSandbox& sandbox, objid id);
std::vector<objid> getIdsInGroup(SceneSandbox& sandbox, objid index);
bool idExists(SceneSandbox& sandbox, objid id);
GameObject& getGameObject(SceneSandbox& sandbox, objid id);

void traverseScene(SceneSandbox& sandbox, Scene& scene, std::function<void(objid, glm::mat4, glm::mat4, bool, std::string)> onObject);
void traverseSandbox(SceneSandbox& sandbox, std::function<void(objid, glm::mat4, glm::mat4, bool, bool, std::string)> onObject);

glm::mat4 fullModelTransform(SceneSandbox& sandbox, objid id);
glm::mat4 groupModelTransform(SceneSandbox& sandbox, objid id);
Transformation fullTransformation(SceneSandbox& sandbox, objid id);

struct AddSceneDataValues {
  SceneDeserialization deserializedScene;
  std::vector<objid> idsAdded;
};
AddSceneDataValues addSceneDataToScenebox(SceneSandbox& sandbox, objid sceneId, std::string sceneData, std::vector<LayerInfo> layers);

void addLink(SceneSandbox& sandbox, objid childSceneId, objid id);
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