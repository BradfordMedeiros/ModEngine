#include "./scene_sandbox.h"

void addObjectToCache(Scene& mainScene, objid id);
void removeObjectFromCache(Scene& mainScene, objid id);

objid sandboxAddToScene(Scene& scene, objid sceneId, objid parentId, std::string name, GameObject& gameobjectObj){
  modassert(name == gameobjectObj.name, "name does not match gameobjectObj name");
  auto gameobjectH = GameObjectH { 
    .id = gameobjectObj.id,
    .parentId = parentId,
    .groupId = gameobjectObj.id,
    .sceneId = sceneId,
  };
  modassert(scene.idToGameObjectsH.find(gameobjectObj.id) == scene.idToGameObjectsH.end(), "id already exists");
  scene.idToGameObjectsH[gameobjectObj.id] = gameobjectH;
  scene.idToGameObjects[gameobjectObj.id] = gameobjectObj;
  if (!(scene.sceneToNameToId.at(sceneId).find(name) == scene.sceneToNameToId.at(sceneId).end())){
    modassert(false, std::string("name already exists: " + name))
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


AttrChildrenPair rootGameObject(){
  return AttrChildrenPair{
    .attr = GameobjAttributes {
      .stringAttributes = {{"physics", "disabled"}}
    },
    .children = {},
  };
}

SceneDeserialization createSceneFromParsedContent(
  objid sceneId,
  std::vector<Token> tokens,  
  std::function<objid()> getNewObjectId,
  std::function<std::set<std::string>(std::string&)> getObjautoserializerFields
){
  Scene scene;

  auto dividedTokens = divideMainAndSubelementTokens(tokens);
  auto serialGameAttrs = deserializeSceneTokens(dividedTokens.mainTokens);
  auto subelementAttrs = deserializeSceneTokens(dividedTokens.subelementTokens);

  auto rootId = getNewObjectId();
  auto rootName = "~root:" + std::to_string(rootId);
  scene.rootId = rootId;

  assert(serialGameAttrs.find(rootName) == serialGameAttrs.end());
  assert(rootName.find(',') == std::string::npos);

  serialGameAttrs[rootName] = rootGameObject();

  std::map<std::string, GameObject> gameobjs;

  for (auto [name, attrWithChildren] : serialGameAttrs){
    auto objName = name;
    if (name != rootName){
      objid id = (attrWithChildren.attr.stringAttributes.find("id") != attrWithChildren.attr.stringAttributes.end()) ? 
        std::atoi(attrWithChildren.attr.stringAttributes.at("id").c_str()) : 
        getNewObjectId();

      gameobjs[name] = gameObjectFromFields(name, id, attrWithChildren.attr, getObjautoserializerFields(objName));
    }else{
      gameobjs[name] = gameObjectFromFields(name, scene.rootId, attrWithChildren.attr, getObjautoserializerFields(objName)); 
    }
  }

  scene.sceneToNameToId[sceneId] = {};
  for (auto [name, gameobjectObj] : gameobjs){
    auto addedId = sandboxAddToScene(scene, sceneId, -1, name, gameobjectObj);
  }

  for (auto [name, attrWithChildren] : serialGameAttrs){
    for (auto childName : attrWithChildren.children){
      auto parentId = scene.sceneToNameToId.at(sceneId).at(name);
      enforceParentRelationship(scene, scene.sceneToNameToId.at(sceneId).at(childName), parentId);
    }
  }
  enforceRootObjects(scene);

  std::map<std::string, GameobjAttributes> additionalFields;
  for (auto &[name, attrWithChildren] : serialGameAttrs){
    additionalFields[name] = attrWithChildren.attr;
  }

  std::map<std::string, GameobjAttributes> subelementAttributes;
  for (auto &[name, attrWithChildren] : subelementAttrs){
    subelementAttributes[name] = attrWithChildren.attr;
  }

  SceneDeserialization deserializedScene {
    .scene = scene,
    .additionalFields = additionalFields,
    .subelementAttributes = subelementAttributes,
  };
  return deserializedScene;
}

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
){
  Scene& scene = sandbox.mainScene; 
  std::vector<LayerInfo>& layers = sandbox.layers;

  std::map<std::string,  GameobjAttributesWithId> nameToAdditionalFields;
  std::map<objid, objid> nodeIdToRealId;
  auto rootObj = scene.idToGameObjects.at(rootId);
  std::vector<objid> addedIds;

  for (auto [nodeId, transform] : gameobjTransforms){
    if (nodeId == rootIdNode){
      continue;
    }
    objid id = getNewObjectId();
    nodeIdToRealId[nodeId] = id;

    nameToAdditionalFields[names.at(nodeId)] = GameobjAttributesWithId{
      .id = id,
      .attr = additionalFields.at(nodeId),
    };

    auto name = names.at(nodeId);
    auto gameobj = gameObjectFromFields(name, id, defaultAttributesForMultiObj(transform, rootObj, additionalFields.at(nodeId)), getObjautoserializerFields(name));
    gameobj.transformation.rotation = transform.rotation; // todo make this work w/ attributes better

    auto addedId = sandboxAddToScene(scene, sceneId, -1, names.at(nodeId), gameobj);
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
    addObjectToCache(scene, id);
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

void addGameObjectToScene(SceneSandbox& sandbox, objid sceneId, std::string name, GameObject& gameobjectObj, std::vector<std::string> children){
  auto addedId = sandboxAddToScene(sandbox.mainScene, sceneId, -1, name, gameobjectObj);      
  for (auto child : children){
    if (sandbox.mainScene.sceneToNameToId.at(sceneId).find(child) == sandbox.mainScene.sceneToNameToId.at(sceneId).end()){
       // @TODO - shouldn't be an error should automatically create instead
      std::cout << "ERROR: NOT YET IMPLEMENTED : ADDING OBJECT WITH CHILD THAT DOES NOT EXIST IN THE SCENE" << std::endl;
      assert(false);
    }
    enforceParentRelationship(sandbox.mainScene, sandbox.mainScene.sceneToNameToId.at(sceneId).at(child), sandbox.mainScene.sceneToNameToId.at(sceneId).at(name));  
  }
  enforceRootObjects(sandbox.mainScene);
  addObjectToCache(sandbox.mainScene, addedId);
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
  sandbox.sceneIdToSceneMetadata.erase(sceneId);
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

std::vector<objid> listObjInScene(SceneSandbox& sandbox, std::optional<objid> sceneId){
  std::vector<objid> allObjects;
  for (auto const&[id, obj] : sandbox.mainScene.idToGameObjectsH){
    if (!sceneId.has_value() || obj.sceneId == sceneId.value()){
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

void traverseSceneObjects(Scene& scene, std::function<void(objid, glm::mat4, glm::mat4)> onObject){
  objid id = scene.rootId;
  traverseScene(id, scene.idToGameObjectsH.at(id), scene, glm::mat4(1.f),  [&scene, &onObject](objid foundId, glm::mat4 modelMatrix, glm::mat4 parentMatrix, std::string) -> void {
    onObject(foundId, modelMatrix, parentMatrix);
  });

}
void traverseSceneByLayer(Scene& scene, std::vector<LayerInfo> layers, std::function<void(objid, glm::mat4, glm::mat4, LayerInfo&, std::string)> onObject){
  std::vector<traversalData> datum;

  traverseSceneObjects(scene, [&datum](objid foundId, glm::mat4 modelMatrix, glm::mat4 parentMatrix) -> void {
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
        onObject(data.id, data.modelMatrix, data.parentMatrix, layer, gameobject.shader);
      }
    }  
  }
}

void traverseSandboxByLayer(SceneSandbox& sandbox, std::function<void(objid, glm::mat4, glm::mat4, LayerInfo&, std::string)> onObject){
  traverseSceneByLayer(sandbox.mainScene, sandbox.layers, onObject);
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
  auto gameobj = maybeGetGameObjectByName(sandbox, name, sceneId, false);
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

    auto gameobj = getGameObject(sandbox, id);
    auto gameobjecth = getGameObjectH(sandbox, id);
    if (rootIdForScene(sandbox, gameobjecth.sceneId) == gameobj.id){
      continue;
    }

    auto additionalFields = getAdditionalFields(id); 
    auto children = childnames(sandbox, gameobjecth);
    sceneData = sceneData + serializeObj(id, gameobjecth.groupId, gameobj, children, includeIds, additionalFields,  "");
  }
  return sceneData;
}


SceneSandbox createSceneSandbox(std::vector<LayerInfo> layers, std::function<std::set<std::string>(std::string&)> getObjautoserializerFields){
  Scene mainScene {
    .rootId = 0,
    .sceneToNameToId = {{ 0, {}}}
  };
  std::sort(std::begin(layers), std::end(layers), [](LayerInfo layer1, LayerInfo layer2) { return layer1.zIndex < layer2.zIndex; });

  std::string name = "root";
  auto rootObj = gameObjectFromFields(name, mainScene.rootId, rootGameObject().attr, getObjautoserializerFields(name)); 
  auto rootObjId = sandboxAddToScene(mainScene, 0, -1, rootObj.name, rootObj);
  addObjectToCache(mainScene, rootObjId);

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


bool sceneContainsTag(SceneSandbox& sandbox, objid sceneId, std::vector<std::string>& tags){
  for (auto &tag : sandbox.sceneIdToSceneMetadata.at(sceneId).tags){
    for (auto filterTag : tags){
      if (tag == filterTag){
        return true;
      }
    }
  } 
  return false;
}

std::vector<objid> allSceneIds(SceneSandbox& sandbox, std::optional<std::vector<std::string>> tags){
  std::vector<objid> sceneIds;
  for (auto [sceneId, _] : sandbox.sceneIdToRootObj){
    if (!tags.has_value() || sceneContainsTag(sandbox, sceneId, tags.value())){
      sceneIds.push_back(sceneId);
    }
  }
  return sceneIds;
} 

// something like .2041308683/testobject
bool extractSceneIdFromName(std::string& name, objid* _id, std::string* _searchName){
  if (name.at(0) == '.'){
    auto parts = split(name, '/');
    if (parts.size() > 1){
      assert(parts.size() >= 2);
      auto prefixSize = parts.at(0).size();
      auto prefix = name.substr(1, prefixSize - 1);
      auto rest = name.substr(prefixSize + 1, name.size());
      auto sceneId = std::atoi(prefix.c_str());
      *_id = sceneId;
      *_searchName = rest;
      return true;
    }
  }
  *_id = 0;
  *_searchName = "";
  return false;
}

std::optional<GameObject*> maybeGetGameObjectByName(SceneSandbox& sandbox, std::string name, objid sceneId, bool enablePrefixMatch){
  auto sceneToSearchIn = sceneId;
  auto effectiveName = name;
  if (enablePrefixMatch){
    objid prefixSceneId = 0;
    std::string restString = "";
    auto validPrefix = extractSceneIdFromName(name, &prefixSceneId, &restString);
    if (validPrefix){
      sceneToSearchIn = prefixSceneId;
      effectiveName = restString;
    }
  }
  for (auto &[id, gameObj]: sandbox.mainScene.idToGameObjects){
    if (gameObj.name == effectiveName && (sandbox.mainScene.idToGameObjectsH.at(id).sceneId == sceneToSearchIn)){
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
  return maybeGetGameObjectByName(sandbox, name, sceneId, false).has_value();
}
GameObject& getGameObject(SceneSandbox& sandbox, objid id){
  return getGameObject(sandbox.mainScene, id);
}
GameObject& getGameObject(SceneSandbox& sandbox, std::string name, objid sceneId){
  auto gameobj = maybeGetGameObjectByName(sandbox, name, sceneId, false);
  GameObject& obj = *(gameobj.value()); 
  return obj;
}
GameObjectH& getGameObjectH(SceneSandbox& sandbox, objid id){
  return getGameObjectH(sandbox.mainScene, id);
}
GameObjectH& getGameObjectH(SceneSandbox& sandbox, std::string name, objid sceneId){
  auto gameobj = maybeGetGameObjectByName(sandbox, name, sceneId, false);
  GameObject& obj = *(gameobj.value()); 
  GameObjectH& objh = getGameObjectH(sandbox, obj.id);
  return objh;
}


// What position should the gameobject be based upon the two absolute transform of parent and child
Transformation calcRelativeTransform(SceneSandbox& sandbox, objid childId, objid parentId){
  auto absTransformCache = matrixFromComponents(sandbox.mainScene.absoluteTransforms.at(childId).transform); 
  auto parentTransform = matrixFromComponents(sandbox.mainScene.absoluteTransforms.at(parentId).transform);

  //std::cout << "parent transform: " << std::endl;
  //printTransformInformation(getTransformationFromMatrix(parentTransform));

  //auto relativeTransform = glm::inverse(parentTransform) * absTransformCache;
  auto relativeTransform = glm::inverse(parentTransform) * absTransformCache;

  //std::cout << "child transform: " << std::endl;
  //printTransformInformation(getTransformationFromMatrix(absTransformCache));

  //std::cout << "rel transform: " << std::endl;
  //printTransformInformation(getTransformationFromMatrix(relativeTransform));
  //std::cout << std::endl;

  return getTransformationFromMatrix(relativeTransform);
}

Transformation calcRelativeTransform(SceneSandbox& sandbox, objid childId){
  auto parentId = getGameObjectH(sandbox, childId).parentId;
  if (parentId == -1){
    return sandbox.mainScene.absoluteTransforms.at(childId).transform;
  }
  return calcRelativeTransform(sandbox, childId, parentId);
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

// Might be better conceptually thought of as "enforceParentConstraints"
// Returns elemenets are the ids that were not explicitly updated by calling code, and therefore may need to be updated by calling code 
// In practice this means they need to update the actual physics trasform since this code recalculated it from the scenegraph
std::set<objid> updateSandbox(SceneSandbox& sandbox){
  std::set<objid> dirtiedElements;
  std::set<objid> newUpdatedElements;
  for (auto &[id, transform] : sandbox.mainScene.absoluteTransforms){
    if (id == 0){
      continue;
    }
    if (transform.updateType != UPDATE_NONE){
      dirtiedElements.insert(id);
    }
  }
  std::set<objid> alreadyUpdated;
  for (auto element : dirtiedElements){
    //std::cout << "dirty element: " << element << std::endl;
    updateTraverse(sandbox.mainScene, element, [&sandbox, &alreadyUpdated, &dirtiedElements, &newUpdatedElements](objid id) -> bool {
      if (dirtiedElements.find(id) == dirtiedElements.end()){
        newUpdatedElements.insert(id);
      }
      if (alreadyUpdated.find(id) != alreadyUpdated.end()){
        //std::cout << "id: " << id << " was already updated" << std::endl;
        return false;
      }
      TransformCacheElement& element = sandbox.mainScene.absoluteTransforms.at(id);

      auto parentId = getGameObjectH(sandbox, id).parentId;
      if(element.updateType != UPDATE_ABSOLUTE){ // element needs to be updated based on its old relative constraint
        auto gameobj = getGameObject(sandbox, id);
        //std::cout << "updated: " << id << " - " << gameobj.name << std::endl;
        auto relativeTransform = gameobj.transformation;
        element.transform = calcAbsoluteTransform(sandbox, parentId, relativeTransform);
      }else if (element.updateType == UPDATE_ABSOLUTE){
        // if the element was updated
        //std::cout << "> updating: " << getGameObject(sandbox, id).name << " - " << print(getGameObject(sandbox, id).transformation.position) << std::endl;
        getGameObject(sandbox, id).transformation = calcRelativeTransform(sandbox, id, parentId);
      }
      //std::cout << "updated: " << id << " - " << print (getGameObject(sandbox, id).transformation.position) << std::endl;

      element.updateType = UPDATE_NONE;
      alreadyUpdated.insert(id);
      return true;
    });
  }

  return newUpdatedElements;
}


void addObjectToCache(Scene& mainScene, objid id){
  traverseSceneObjects(mainScene, [&mainScene](objid id, glm::mat4 model, glm::mat4 parent) -> void {
    mainScene.absoluteTransforms[id] = TransformCacheElement {
      .transform = getTransformationFromMatrix(model),
      .updateType = UPDATE_NONE,
    };
  });

}
void removeObjectFromCache(Scene& mainScene, objid id){
  mainScene.absoluteTransforms.erase(id);
}

void updateAbsoluteTransform(SceneSandbox& sandbox, objid id, Transformation transform){
  //std::cout << "updating: " << id << " (" << getGameObject(sandbox, id).name << ") - " << print(transform.position) << std::endl;

  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform =  transform,
    .updateType = UPDATE_ABSOLUTE,
  };
}

void updateRelativeTransform(SceneSandbox& sandbox, objid id, Transformation transform){
  auto parentId = getGameObjectH(sandbox, id).parentId;
  getGameObject(sandbox, id).transformation = transform;
  auto newTransform = calcAbsoluteTransform(sandbox, parentId, transform);
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
    .updateType = UPDATE_RECALC_RELATIVE,
  };
}

void updateAbsolutePosition(SceneSandbox& sandbox, objid id, glm::vec3 position){
  Transformation newTransform =  sandbox.mainScene.absoluteTransforms.at(id).transform;
  newTransform.position = position;
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
    .updateType = UPDATE_ABSOLUTE,
  };
}
void updateRelativePosition(SceneSandbox& sandbox, objid id, glm::vec3 position){
  auto parentId = getGameObjectH(sandbox, id).parentId;
  auto oldRelativeTransform = calcRelativeTransform(sandbox, id, parentId);
  oldRelativeTransform.position = position;
  getGameObject(sandbox, id).transformation = oldRelativeTransform;
  auto newTransform = calcAbsoluteTransform(sandbox, parentId, oldRelativeTransform);
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
    .updateType = UPDATE_RECALC_RELATIVE,
  };
}
void updateAbsoluteScale(SceneSandbox& sandbox, objid id, glm::vec3 scale){
  Transformation newTransform =  sandbox.mainScene.absoluteTransforms.at(id).transform;
  newTransform.scale = scale;
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
    .updateType = UPDATE_ABSOLUTE,
  };
}
void updateRelativeScale(SceneSandbox& sandbox, objid id, glm::vec3 scale){
  auto parentId = getGameObjectH(sandbox, id).parentId;
  auto oldRelativeTransform = calcRelativeTransform(sandbox, id, parentId);
  oldRelativeTransform.scale = scale;
  getGameObject(sandbox, id).transformation = oldRelativeTransform;
  auto newTransform = calcAbsoluteTransform(sandbox, parentId, oldRelativeTransform);
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
    .updateType = UPDATE_RECALC_RELATIVE,
  };
}
void updateAbsoluteRotation(SceneSandbox& sandbox, objid id, glm::quat rotation){
  Transformation newTransform = sandbox.mainScene.absoluteTransforms.at(id).transform;
  newTransform.rotation = rotation;
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
    .updateType = UPDATE_ABSOLUTE,
  };
}
void updateRelativeRotation(SceneSandbox& sandbox, objid id, glm::quat rotation){
  auto parentId = getGameObjectH(sandbox, id).parentId;
  auto oldRelativeTransform = calcRelativeTransform(sandbox, id, parentId);
  oldRelativeTransform.rotation = rotation;
  getGameObject(sandbox, id).transformation = oldRelativeTransform;
  auto newTransform = calcAbsoluteTransform(sandbox, parentId, oldRelativeTransform);
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
    .updateType = UPDATE_RECALC_RELATIVE,
  };
}

glm::mat4 fullModelTransform(SceneSandbox& sandbox, objid id){
  TransformCacheElement& element = sandbox.mainScene.absoluteTransforms.at(id);
  //assert(element.updated == false && element.absTransformUpdated == false);
  return matrixFromComponents(element.transform);
}
Transformation fullTransformation(SceneSandbox& sandbox, objid id){
  return getTransformationFromMatrix(fullModelTransform(sandbox, id));
}
glm::mat4 armatureTransform(SceneSandbox& sandbox, objid id, std::string skeletonRoot, objid sceneId){
  auto gameobj = maybeGetGameObjectByName(sandbox, skeletonRoot, sceneId, false);
  assert(gameobj.has_value());
 
  auto groupTransform = fullModelTransform(sandbox, gameobj.value() -> id);
  auto modelTransform = fullModelTransform(sandbox, id);
  // group * something = model (aka aX = b, so X = inv(A) * B)
  // inverse(group) * model
  //auto groupToModel =  modelTransform * glm::inverse(groupTransform); 
  auto groupToModel =  glm::inverse(groupTransform) * modelTransform; 

  auto resultCheck = groupTransform * groupToModel;
  if (false && resultCheck != modelTransform){
    std::cout << "group_to_model = " << print(groupToModel) << std::endl;
    std::cout << "result_check = " << print(resultCheck) << std::endl;
    std::cout << "model_transform = " << print(modelTransform) << std::endl;
    assert(false);
  }
  return groupToModel;
}


SceneDeserialization deserializeScene(objid sceneId, std::string content, std::function<objid()> getNewObjectId, std::vector<Style>& styles, std::function<std::set<std::string>(std::string&)> getObjautoserializerFields){
  auto tokens = parseFormat(content);
  applyStyles(tokens, styles);
  return createSceneFromParsedContent(sceneId, tokens, getNewObjectId, getObjautoserializerFields);
}
AddSceneDataValues addSceneDataToScenebox(SceneSandbox& sandbox, std::string sceneFileName, objid sceneId, std::string sceneData, std::vector<Style>& styles, std::optional<std::string> name, std::optional<std::vector<std::string>> tags, std::function<std::set<std::string>(std::string&)> getObjautoserializerFields){
  assert(sandbox.sceneIdToRootObj.find(sceneId) == sandbox.sceneIdToRootObj.end());
  for (auto &[_, metadata] : sandbox.sceneIdToSceneMetadata){ // all scene names should be unique
    if (metadata.name == name && name != std::nullopt){
      std::cout << "scene name already exists: " << name.value() << std::endl;
      assert(false);
    }
  }

  SceneDeserialization deserializedScene = deserializeScene(sceneId, sceneData, getUniqueObjId, styles, getObjautoserializerFields);

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

  sandbox.sceneIdToSceneMetadata[sceneId] = SceneMetadata{
    .scenefile = sceneFileName,
    .name = name,
    .tags = tags.has_value() ? tags.value() : std::vector<std::string>({}),
  }; 

  enforceParentRelationship(sandbox.mainScene, rootId, sandbox.mainScene.rootId);

  std::vector<objid> idsAdded;
  for (auto &[id, obj] :  sandbox.mainScene.idToGameObjectsH){
    if (obj.sceneId == sceneId){
      idsAdded.push_back(id);
    }
  }

  for (auto &[id, obj] : deserializedScene.scene.idToGameObjects){
    addObjectToCache(sandbox.mainScene, id);
  }


  std::map<std::string, GameobjAttributesWithId>  additionalFields;
  for (auto &[name, attr] : deserializedScene.additionalFields){
    additionalFields[name] = GameobjAttributesWithId{
      .id = sandbox.mainScene.sceneToNameToId.at(sceneId).at(name),
      .attr = attr,
    };
  }

  AddSceneDataValues data  {
    .additionalFields = additionalFields,
    .idsAdded = idsAdded,
    .subelementAttributes = deserializedScene.subelementAttributes,
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

  sandbox.mainScene.absoluteTransforms.at(parent).updateType = UPDATE_ABSOLUTE;
}

std::optional<objid> listParentObjId(SceneSandbox& sandbox, objid sceneId){
  auto rootObj = rootIdForScene(sandbox, sceneId);
  GameObjectH& gameobjecth = getGameObjectH(sandbox, rootObj);
  if (gameobjecth.parentId == -1){
    return std::nullopt;
  }
  return gameobjecth.parentId;
}

objid rootIdForScene(SceneSandbox& sandbox, objid sceneId){
  return sandbox.sceneIdToRootObj.at(sceneId);
}

objid sceneId(SceneSandbox& sandbox, objid id){
  return sandbox.mainScene.idToGameObjectsH.at(id).sceneId;
}

bool parentSceneId(SceneSandbox& sandbox, objid sceneId, objid* _parentSceneId){
  auto currObjId = rootIdForScene(sandbox, sceneId);
  while (true){
    auto gameobjH = getGameObjectH(sandbox, currObjId); 
    if (gameobjH.parentId == -1){
      break;
    }
    currObjId = gameobjH.parentId;
    if (gameobjH.sceneId != sceneId){
      *_parentSceneId = gameobjH.sceneId;
      return true;
    }
  }
  *_parentSceneId = 0;
  return false;
}

std::vector<objid> childSceneIds(SceneSandbox& sandbox, objid sceneId){
  auto rootObjId = rootIdForScene(sandbox, sceneId);
  auto allIds = getChildrenIdsAndParent(sandbox.mainScene, rootObjId);

  std::set<objid> uniqueSceneIds;
  for (auto id : allIds){
    auto sceneId = getGameObjectH(sandbox, id).sceneId;
    uniqueSceneIds.insert(sceneId);
  }

  std::vector<objid> allSceneIds;
  for (auto id : uniqueSceneIds){
    if (id != sceneId){
      allSceneIds.push_back(id);
    }
  }

  return allSceneIds;
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

std::optional<objid> sceneIdByName(SceneSandbox& sandbox, std::string name){
  for (auto &[sceneId, metadata] : sandbox.sceneIdToSceneMetadata){
    if (metadata.name == name){
      return sceneId;
    }
  }
  return std::nullopt;
}