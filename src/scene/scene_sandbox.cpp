#include "./scene_sandbox.h"

SceneSandbox createSceneSandbox(){
  SceneSandbox sandbox {

  };
  return sandbox;
}

void forEveryGameobj(SceneSandbox& sandbox, std::function<void(objid id, Scene& scene, GameObject& gameobj)> onElement){
  for (auto &[_, scene] : sandbox.scenes){
    for (auto [id, gameObj]: scene.idToGameObjects){
      onElement(id, scene, gameObj);
    }
  }
}

std::vector<objid> allSceneIds(SceneSandbox& sandbox){
  std::vector<objid> sceneIds;
  for (auto [sceneId, _] : sandbox.scenes){
    sceneIds.push_back(sceneId);
  }
  return sceneIds;
} 

Scene& sceneForId(SceneSandbox& sandbox, objid id){
  return sandbox.scenes.at(sandbox.idToScene.at(id));
}

std::optional<GameObject*> maybeGetGameObjectByName(SceneSandbox& sandbox, std::string name){
  for (auto [sceneId, _] : sandbox.scenes){
    for (auto &[id, gameObj]: sandbox.scenes.at(sceneId).idToGameObjects){
      if (gameObj.name == name){
       return &gameObj;
      }
    }
  }
  return std::nullopt;
}

objid getGroupId(SceneSandbox& sandbox, objid id){
  return getGroupId(sceneForId(sandbox, id), id); 
}

std::vector<objid> getIdsInGroup(SceneSandbox& sandbox, objid index){
  return getIdsInGroup(sceneForId(sandbox, index), getGroupId(sandbox, index));
}

bool idExists(SceneSandbox& sandbox, objid id){
  return sandbox.idToScene.find(id) != sandbox.idToScene.end();
}
GameObject& getGameObject(SceneSandbox& sandbox, objid id){
  return getGameObject(sceneForId(sandbox, id), id);
}

void traverseScene(SceneSandbox& sandbox, Scene& scene, glm::mat4 initialModel, glm::vec3 scale, std::function<void(objid, glm::mat4, glm::mat4, bool, std::string)> onObject){
  traverseScene(scene, initialModel, scale, onObject, [&sandbox, &scene, &onObject](objid id, glm::mat4 modelMatrix, glm::vec3 scale) -> void {
      Scene& linkScene = sandbox.scenes.at(sandbox.idToScene.at(id));
      traverseScene(sandbox, linkScene, modelMatrix, scale, onObject);
  });
}

void traverseScene(SceneSandbox& sandbox, Scene& scene, std::function<void(objid, glm::mat4, glm::mat4, bool, std::string)> onObject){
  traverseScene(sandbox, scene, glm::mat4(1.f), glm::vec3(1.f, 1.f, 1.f), onObject);
}

Transformation fullTransformation(SceneSandbox& sandbox, objid id){
  Scene& scene = sceneForId(sandbox, id);
  while(scene.isNested){
    scene = sceneForId(sandbox, parentId(scene, scene.rootId));
  }
  
  Transformation transformation = {};
  bool foundId = false;
  
  traverseScene(sandbox, scene, [id, &foundId, &transformation](objid traversedId, glm::mat4 model, glm::mat4 parent, bool isOrtho, std::string fragshader) -> void {
    if (traversedId == id){
      foundId = true;
      transformation = getTransformationFromMatrix(model);
    }
  });
  assert(foundId);
  return transformation;
}

AddSceneDataValues addSceneDataToScenebox(SceneSandbox& sandbox, objid sceneId, std::string sceneData){
  assert(sandbox.scenes.find(sceneId) == sandbox.scenes.end());
  SceneDeserialization deserializedScene = deserializeScene(sceneData, getUniqueObjId);
  sandbox.scenes[sceneId] = deserializedScene.scene;
  std::vector<objid> idsAdded;
  for (auto &[id, _] :  sandbox.scenes.at(sceneId).idToGameObjects){
    idsAdded.push_back(id);
  }
  AddSceneDataValues data  {
    .deserializedScene = deserializedScene,
    .idsAdded = idsAdded,
  };
  return data;
}

