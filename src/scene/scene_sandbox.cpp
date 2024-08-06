#include "./scene_sandbox.h"

void addObjectToCache(SceneSandbox& sandbox, objid id);
void removeObjectFromCache(SceneSandbox& sandbox, objid id);

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

  modassert(scene.sceneToNameToId.find(sceneId) != scene.sceneToNameToId.end(), std::string("scene does not exist: ") + std::to_string(sceneId));
  if (scene.sceneToNameToId.at(sceneId).find(name) != scene.sceneToNameToId.at(sceneId).end()){
    modassert(false, std::string("name already exists: " + name))
  }
  scene.sceneToNameToId.at(sceneId)[name]= gameobjectObj.id;
  return gameobjectObj.id;
}

// @TODO - bug around having multiple children in common. 
void enforceParentRelationship(Scene& scene, objid id, objid parentId){
  scene.idToGameObjectsH.at(id).parentId = parentId;
  scene.idToGameObjectsH.at(parentId).children.insert(id);
  //scene.constraints[id] = scene.idToGameObjects.at(id).transformation;
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
    .attr = GameobjAttributes {},
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
      auto idValue = objIdFromAttribute(attrWithChildren.attr);
      objid id = idValue.has_value() ? idValue.value() : getNewObjectId();
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
    addObjectToCache(sandbox, id);
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
  addObjectToCache(sandbox, addedId);
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
    if (scene.idToGameObjects.find(id) == scene.idToGameObjects.end()){
      continue;
    }
    std::string objectName = scene.idToGameObjects.at(id).name;
    auto sceneId = scene.idToGameObjectsH.at(id).sceneId;
    scene.idToGameObjects.erase(id);
    scene.idToGameObjectsH.erase(id);
    removeObjectFromCache(sandbox, id);

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


struct traversalData {
  objid id;
  glm::mat4 modelMatrix;
  glm::mat4 parentMatrix;
};
void traverseSandboxByLayer(SceneSandbox& sandbox, std::function<void(objid, glm::mat4, glm::mat4, LayerInfo&, std::string)> onObject){
  std::vector<traversalData> datum;
  for (auto &[id, transformCacheElement] : sandbox.mainScene.absoluteTransforms){
    datum.push_back(traversalData{
      .id = id,
      .modelMatrix = matrixFromComponents(transformCacheElement.transform),
      .parentMatrix = glm::mat4(1.f),
    });
  }
  for (auto layer : sandbox.layers){      // @TODO could organize this before to not require pass on each frame
    for (auto data : datum){
      auto gameobject = sandbox.mainScene.idToGameObjects.at(data.id);
      if (gameobject.layer == layer.name){
        onObject(data.id, data.modelMatrix, data.parentMatrix, layer, gameobject.shader);
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
  modassert(scene.idToGameObjects.find(id) != scene.idToGameObjects.end(), "gameobj does not exist");
  return scene.idToGameObjects.at(id);
}
GameObjectH& getGameObjectH(Scene& scene, objid id){
  modassert(scene.idToGameObjectsH.find(id) != scene.idToGameObjectsH.end(), "gameobjh does not exist");
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
    .idToGameObjects = {},
    .idToGameObjectsH = {},
    .sceneToNameToId = {{ 0, {}}},
    .absoluteTransforms = {},
  };
  std::sort(std::begin(layers), std::end(layers), [](LayerInfo layer1, LayerInfo layer2) { return layer1.zIndex < layer2.zIndex; });

  std::string name = "root";
  auto rootObj = gameObjectFromFields(name, mainScene.rootId, rootGameObject().attr, getObjautoserializerFields(name)); 
  auto rootObjId = sandboxAddToScene(mainScene, 0, -1, rootObj.name, rootObj);

  SceneSandbox sandbox {
    .mainScene = mainScene,
    .layers = layers,
    .updatedIds = {},
  };
  addObjectToCache(sandbox, rootObjId);

  return sandbox;
}

void forEveryGameobj(SceneSandbox& sandbox, std::function<void(objid id, GameObject& gameobj)> onElement){
  for (auto &[id, gameObj]: sandbox.mainScene.idToGameObjects){
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

std::vector<objid> getIdsInGroupByObjId(SceneSandbox& sandbox, objid index){
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

Transformation calcRelativeTransform(Transformation& child, Transformation& parent){
  auto absTransformCache = matrixFromComponents(child); 
  auto parentTransform = matrixFromComponents(parent);
  auto relativeTransform = glm::inverse(parentTransform) * absTransformCache;
  return getTransformationFromMatrix(relativeTransform);
}

// What position should the gameobject be based upon the two absolute transform of parent and child
Transformation calcRelativeTransform(SceneSandbox& sandbox, objid childId, objid parentId){
  return calcRelativeTransform(sandbox.mainScene.absoluteTransforms.at(childId).transform, sandbox.mainScene.absoluteTransforms.at(parentId).transform);
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
  modassert(parentId != -1, "scene sandbox - calc absolute Transform - parent id should not be -1");
  Transformation parentTransform = sandbox.mainScene.absoluteTransforms.at(parentId).transform;

  glm::mat4 parentMatrix = matrixFromComponents(parentTransform);
  glm::mat4 childMatrix = matrixFromComponents(transform);

  auto finalTransform = parentMatrix * childMatrix;
  return getTransformationFromMatrix(finalTransform);
}

std::vector<objid> bfsElementAndChildren(SceneSandbox& sandbox, objid updatedId){
  std::vector<objid> ids;
  std::queue<objid> idsToVisit;  // shouldn't actually be needed since no common children
  std::set<objid> visited;

  auto currentId = updatedId;
  idsToVisit.push(currentId);

  while (idsToVisit.size() > 0){
    auto idToVisit = idsToVisit.front();
    ids.push_back(idToVisit);
    visited.insert(idToVisit);
    idsToVisit.pop();
    auto objH = sandbox.mainScene.idToGameObjectsH.at(idToVisit);
    for (objid id : objH.children){
      if (visited.count(id) == 0){
        idsToVisit.push(id);
      }
    }    
  }
  //

  return ids;
}

void updateAllChildrenPositions(SceneSandbox& sandbox, objid updatedId, bool justAdded = false){
  //std::cout << "should update id: " << updatedId << std::endl;
  // do a breath first search, and then update it in that order
  auto updatedIdElements = bfsElementAndChildren(sandbox, updatedId);
  //std::cout << "should update: " << print(updatedIdElements) << std::endl;
  for (auto id : updatedIdElements){
    if (id != updatedId){
      sandbox.updatedIds.insert(id);  
    }
    auto parentId = getGameObjectH(sandbox, id).parentId;
    if (
      sandbox.mainScene.absoluteTransforms.find(id) == sandbox.mainScene.absoluteTransforms.end() ||
      sandbox.mainScene.absoluteTransforms.find(parentId) == sandbox.mainScene.absoluteTransforms.end()){
      continue;
    }
    if (parentId != -1){
      GameObject& gameobj = getGameObject(sandbox, id);
      if (gameobj.physicsOptions.isStatic || !gameobj.physicsOptions.enabled || justAdded /* this is so when you spawn it, the relative transform determines where it goes */){
         auto currentConstraint = gameobj.transformation;
         auto newTransform = calcAbsoluteTransform(sandbox, parentId, currentConstraint);
         sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
           .transform = newTransform,
         };       
      }
    }
  }

}

std::set<objid> updateSandbox(SceneSandbox& sandbox){
  std::set<objid> oldUpdated = sandbox.updatedIds;
  sandbox.updatedIds = {};
  return oldUpdated;
}


void addObjectToCache(SceneSandbox& sandbox, objid id){
  GameObject object = sandbox.mainScene.idToGameObjects.at(id);
  auto elementMatrix = matrixFromComponents(
    glm::mat4(1.f),
    object.transformation.position, 
    object.transformation.scale, 
    object.transformation.rotation
  );
  sandbox.mainScene.absoluteTransforms[id] = TransformCacheElement {
    .transform = getTransformationFromMatrix(elementMatrix),
  };
  updateAllChildrenPositions(sandbox, id, true);
}
void removeObjectFromCache(SceneSandbox& sandbox, objid id){
  sandbox.mainScene.absoluteTransforms.erase(id);
  sandbox.updatedIds.erase(id);
}

void updateAbsoluteTransform(SceneSandbox& sandbox, objid id, Transformation transform){
  auto parentId = getGameObjectH(sandbox, id).parentId;

  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform =  transform,
  };

  if (parentId != -1){
    auto oldRelativeTransform = calcRelativeTransform(sandbox, id);
    getGameObject(sandbox, id).transformation = oldRelativeTransform;
    //modassert(!aboutEqual(oldRelativeTransform.scale.x, 0.f), "0 scale for x component");
    //modassert(!aboutEqual(oldRelativeTransform.scale.y, 0.f), "0 scale for y component");
    //modassert(!aboutEqual(oldRelativeTransform.scale.z, 0.f), "0 scale for z component");
  }
  updateAllChildrenPositions(sandbox, id);
}

void updateRelativeTransform(SceneSandbox& sandbox, objid id, Transformation transform){
  auto parentId = getGameObjectH(sandbox, id).parentId;
  getGameObject(sandbox, id).transformation = transform;
  auto newTransform = parentId == -1 ? transform : calcAbsoluteTransform(sandbox, parentId, transform);
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
  };
  updateAllChildrenPositions(sandbox, id);
}

void updateAbsolutePosition(SceneSandbox& sandbox, objid id, glm::vec3 position){
  auto oldAbsoluteTransform = sandbox.mainScene.absoluteTransforms.at(id);
  Transformation newTransform = oldAbsoluteTransform.transform;
  newTransform.position = position;
  updateAbsoluteTransform(sandbox, id, newTransform);
}
void updateRelativePosition(SceneSandbox& sandbox, objid id, glm::vec3 position){
   // just update the constraint, mark absolute transform dirtyu
  auto oldRelativeTransform = calcRelativeTransform(sandbox, id);
  oldRelativeTransform.position = position;
  updateRelativeTransform(sandbox, id, oldRelativeTransform);

}
void updateAbsoluteScale(SceneSandbox& sandbox, objid id, glm::vec3 scale){
  auto oldAbsoluteTransform = sandbox.mainScene.absoluteTransforms.at(id);
  Transformation newTransform = oldAbsoluteTransform.transform;
  newTransform.scale = scale;
  updateAbsoluteTransform(sandbox, id, newTransform); 
}
void updateRelativeScale(SceneSandbox& sandbox, objid id, glm::vec3 scale){
  auto oldRelativeTransform = calcRelativeTransform(sandbox, id);
  oldRelativeTransform.scale = scale;
  updateRelativeTransform(sandbox, id, oldRelativeTransform);
}
void updateAbsoluteRotation(SceneSandbox& sandbox, objid id, glm::quat rotation){
  auto oldAbsoluteTransform = sandbox.mainScene.absoluteTransforms.at(id);
  Transformation newTransform = oldAbsoluteTransform.transform;
  newTransform.rotation = rotation;
  updateAbsoluteTransform(sandbox, id, newTransform); 
}
void updateRelativeRotation(SceneSandbox& sandbox, objid id, glm::quat rotation){
  auto oldRelativeTransform = calcRelativeTransform(sandbox, id);
  oldRelativeTransform.rotation = rotation;
  updateRelativeTransform(sandbox, id, oldRelativeTransform);
}

glm::mat4 fullModelTransform(SceneSandbox& sandbox, objid id){
  TransformCacheElement& element = sandbox.mainScene.absoluteTransforms.at(id);
  //assert(element.updated == false && element.absTransformUpdated == false);
  return matrixFromComponents(element.transform);
}
Transformation fullTransformation(SceneSandbox& sandbox, objid id){
  return getTransformationFromMatrix(fullModelTransform(sandbox, id));
}

SceneDeserialization deserializeScene(objid sceneId, std::string content, std::function<objid()> getNewObjectId, std::function<std::set<std::string>(std::string&)> getObjautoserializerFields){
  auto tokens = parseFormat(content);
  return createSceneFromParsedContent(sceneId, tokens, getNewObjectId, getObjautoserializerFields);
}
AddSceneDataValues addSceneDataToScenebox(SceneSandbox& sandbox, std::string sceneFileName, objid sceneId, std::string sceneData,  std::optional<std::string> name, std::optional<std::vector<std::string>> tags, std::function<std::set<std::string>(std::string&)> getObjautoserializerFields){
  assert(sandbox.sceneIdToRootObj.find(sceneId) == sandbox.sceneIdToRootObj.end());
  for (auto &[_, metadata] : sandbox.sceneIdToSceneMetadata){ // all scene names should be unique
    if (metadata.name == name && name != std::nullopt){
      modassert(false, std::string("scene name already exists: ") + name.value());
    }
  }

  SceneDeserialization deserializedScene = deserializeScene(sceneId, sceneData, getUniqueObjId, getObjautoserializerFields);

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
    addObjectToCache(sandbox, id);
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
  updateAllChildrenPositions(sandbox, parent, true); // TODO - only update the newly parented children
}

std::optional<objid> listParentObjId(SceneSandbox& sandbox, objid sceneId){
  auto rootObj = rootIdForScene(sandbox, sceneId);
  GameObjectH& gameobjecth = getGameObjectH(sandbox, rootObj);
  if (gameobjecth.parentId == -1){
    return std::nullopt;
  }
  return gameobjecth.parentId;
}

std::optional<objid> getParent(SceneSandbox& sandbox, objid id){
  GameObjectH& gameobjecth = getGameObjectH(sandbox, id);
  if (gameobjecth.parentId == -1){
    return std::nullopt;
  }
  return gameobjecth.parentId;
}

objid rootIdForScene(SceneSandbox& sandbox, objid sceneId){
  std::cout << "scene id: " << sceneId << std::endl;
  if (sceneId == 0){
    return sandbox.mainScene.rootId;
  }
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

objid rootSceneId(SceneSandbox& sandbox){
  return sandbox.mainScene.idToGameObjectsH.at(sandbox.mainScene.rootId).sceneId;
}