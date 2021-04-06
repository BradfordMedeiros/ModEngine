#include "./scene_sandbox.h"

objid addObjectToScene(Scene& scene, objid sceneId, objid parentId, std::string name, GameObject& gameobjectObj){
  assert(name == gameobjectObj.name);
  auto gameobjectH = GameObjectH {
    .id = gameobjectObj.id,
    .parentId = parentId,
    .groupId = gameobjectObj.id,
    .sceneId = sceneId,
  };
  assert(scene.idToGameObjectsH.find(gameobjectObj.id) == scene.idToGameObjectsH.end());
  scene.idToGameObjectsH[gameobjectObj.id] = gameobjectH;
  scene.idToGameObjects[gameobjectObj.id] = gameobjectObj;
  if (!(scene.nameToId.find(name) == scene.nameToId.end())){
    std::cout << "name already exists: " << name << std::endl;
    assert(false);
  }
  scene.nameToId[name] = gameobjectObj.id;
  return gameobjectObj.id;
}

// @TODO - bug around having multiple children in common.  Currently assertion error
void enforceParentRelationship(Scene& scene, objid id, std::string parentName){
  auto gameobj = scene.idToGameObjectsH.at(id);
  auto parentId = scene.nameToId.at(parentName);
  scene.idToGameObjectsH.at(id).parentId = parentId;
  scene.idToGameObjectsH.at(parentId).children.insert(id);
}

void enforceRootObjects(Scene& scene){
  scene.idToGameObjectsH.at(scene.rootId).children.clear();
  for (auto &[id, objh] : scene.idToGameObjectsH){
    if ((objh.parentId == -1 || objh.parentId == scene.rootId) && id != scene.rootId){
      enforceParentRelationship(scene, id, scene.idToGameObjects.at(scene.rootId).name);
    }
  }
}

GameobjAttributes rootGameObject(){
  return GameobjAttributes {
    .stringAttributes = {{"physics", "disabled"}}
  };
}

SceneDeserialization createSceneFromParsedContent(
  objid sceneId,
  std::vector<Token> tokens,  
  std::function<objid()> getNewObjectId
){
  Scene scene;

  auto serialGameAttrs = deserializeSceneTokens(tokens);

  auto rootId = getNewObjectId();
  auto rootName = "~root:" + std::to_string(rootId);
  scene.rootId = rootId;

  assert(serialGameAttrs.find(rootName) == serialGameAttrs.end());
  assert(rootName.find(',') == std::string::npos);
  serialGameAttrs[rootName] = rootGameObject();

  std::map<std::string, GameObject> gameobjs;

  for (auto [name, gameAttr] : serialGameAttrs){
    if (name != rootName){
      objid id = (gameAttr.stringAttributes.find("id") != gameAttr.stringAttributes.end()) ? 
        std::atoi(gameAttr.stringAttributes.at("id").c_str()) : 
        getNewObjectId();
      gameobjs[name] = gameObjectFromFields(name, id, gameAttr);
    }else{
      gameobjs[name] = gameObjectFromFields(name, scene.rootId, gameAttr); 
    }
  }

  for (auto [name, gameobjectObj] : gameobjs){
    auto addedId = addObjectToScene(scene, sceneId, -1, name, gameobjectObj);
  }

  for (auto [name, gameobj] : serialGameAttrs){
    for (auto childName : gameobj.children){
      enforceParentRelationship(scene, scene.nameToId.at(childName), name);
    }
  }
  enforceRootObjects(scene);

  std::map<std::string, std::map<std::string, std::string>> additionalFields;

  for (auto &[name, gameobj] : serialGameAttrs){
    additionalFields[name] = gameobj.additionalFields;
  }

  SceneDeserialization deserializedScene {
    .scene = scene,
    .additionalFields = additionalFields,
  };
  return deserializedScene;
}

GameobjAttributes defaultAttributesForMultiObj(Transformation transform, GameObject& gameobj){
  GameobjAttributes attributes {
    .stringAttributes = {
      {"fragshader", gameobj.fragshader},
      {"layer", gameobj.layer},
    },
    .vecAttributes = {
      {"position", transform.position },
      {"scale",    transform.scale    },
    },
  };
  return attributes;
}

std::map<std::string,  std::map<std::string, std::string>> addSubsceneToRoot(
  Scene& scene, 
  objid sceneId,
  objid rootId, 
  objid rootIdNode, 
  std::map<objid, objid> childToParent, 
  std::map<objid, Transformation> gameobjTransforms, 
  std::map<objid, std::string> names,
  std::map<objid, std::map<std::string, std::string>> additionalFields,
  std::function<objid()> getNewObjectId
){
  std::map<std::string,  std::map<std::string, std::string>> nameToAdditionalFields;
  std::map<objid, objid> nodeIdToRealId;
  auto rootObj = scene.idToGameObjects.at(rootId);

  for (auto [nodeId, transform] : gameobjTransforms){
    if (nodeId == rootIdNode){
      continue;
    }
    objid id = getNewObjectId();
    nodeIdToRealId[nodeId] = id;

    nameToAdditionalFields[names.at(nodeId)] = additionalFields.at(nodeId);

    auto gameobj = gameObjectFromFields(names.at(nodeId), id, defaultAttributesForMultiObj(transform, rootObj));
    gameobj.transformation.rotation = transform.rotation; // todo make this work w/ attributes better

    addObjectToScene(scene, sceneId, -1, names.at(nodeId), gameobj);
    scene.idToGameObjectsH.at(id).groupId = rootId;
  }

  for (auto [childId, parentId] : childToParent){
    auto realChildId = nodeIdToRealId.at(childId);
    auto realParentId = parentId == rootIdNode ? rootId : nodeIdToRealId.at(parentId);
    enforceParentRelationship(scene, realChildId, scene.idToGameObjects.at(realParentId).name);
  }
  enforceRootObjects(scene);
  return nameToAdditionalFields;
} 

std::vector<std::string> childnames(SceneSandbox& sandbox, GameObjectH& gameobjecth){   
  std::vector<std::string> childnames;
  for (auto childid : gameobjecth.children){
    auto childH = getGameObjectH(sandbox, childid);
    if (childH.groupId == childid){
      childnames.push_back(getGameObject(sandbox, childid).name);
    }
  }
  return childnames;
}

std::string serializeObject(SceneSandbox& sandbox, objid id, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds, std::string overridename){
  auto gameobjecth = getGameObjectH(sandbox, id);;
  auto gameobj = getGameObject(sandbox, id);
  auto objectSerialization = serializeObj(
    id, 
    gameobjecth.groupId, 
    gameobj, 
    childnames(sandbox, gameobjecth), 
    includeIds, 
    getAdditionalFields(id), 
    overridename
  );
  return objectSerialization;
}

void addGameObjectToScene(SceneSandbox& sandbox, objid sceneId, std::string name, GameObject& gameobjectObj, std::vector<std::string> children){
   // @TODO - this is a bug sort of.  If this layer does not exist in the scene already, it should be added. 
  // Result if it doesn't exist is that it just doesn't get rendered, so nbd, but it really probably should be rendered (probably as a new layer with max depth?)
  auto addedId = addObjectToScene(sandbox.mainScene, sceneId, -1, name, gameobjectObj);      

  for (auto child : children){
    if (sandbox.mainScene.nameToId.find(child) == sandbox.mainScene.nameToId.end()){
       // @TODO - shouldn't be an error should automatically create instead
      std::cout << "ERROR: NOT YET IMPLEMENTED : ADDING OBJECT WITH CHILD THAT DOES NOT EXIST IN THE SCENE" << std::endl;
      assert(false);
    }
    enforceParentRelationship(sandbox.mainScene, sandbox.mainScene.nameToId.at(child), name);  
  }
  enforceRootObjects(sandbox.mainScene);
}

void traverseNodes(Scene& scene, objid id, std::function<void(objid)> onAddObject){
  auto parentObjH = scene.idToGameObjectsH.at(id);
  onAddObject(parentObjH.id);
  for (objid id : parentObjH.children){
    traverseNodes(scene, id, onAddObject);
  }
}

std::vector<objid> getChildrenIdsAndParent(Scene& scene,  objid id){
  std::vector<objid> objectIds;
  auto onAddObject = [&objectIds](objid id) -> void {
    objectIds.push_back(id);
  };
  traverseNodes(scene, id, onAddObject);
  return objectIds;
}

// Conceptually needs => get all ids for this scene starting from this id
std::vector<objid> idsToRemoveFromScenegraph(SceneSandbox& sandbox, objid id){
  auto objects = getChildrenIdsAndParent(sandbox.mainScene, id);
  assert(id != sandbox.mainScene.rootId);
  return objects;
}


void removeObjectsFromScenegraph(SceneSandbox& sandbox, std::vector<objid> objects){  
  for (auto id : objects){
    Scene& scene = sandbox.mainScene;
    std::string objectName = scene.idToGameObjects.at(id).name;
    scene.idToGameObjects.erase(id);
    scene.idToGameObjectsH.erase(id);
    scene.nameToId.erase(objectName);
    for (auto &[_, objh] : scene.idToGameObjectsH){  
      objh.children.erase(id);
    }
  }
}

std::vector<objid> listObjInScene(SceneSandbox& sandbox, objid sceneId){
  std::vector<objid> allObjects;
  for (auto const&[id, obj] : sandbox.mainScene.idToGameObjectsH){
    if (obj.sceneId == sceneId){
      allObjects.push_back(id);
    }
  }
  return allObjects;
}

void traverseScene(objid id, GameObjectH objectH, Scene& scene, glm::mat4 model, glm::vec3 totalScale, std::function<void(objid, glm::mat4, glm::mat4, std::string)> onObject){
  GameObject object = scene.idToGameObjects.at(objectH.id);
  glm::mat4 modelMatrix = glm::translate(model, object.transformation.position);
  modelMatrix = modelMatrix * glm::toMat4(object.transformation.rotation);
  
  glm::vec3 scaling = object.transformation.scale * totalScale;  // having trouble with doing the scaling here so putting out of band.   Anyone in the ether please help if more elegant. 
  glm::mat4 scaledModelMatrix = modelMatrix * glm::scale(glm::mat4(1.f), scaling);

  onObject(id, scaledModelMatrix, model, "");

  for (objid id: objectH.children){
    traverseScene(id, scene.idToGameObjectsH.at(id), scene, modelMatrix, scaling, onObject);
  }
}

struct traversalData {
  objid id;
  glm::mat4 modelMatrix;
  glm::mat4 parentMatrix;
};
void traverseScene(Scene& scene, std::vector<LayerInfo> layers, glm::mat4 initialModel, glm::vec3 totalScale, std::function<void(objid, glm::mat4, glm::mat4, bool, bool, std::string)> onObject){
  std::vector<traversalData> datum;

  objid id = scene.rootId;
  traverseScene(id, scene.idToGameObjectsH.at(id), scene, initialModel, totalScale, [&datum](objid foundId, glm::mat4 modelMatrix, glm::mat4 parentMatrix, std::string) -> void {
    datum.push_back(traversalData{
      .id = foundId,
      .modelMatrix = modelMatrix,
      .parentMatrix = parentMatrix,
    });
  });
  

  for (auto layer : layers){      // @TODO could organize this before to not require pass on each frame
    for (auto data : datum){
      auto gameobject = scene.idToGameObjects.at(data.id);
      if (gameobject.layer == layer.name){
        onObject(data.id, data.modelMatrix, data.parentMatrix, layer.orthographic, layer.ignoreDepthBuffer, gameobject.fragshader);
      }
    }  
  }
}

std::vector<objid> getIdsInGroup(Scene& scene, objid groupId){
  std::vector<objid> ids;
  for (auto [_, gameobj] : scene.idToGameObjectsH){
    if (gameobj.groupId == groupId){
      ids.push_back(gameobj.id);
    }
  }
  return ids;
}

GameObject& getGameObject(Scene& scene, objid id){
  return scene.idToGameObjects.at(id);
}
GameObject& getGameObject(Scene& scene, std::string name){
  return scene.idToGameObjects.at(scene.nameToId.at(name));
}
GameObjectH& getGameObjectH(Scene& scene, objid id){
  return scene.idToGameObjectsH.at(id);
}
objid getGroupId(Scene& scene, objid id){
  return scene.idToGameObjectsH.at(id).groupId; 
}
objid getIdForName(SceneSandbox& sandbox, std::string name){
  auto gameobj = maybeGetGameObjectByName(sandbox, name);
  return gameobj.value() -> id;
}
bool idExists(Scene& scene, objid id){
  return scene.idToGameObjects.find(id) != scene.idToGameObjects.end();
}
objid parentId(Scene& scene, objid id){
  return scene.idToGameObjectsH.at(id).parentId;
}

std::string serializeScene(SceneSandbox& sandbox, objid sceneId, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds){
  std::string sceneData = "# Generated scene \n";
  for (auto [id, obj]: sandbox.mainScene.idToGameObjectsH){
    if (obj.sceneId != sceneId){
      continue;
    }
    if (id == sandbox.mainScene.rootId){
      continue;
    }
    sceneData = sceneData + serializeObject(sandbox, id, getAdditionalFields, includeIds, "");
  }
  return sceneData;
}

SceneSandbox createSceneSandbox(std::vector<LayerInfo> layers){
  Scene mainScene {
    .rootId = 0,
  };
  std::sort(std::begin(layers), std::end(layers), [](LayerInfo layer1, LayerInfo layer2) { return layer1.zIndex < layer2.zIndex; });

  auto rootObj = gameObjectFromFields("root", mainScene.rootId, rootGameObject()); 
  addObjectToScene(mainScene, 0, -1, rootObj.name, rootObj);

  SceneSandbox sandbox {
    .mainScene = mainScene,
    .layers = layers,
  };
  return sandbox;
}

void forEveryGameobj(SceneSandbox& sandbox, std::function<void(objid id, GameObject& gameobj)> onElement){
  for (auto [id, gameObj]: sandbox.mainScene.idToGameObjects){
    onElement(id, gameObj); 
  }
}

std::vector<objid> allSceneIds(SceneSandbox& sandbox){
  std::vector<objid> sceneIds;
  for (auto [sceneId, _] : sandbox.sceneIdToRootObj){
    sceneIds.push_back(sceneId);
  }
  return sceneIds;
} 



std::optional<GameObject*> maybeGetGameObjectByName(SceneSandbox& sandbox, std::string name){
  for (auto &[id, gameObj]: sandbox.mainScene.idToGameObjects){
    if (gameObj.name == name){
      return &gameObj;      
    }
  }
  return std::nullopt;
}

objid getGroupId(SceneSandbox& sandbox, objid id){
  return getGroupId(sandbox.mainScene, id); 
}

std::vector<objid> getIdsInGroup(SceneSandbox& sandbox, objid index){
  return getIdsInGroup(sandbox.mainScene, getGroupId(sandbox, index));
}

bool idExists(SceneSandbox& sandbox, objid id){
  return sandbox.mainScene.idToGameObjects.find(id) != sandbox.mainScene.idToGameObjects.end();
}
bool idExists(SceneSandbox& sandbox, std::string name){
  return maybeGetGameObjectByName(sandbox, name).has_value();
}
GameObject& getGameObject(SceneSandbox& sandbox, objid id){
  return getGameObject(sandbox.mainScene, id);
}
GameObject& getGameObject(SceneSandbox& sandbox, std::string name){
  auto gameobj = maybeGetGameObjectByName(sandbox, name);
  GameObject& obj = *(gameobj.value()); 
  return obj;
}
GameObjectH& getGameObjectH(SceneSandbox& sandbox, objid id){
  return getGameObjectH(sandbox.mainScene, id);
}
GameObjectH& getGameObjectH(SceneSandbox& sandbox, std::string name){
  auto gameobj = maybeGetGameObjectByName(sandbox, name);
  GameObject& obj = *(gameobj.value()); 
  GameObjectH& objh = getGameObjectH(sandbox, obj.id);
  return objh;
}

void traverseScene(SceneSandbox& sandbox, Scene& scene, glm::mat4 initialModel, glm::vec3 scale, std::function<void(objid, glm::mat4, glm::mat4, bool, bool, std::string)> onObject){
  traverseScene(scene, sandbox.layers, initialModel, scale, onObject);
}

void traverseScene(SceneSandbox& sandbox, Scene& scene, std::function<void(objid, glm::mat4, glm::mat4, bool, bool, std::string)> onObject){
  traverseScene(sandbox, scene, glm::mat4(1.f), glm::vec3(1.f, 1.f, 1.f), onObject);
}

void traverseSandbox(SceneSandbox& sandbox, std::function<void(objid, glm::mat4, glm::mat4, bool, bool, std::string)> onObject){
  traverseScene(sandbox, sandbox.mainScene, onObject);
}

glm::mat4 fullModelTransform(SceneSandbox& sandbox, objid id){
  Scene& scene = sandbox.mainScene;
  glm::mat4 transformation = {};
  bool foundId = false;
  
  traverseScene(sandbox, scene, [id, &foundId, &transformation](objid traversedId, glm::mat4 model, glm::mat4 parent, bool isOrtho, bool ignoreDepth, std::string fragshader) -> void {
    if (traversedId == id){
      foundId = true;
      transformation = model;
    }
  });
  assert(foundId);
  return transformation;
}
glm::mat4 armatureTransform(SceneSandbox& sandbox, objid id, std::string skeletonRoot){
  auto gameobj = maybeGetGameObjectByName(sandbox, skeletonRoot);
  assert(gameobj.has_value());
 
  auto groupTransform = fullModelTransform(sandbox, gameobj.value() -> id);
  auto modelTransform = fullModelTransform(sandbox, id);
  // group * something = model (aka aX = b, so X = inv(A) * B)
  // inverse(group) * model
  auto groupToModel =  inverse(groupTransform) * modelTransform;

  auto resultCheck = groupTransform * groupToModel;
  if (false && resultCheck != modelTransform){
    std::cout << "result_check = " << print(resultCheck) << std::endl;
    std::cout << "model_transform = " << print(modelTransform) << std::endl;
    assert(false);

  }
  return groupToModel;
}

Transformation fullTransformation(SceneSandbox& sandbox, objid id){
  return getTransformationFromMatrix(fullModelTransform(sandbox, id));
}

SceneDeserialization deserializeScene(objid sceneId, std::string content, std::function<objid()> getNewObjectId){
  std::cout << "INFO: Deserialization: " << std::endl;
  return createSceneFromParsedContent(sceneId, parseFormat(content), getNewObjectId);
}
AddSceneDataValues addSceneDataToScenebox(SceneSandbox& sandbox, objid sceneId, std::string sceneData){
  assert(sandbox.sceneIdToRootObj.find(sceneId) == sandbox.sceneIdToRootObj.end());

  SceneDeserialization deserializedScene = deserializeScene(sceneId, sceneData, getUniqueObjId);

  for (auto &[id, obj] : deserializedScene.scene.idToGameObjects){
    sandbox.mainScene.idToGameObjects[id] = obj;
  }
  for (auto &[id, obj] : deserializedScene.scene.idToGameObjectsH){
    sandbox.mainScene.idToGameObjectsH[id] = obj;
  }
  for (auto &[name, id] : deserializedScene.scene.nameToId){
    sandbox.mainScene.nameToId[name] = id;
  }
  auto rootId = deserializedScene.scene.rootId;
  sandbox.sceneIdToRootObj[deserializedScene.scene.idToGameObjectsH.at(rootId).sceneId] = rootId;
  enforceParentRelationship(sandbox.mainScene, rootId, "root");

  std::vector<objid> idsAdded;
  for (auto &[id, obj] :  sandbox.mainScene.idToGameObjectsH){
    if (obj.sceneId == sceneId){
      idsAdded.push_back(id);
    }
  }
  AddSceneDataValues data  {
    .additionalFields = deserializedScene.additionalFields,
    .idsAdded = idsAdded,
  };
  return data;
}

// This should find the root of the scene and remove that element + all children (clean up empty scenes i guess? -> but would require unloading the scene!) 
void removeScene(SceneSandbox& sandbox, objid sceneId){
  removeObjectsFromScenegraph(sandbox,listObjInScene(sandbox, sceneId));
  sandbox.sceneIdToRootObj.erase(sceneId);
}
bool sceneExists(SceneSandbox& sandbox, objid sceneId){
  return !(sandbox.sceneIdToRootObj.find(sceneId) == sandbox.sceneIdToRootObj.end());
}    

std::map<std::string,  std::map<std::string, std::string>> multiObjAdd(
  SceneSandbox& sandbox,
  objid sceneId,
  objid rootId,
  objid rootIdNode, 
  std::map<objid, objid> childToParent, 
  std::map<objid, Transformation> gameobjTransforms, 
  std::map<objid, std::string> names, 
  std::map<objid, std::map<std::string, std::string>> additionalFields,
  std::function<objid()> getNewObjectId){
  auto nameToAdditionalFields = addSubsceneToRoot(sandbox.mainScene, sceneId, rootId, rootIdNode, childToParent, gameobjTransforms, names, additionalFields, getNewObjectId);
  return nameToAdditionalFields;
}

void makeParent(SceneSandbox& sandbox, objid child, objid parent){
  assert(child != parent);

  GameObjectH& parentObjH = getGameObjectH(sandbox, parent);
  GameObjectH& childObjH = getGameObjectH(sandbox, child);
  assert(childObjH.id == childObjH.groupId);

  auto allDescendents = getChildrenIdsAndParent(sandbox.mainScene,  child);
  for (auto decendentId : allDescendents){
    assert(decendentId != parent);
  }

  auto oldParentId = childObjH.parentId;
  GameObjectH& oldParentH = getGameObjectH(sandbox, oldParentId);

  oldParentH.children.erase(childObjH.id);
  parentObjH.children.insert(childObjH.id);
  childObjH.parentId = parentObjH.id;

  bool inDifferentScenes = parentObjH.sceneId != childObjH.sceneId;

  if (inDifferentScenes && sandbox.sceneIdToRootObj.find(child) != sandbox.sceneIdToRootObj.end()){
    auto ids = getChildrenIdsAndParent(sandbox.mainScene, child);
    for (auto id : ids){
      GameObjectH& objh = getGameObjectH(sandbox, id);
      objh.sceneId = parentObjH.sceneId;
    }
  }
}

objid rootIdForScene(SceneSandbox& sandbox, objid sceneId){
  return sandbox.sceneIdToRootObj.at(sceneId);
}