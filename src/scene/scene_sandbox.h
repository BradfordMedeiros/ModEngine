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

void traverseScene(SceneSandbox& sandbox, Scene& scene, std::function<void(objid, glm::mat4, glm::mat4, bool, std::string, glm::vec3)> onObject);
Transformation fullTransformation(SceneSandbox& sandbox, objid id);

struct AddSceneDataValues {
  SceneDeserialization deserializedScene;
  std::vector<objid> idsAdded;
};
AddSceneDataValues addSceneDataToScenebox(SceneSandbox& sandbox, objid sceneId, std::string sceneData);

#endif 