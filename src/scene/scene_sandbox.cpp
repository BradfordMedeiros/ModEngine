#include "./scene_sandbox.h"

glm::mat4 matrixFromComponents(glm::mat4 initialModel, glm::vec3 position, glm::vec3 scale, glm::quat rotation){
  glm::mat4 modelMatrix = glm::translate(initialModel, position);
  modelMatrix = modelMatrix * glm::toMat4(rotation);
  glm::mat4 scaledModelMatrix = modelMatrix * glm::scale(glm::mat4(1.f), scale);
  return scaledModelMatrix;
}
glm::mat4 matrixFromComponents(Transformation transformation){
  return matrixFromComponents(glm::mat4(1.f), transformation.position, transformation.scale, transformation.rotation);
}

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
  if (!(scene.sceneToNameToId.at(sceneId).find(name) == scene.sceneToNameToId.at(sceneId).end())){
    std::cout << "name already exists: " << name << std::endl;
    assert(false);
  }
  scene.sceneToNameToId.at(sceneId)[name]= gameobjectObj.id;
  return gameobjectObj.id;
}

// @TODO - bug around having multiple children in common. 
void enforceParentRelationship(Scene& scene, objid id, objid parentId){
  scene.idToGameObjectsH.at(id).parentId = parentId;
  scene.idToGameObjectsH.at(parentId).children.insert(id);
}

void enforceRootObjects(Scene& scene){
  scene.idToGameObjectsH.at(scene.rootId).children.clear();
  for (auto &[id, objh] : scene.idToGameObjectsH){
    if ((objh.parentId == -1 || objh.parentId == scene.rootId) && id != scene.rootId){
      enforceParentRelationship(scene, id, scene.rootId);
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

  scene.sceneToNameToId[sceneId] = {};
  for (auto [name, gameobjectObj] : gameobjs){
    auto addedId = addObjectToScene(scene, sceneId, -1, name, gameobjectObj);
  }

  for (auto [name, gameobj] : serialGameAttrs){
    for (auto childName : gameobj.children){
      std::cout << "child parenting not yet implemented" << std::endl;
      auto parentId = scene.sceneToNameToId.at(sceneId).at(name);
      enforceParentRelationship(scene, scene.sceneToNameToId.at(sceneId).at(childName), parentId);
    }
  }
  enforceRootObjects(scene);

  std::map<std::string, GameobjAttributes> additionalFields;
  for (auto &[name, attr] : serialGameAttrs){
    additionalFields[name] = attr;
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

std::map<std::string, GameobjAttributes> addSubsceneToRoot(
  Scene& scene, 
  std::vector<LayerInfo>& layers,
  objid sceneId,
  objid rootId, 
  objid rootIdNode, 
  std::map<objid, objid> childToParent, 
  std::map<objid, Transformation> gameobjTransforms, 
  std::map<objid, std::string> names,
  std::map<objid, GameobjAttributes> additionalFields,
  std::function<objid()> getNewObjectId
){
  std::map<std::string,  GameobjAttributes> nameToAdditionalFields;
  std::map<objid, objid> nodeIdToRealId;
  auto rootObj = scene.idToGameObjects.at(rootId);
  std::vector<objid> addedIds;

  for (auto [nodeId, transform] : gameobjTransforms){
    if (nodeId == rootIdNode){
      continue;
    }
    objid id = getNewObjectId();
    nodeIdToRealId[nodeId] = id;

    nameToAdditionalFields[names.at(nodeId)] = additionalFields.at(nodeId);

    auto gameobj = gameObjectFromFields(names.at(nodeId), id, defaultAttributesForMultiObj(transform, rootObj));
    gameobj.transformation.rotation = transform.rotation; // todo make this work w/ attributes better

    auto addedId = addObjectToScene(scene, sceneId, -1, names.at(nodeId), gameobj);
    addedIds.push_back(addedId);
    scene.idToGameObjectsH.at(id).groupId = rootId;
  }

  for (auto [childId, parentId] : childToParent){
    auto realChildId = nodeIdToRealId.at(childId);
    auto realParentId = parentId == rootIdNode ? rootId : nodeIdToRealId.at(parentId);
    enforceParentRelationship(scene, realChildId, realParentId);
  }
  enforceRootObjects(scene);
  for (auto id : addedIds){
    addObjectToCache(scene, layers, id);
  }
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
    if (sandbox.mainScene.sceneToNameToId.at(sceneId).find(child) == sandbox.mainScene.sceneToNameToId.at(sceneId).end()){
       // @TODO - shouldn't be an error should automatically create instead
      std::cout << "ERROR: NOT YET IMPLEMENTED : ADDING OBJECT WITH CHILD THAT DOES NOT EXIST IN THE SCENE" << std::endl;
      assert(false);
    }
    enforceParentRelationship(sandbox.mainScene, sandbox.mainScene.sceneToNameToId.at(sceneId).at(child), sandbox.mainScene.sceneToNameToId.at(sceneId).at(name));  
  }
  enforceRootObjects(sandbox.mainScene);
  addObjectToCache(sandbox.mainScene, sandbox.layers, addedId);
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

void pruneSceneId(SceneSandbox& sandbox, objid sceneId){
  sandbox.sceneIdToRootObj.erase(sceneId);
  sandbox.sceneIdToSceneName.erase(sceneId);
  sandbox.mainScene.sceneToNameToId.erase(sceneId); 
}
void maybePruneScenes(SceneSandbox& sandbox){
  std::vector<objid> sceneIdsToPrune;
  for (auto &[sceneId, rootObjId] : sandbox.sceneIdToRootObj){
    if (sandbox.mainScene.idToGameObjectsH.find(rootObjId) == sandbox.mainScene.idToGameObjectsH.end()){
      sceneIdsToPrune.push_back(sceneId);
    }
  }
  for (auto sceneId : sceneIdsToPrune){
    pruneSceneId(sandbox, sceneId);
  }
}

void removeObjectsFromScenegraph(SceneSandbox& sandbox, std::vector<objid> objects){  
  for (auto id : objects){
    Scene& scene = sandbox.mainScene;
    std::string objectName = scene.idToGameObjects.at(id).name;
    auto sceneId = scene.idToGameObjectsH.at(id).sceneId;
    scene.idToGameObjects.erase(id);
    scene.idToGameObjectsH.erase(id);
    removeObjectFromCache(scene, id);

    std::cout << "scene id: " << sceneId << std::endl;
    std::cout << "object name: " << objectName << std::endl;
    scene.sceneToNameToId.at(sceneId).erase(objectName);
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

void traverseScene(objid id, GameObjectH objectH, Scene& scene, glm::mat4 model, std::function<void(objid, glm::mat4, glm::mat4, std::string)> onObject){
  GameObject object = scene.idToGameObjects.at(objectH.id);
  
  glm::mat4 modelMatrix = matrixFromComponents(
    model,
    object.transformation.position, 
    object.transformation.scale, 
    object.transformation.rotation
  );

  onObject(id, modelMatrix, model, "");

  for (objid id: objectH.children){
    traverseScene(id, scene.idToGameObjectsH.at(id), scene, modelMatrix,  onObject);
  }
}

struct traversalData {
  objid id;
  glm::mat4 modelMatrix;
  glm::mat4 parentMatrix;
};
void traverseScene(Scene& scene, std::vector<LayerInfo> layers, std::function<void(objid, glm::mat4, glm::mat4, bool, int, std::string)> onObject){
  glm::mat4 initialModel(1.f);
  std::vector<traversalData> datum;

  objid id = scene.rootId;
  traverseScene(id, scene.idToGameObjectsH.at(id), scene, initialModel,  [&datum](objid foundId, glm::mat4 modelMatrix, glm::mat4 parentMatrix, std::string) -> void {
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
        onObject(data.id, data.modelMatrix, data.parentMatrix, layer.orthographic, layer.depthBufferLayer, gameobject.fragshader);
      }
    }  
  }
}

void traverseSandbox(SceneSandbox& sandbox, std::function<void(objid, glm::mat4, glm::mat4, bool, int, std::string)> onObject){
  traverseScene(sandbox.mainScene, sandbox.layers, onObject);
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
GameObjectH& getGameObjectH(Scene& scene, objid id){
  return scene.idToGameObjectsH.at(id);
}
objid getGroupId(Scene& scene, objid id){
  return scene.idToGameObjectsH.at(id).groupId; 
}
objid getIdForName(SceneSandbox& sandbox, std::string name, objid sceneId){
  auto gameobj = maybeGetGameObjectByName(sandbox, name, sceneId);
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
    .sceneToNameToId = {{ 0, {}}}
  };
  std::sort(std::begin(layers), std::end(layers), [](LayerInfo layer1, LayerInfo layer2) { return layer1.zIndex < layer2.zIndex; });

  auto rootObj = gameObjectFromFields("root", mainScene.rootId, rootGameObject()); 
  auto rootObjId = addObjectToScene(mainScene, 0, -1, rootObj.name, rootObj);
  addObjectToCache(mainScene, layers, rootObjId);

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

std::optional<GameObject*> maybeGetGameObjectByName(SceneSandbox& sandbox, std::string name, objid sceneId){
  for (auto &[id, gameObj]: sandbox.mainScene.idToGameObjects){
    if (gameObj.name == name && (sandbox.mainScene.idToGameObjectsH.at(id).sceneId == sceneId)){
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
bool idExists(SceneSandbox& sandbox, std::string name, objid sceneId){
  return maybeGetGameObjectByName(sandbox, name, sceneId).has_value();
}
GameObject& getGameObject(SceneSandbox& sandbox, objid id){
  return getGameObject(sandbox.mainScene, id);
}
GameObject& getGameObject(SceneSandbox& sandbox, std::string name, objid sceneId){
  auto gameobj = maybeGetGameObjectByName(sandbox, name, sceneId);
  GameObject& obj = *(gameobj.value()); 
  return obj;
}
GameObjectH& getGameObjectH(SceneSandbox& sandbox, objid id){
  return getGameObjectH(sandbox.mainScene, id);
}
GameObjectH& getGameObjectH(SceneSandbox& sandbox, std::string name, objid sceneId){
  auto gameobj = maybeGetGameObjectByName(sandbox, name, sceneId);
  GameObject& obj = *(gameobj.value()); 
  GameObjectH& objh = getGameObjectH(sandbox, obj.id);
  return objh;
}

// What position should the gameobject be based upon the two absolute transform of parent and child
Transformation calcRelativeTransform(SceneSandbox& sandbox, objid childId, objid parentId){
  auto absTransformCache = matrixFromComponents(sandbox.mainScene.absoluteTransforms.at(childId).transform); 
  auto parentTransform = matrixFromComponents(sandbox.mainScene.absoluteTransforms.at(parentId).transform);
  auto relativeTransform = glm::inverse(parentTransform) * absTransformCache;
  return getTransformationFromMatrix(relativeTransform);
}

// What should the absolute transform be given a parents absolute and a relative child transform
Transformation calcAbsoluteTransform(SceneSandbox& sandbox, objid parentId, Transformation transform){
  Transformation parentTransform = sandbox.mainScene.absoluteTransforms.at(parentId).transform;

  glm::mat4 parentMatrix = matrixFromComponents(parentTransform);
  glm::mat4 childMatrix = matrixFromComponents(transform);

  auto finalTransform = parentMatrix * childMatrix;
  return getTransformationFromMatrix(finalTransform);
}

void updateTraverse(Scene& scene, objid id, std::function<bool(objid)> onAddObject){
  auto parentObjH = scene.idToGameObjectsH.at(id);
  bool shouldTraverse = onAddObject(parentObjH.id);
  if (!shouldTraverse){
    return;
  }
  for (objid id : parentObjH.children){
    updateTraverse(scene, id, onAddObject);
  }
}

void updateSandbox(SceneSandbox& sandbox){
  std::set<objid> dirtiedElements;
  for (auto &[id, transform] : sandbox.mainScene.absoluteTransforms){
    if (id == 0){
      continue;
    }
    if (transform.absTransformUpdated){
      dirtiedElements.insert(id);
    }
  }
  /*std::cout << "need to update: ";
  for (auto element : dirtiedElements){
    std::cout << element << " ";
  }
  std::cout << std::endl;
  */
  std::set<objid> alreadyUpdated;
  for (auto element : dirtiedElements){
    updateTraverse(sandbox.mainScene, element, [&sandbox, &alreadyUpdated](objid id) -> bool {
      if (alreadyUpdated.find(id) != alreadyUpdated.end()){
        return false;
      }
      TransformCacheElement& element = sandbox.mainScene.absoluteTransforms.at(id);

      auto parentId = getGameObjectH(sandbox, id).parentId;
      if (!element.absTransformUpdated){
        if (parentId != -1){
          element.transform = calcAbsoluteTransform(sandbox, parentId, getGameObject(sandbox, id).transformation);
        }
      }
      getGameObject(sandbox, id).transformation = calcRelativeTransform(sandbox, id, parentId);
      element.absTransformUpdated = false;
      //std::cout << "traversed element: " << id << std::endl;
      alreadyUpdated.insert(id);
      return true;
    });
    //std::cout << std::endl;
  }
}

void addObjectToCache(Scene& mainScene, std::vector<LayerInfo>& layers, objid id){
  traverseScene(mainScene, layers, [&mainScene](objid id, glm::mat4 model, glm::mat4 parent, bool isOrtho, bool ignoreDepth, std::string fragshader) -> void {
    mainScene.absoluteTransforms[id] = TransformCacheElement {
      .transform = getTransformationFromMatrix(model),
      .absTransformUpdated = false,
    };
  });

}
void removeObjectFromCache(Scene& mainScene, objid id){
  mainScene.absoluteTransforms.erase(id);
}

// All transform updates update the absolute position cache first
// then the relative transform gets updated, if either the id mat4, or parent mat4 were dirtied.
// should have a flag which says to preserve the constaint or not 
void updateAbsoluteTransform(SceneSandbox& sandbox, objid id, Transformation transform){
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform =  transform,
    .absTransformUpdated = true,
  };
}

void updateAbsolutePosition(SceneSandbox& sandbox, objid id, glm::vec3 position){
  Transformation newTransform =  sandbox.mainScene.absoluteTransforms.at(id).transform;
  newTransform.position = position;
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
    .absTransformUpdated = true,
  };
  std::cout << "update absolute position" << std::endl;
}
void updateRelativePosition(SceneSandbox& sandbox, objid id, glm::vec3 position){
  auto parentId = getGameObjectH(sandbox, id).parentId;
  Transformation oldRelativeTransform = getGameObject(sandbox, id).transformation;
  oldRelativeTransform.position = position;
  auto newTransform = calcAbsoluteTransform(sandbox, parentId, oldRelativeTransform);
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
    .absTransformUpdated = true,
  };
  std::cout << "update relative position" << std::endl;

}
void updateAbsoluteScale(SceneSandbox& sandbox, objid id, glm::vec3 scale){
  Transformation newTransform =  sandbox.mainScene.absoluteTransforms.at(id).transform;
  newTransform.scale = scale;
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
    .absTransformUpdated = true,
  };
}
void updateRelativeScale(SceneSandbox& sandbox, objid id, glm::vec3 scale){
  auto parentId = getGameObjectH(sandbox, id).parentId;
  Transformation oldRelativeTransform = getGameObject(sandbox, id).transformation;
  oldRelativeTransform.scale = scale;
  auto newTransform = calcAbsoluteTransform(sandbox, parentId, oldRelativeTransform);
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
    .absTransformUpdated = true,
  };
}
void updateAbsoluteRotation(SceneSandbox& sandbox, objid id, glm::quat rotation){
  Transformation newTransform = sandbox.mainScene.absoluteTransforms.at(id).transform;
  newTransform.rotation = rotation;
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
    .absTransformUpdated = true,
  };
}
void updateRelativeRotation(SceneSandbox& sandbox, objid id, glm::quat rotation){
  auto parentId = getGameObjectH(sandbox, id).parentId;
  Transformation oldRelativeTransform = getGameObject(sandbox, id).transformation;
  oldRelativeTransform.rotation = rotation;
  auto newTransform = calcAbsoluteTransform(sandbox, parentId, oldRelativeTransform);
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
    .absTransformUpdated = true,
  };
}

glm::mat4 fullModelTransform(SceneSandbox& sandbox, objid id){
  return matrixFromComponents(sandbox.mainScene.absoluteTransforms.at(id).transform);
}
glm::mat4 armatureTransform(SceneSandbox& sandbox, objid id, std::string skeletonRoot, objid sceneId){
  auto gameobj = maybeGetGameObjectByName(sandbox, skeletonRoot, sceneId);
  assert(gameobj.has_value());
 
  auto groupTransform = fullModelTransform(sandbox, gameobj.value() -> id);
  auto modelTransform = fullModelTransform(sandbox, id);
  // group * something = model (aka aX = b, so X = inv(A) * B)
  // inverse(group) * model
  auto groupToModel =  glm::inverse(groupTransform) * modelTransform;

  auto resultCheck = groupTransform * groupToModel;
  if (resultCheck != modelTransform){
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
AddSceneDataValues addSceneDataToScenebox(SceneSandbox& sandbox, std::string sceneFileName, objid sceneId, std::string sceneData){
  assert(sandbox.sceneIdToRootObj.find(sceneId) == sandbox.sceneIdToRootObj.end());

  SceneDeserialization deserializedScene = deserializeScene(sceneId, sceneData, getUniqueObjId);

  for (auto &[id, obj] : deserializedScene.scene.idToGameObjects){
    sandbox.mainScene.idToGameObjects[id] = obj;
  }
  for (auto &[id, obj] : deserializedScene.scene.idToGameObjectsH){
    sandbox.mainScene.idToGameObjectsH[id] = obj;
  }
  
  assert(sandbox.mainScene.sceneToNameToId.find(sceneId) ==  sandbox.mainScene.sceneToNameToId.end());
  sandbox.mainScene.sceneToNameToId[sceneId] = {};
  for (auto &[name, id] : deserializedScene.scene.sceneToNameToId.at(sceneId)){
    sandbox.mainScene.sceneToNameToId.at(sceneId)[name] = id;
  }
  auto rootId = deserializedScene.scene.rootId;
  sandbox.sceneIdToRootObj[deserializedScene.scene.idToGameObjectsH.at(rootId).sceneId] = rootId;
  sandbox.sceneIdToSceneName[sceneId] = sceneFileName;

  enforceParentRelationship(sandbox.mainScene, rootId, sandbox.mainScene.rootId);

  std::vector<objid> idsAdded;
  for (auto &[id, obj] :  sandbox.mainScene.idToGameObjectsH){
    if (obj.sceneId == sceneId){
      idsAdded.push_back(id);
    }
  }

  for (auto &[id, obj] : deserializedScene.scene.idToGameObjects){
    addObjectToCache(sandbox.mainScene, sandbox.layers, id);
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
  pruneSceneId(sandbox, sceneId);
}
bool sceneExists(SceneSandbox& sandbox, objid sceneId){
  return !(sandbox.sceneIdToRootObj.find(sceneId) == sandbox.sceneIdToRootObj.end());
}    

std::map<std::string, GameobjAttributes> multiObjAdd(
  SceneSandbox& sandbox,
  objid sceneId,
  objid rootId,
  objid rootIdNode, 
  std::map<objid, objid> childToParent, 
  std::map<objid, Transformation> gameobjTransforms, 
  std::map<objid, std::string> names, 
  std::map<objid, GameobjAttributes> additionalFields,
  std::function<objid()> getNewObjectId){
  auto nameToAdditionalFields = addSubsceneToRoot(sandbox.mainScene, sandbox.layers, sceneId, rootId, rootIdNode, childToParent, gameobjTransforms, names, additionalFields, getNewObjectId);
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

objid sceneId(SceneSandbox& sandbox, objid id){
  return sandbox.mainScene.idToGameObjectsH.at(id).sceneId;
}

std::vector<objid> getByName(SceneSandbox& sandbox, std::string name){
  std::vector<objid> ids;
  for (auto &[id, gameobj] : sandbox.mainScene.idToGameObjects){
    if (gameobj.name == name){
      ids.push_back(id);
    }
  }
  return ids;
}

int getNumberOfObjects(SceneSandbox& sandbox){
  return sandbox.mainScene.idToGameObjects.size();
}

int getNumberScenesLoaded(SceneSandbox& sandbox){
  return sandbox.mainScene.sceneToNameToId.size();
}
