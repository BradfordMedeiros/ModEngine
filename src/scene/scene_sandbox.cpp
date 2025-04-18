#include "./scene_sandbox.h"

void addObjectToCache(SceneSandbox& sandbox, objid id);
void removeObjectFromCache(SceneSandbox& sandbox, objid id);

void enforceParentRelationship(Scene& scene, objid id, objid parentId){
  auto oldParent = scene.idToGameObjectsH.at(id).parentId;
  if (oldParent){
    scene.idToGameObjectsH.at(oldParent).children.erase(id);
  }
  scene.idToGameObjectsH.at(id).parentId = parentId;
  scene.idToGameObjectsH.at(parentId).children.insert(id);
}

objid sandboxAddToScene(Scene& scene, objid sceneId, std::optional<objid> parentId, GameObject& gameobjectObj, std::optional<objid> prefabId){
  auto gameobjectH = GameObjectH { 
    .id = gameobjectObj.id,
    .parentId = parentId.has_value() ? parentId.value() : 0,
    .groupId = gameobjectObj.id,
    .sceneId = sceneId,
    .prefabId = prefabId,
  };
  modassert(scene.idToGameObjectsH.find(gameobjectObj.id) == scene.idToGameObjectsH.end(), "id already exists");
  modassert(scene.sceneToNameToId.find(sceneId) != scene.sceneToNameToId.end(), std::string("scene does not exist: ") + std::to_string(sceneId));

  scene.idToGameObjectsH[gameobjectObj.id] = gameobjectH;
  scene.idToGameObjects[gameobjectObj.id] = gameobjectObj;

  if (scene.sceneToNameToId.at(sceneId).find(gameobjectObj.name) != scene.sceneToNameToId.at(sceneId).end()){
    modassert(false, std::string("name already exists: " + gameobjectObj.name))
  }
  scene.sceneToNameToId.at(sceneId)[gameobjectObj.name] = gameobjectObj.id;
  return gameobjectObj.id;
}

SceneDeserialization createSceneFromParsedContent(
  objid sceneId,
  std::vector<Token> tokens,  
  std::function<objid()> getNewObjectId,
  std::function<std::set<std::string>(std::string&)> getObjautoserializerFields,
  std::optional<objid> prefabId
){
  Scene scene;

  auto dividedTokens = divideMainAndSubelementTokens(tokens);
  auto serialGameAttrs = deserializeSceneTokens(dividedTokens.mainTokens);
  auto subelementAttrs = deserializeSceneTokens(dividedTokens.subelementTokens);


  std::map<std::string, GameObject> gameobjs;
  for (auto [name, attrWithChildren] : serialGameAttrs){
    std::string value = name;
    auto idValue = objIdFromAttribute(attrWithChildren.attr);
    objid id = idValue.has_value() ? idValue.value() : getNewObjectId();
    gameobjs[value] = gameObjectFromFields(value, id, attrWithChildren.attr, getObjautoserializerFields(value), false);
  }

  scene.sceneToNameToId[sceneId] = {};

  for (auto [name, gameobjectObj] : gameobjs){
    modassert(name == gameobjectObj.name, "names do not match");
    sandboxAddToScene(scene, sceneId, std::nullopt, gameobjectObj, prefabId);
  }

  std::map<std::string, GameobjAttributes> additionalFields;
  for (auto [name, attrWithChildren] : serialGameAttrs){
    for (auto childName : attrWithChildren.children){
      auto parentId = scene.sceneToNameToId.at(sceneId).at(name);
      enforceParentRelationship(scene, scene.sceneToNameToId.at(sceneId).at(childName), parentId);
    }
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

// todo if this is an existing scene, it doesn't sponsor tags 
AddSceneDataValues addSceneDataToScenebox(SceneSandbox& sandbox, std::string sceneFileName, objid sceneId, std::string sceneData,  std::optional<std::string> name, std::optional<std::vector<std::string>> tags, std::function<std::set<std::string>(std::string&)> getObjautoserializerFields, std::optional<objid> parentId, std::optional<objid> prefabId){
  for (auto &[_, metadata] : sandbox.sceneIdToSceneMetadata){ // all scene names should be unique
    if (metadata.name == name && name != std::nullopt){
      modassert(false, std::string("scene name already exists: ") + name.value());
    }
  }


  auto tokens = parseFormat(sceneData);
  SceneDeserialization deserializedScene = createSceneFromParsedContent(sceneId, tokens, getUniqueObjId, getObjautoserializerFields, prefabId);

  for (auto &[id, obj] : deserializedScene.scene.idToGameObjects){
    modassert(sandbox.mainScene.idToGameObjects.find(id) == sandbox.mainScene.idToGameObjects.end(), "duplicate id");
    sandbox.mainScene.idToGameObjects[id] = obj;
    modlog("sandbox add id", std::to_string(id));
  }
  for (auto &[id, obj] : deserializedScene.scene.idToGameObjectsH){
    sandbox.mainScene.idToGameObjectsH[id] = obj;
    if (parentId.has_value() && obj.parentId == 0){
      enforceParentRelationship(sandbox.mainScene, id, parentId.value());
    }
  }

  bool existingScene = sandbox.mainScene.sceneToNameToId.find(sceneId) != sandbox.mainScene.sceneToNameToId.end();
  if (!existingScene){
    assert(sandbox.mainScene.sceneToNameToId.find(sceneId) ==  sandbox.mainScene.sceneToNameToId.end());
    sandbox.mainScene.sceneToNameToId[sceneId] = {};
    sandbox.sceneIdToSceneMetadata[sceneId] = SceneMetadata{
      .scenefile = sceneFileName,
      .name = name,
      .tags = tags.has_value() ? tags.value() : std::vector<std::string>({}),
    }; 
  }

  for (auto &[name, id] : deserializedScene.scene.sceneToNameToId.at(sceneId)){
    modassert(sandbox.mainScene.sceneToNameToId.at(sceneId).find(name) == sandbox.mainScene.sceneToNameToId.at(sceneId).end(), "duplicate name");
    sandbox.mainScene.sceneToNameToId.at(sceneId)[name] = id;
  }


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
  std::function<std::set<std::string>(std::string&)> getObjautoserializerFields,
  std::set<objid> boneIds,
  std::optional<objid> prefabId
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
    auto isBone = boneIds.count(nodeId) > 0;
    auto gameobj = gameObjectFromFields(name, id, defaultAttributesForMultiObj(transform, rootObj, additionalFields.at(nodeId)), getObjautoserializerFields(name), isBone);
    gameobj.transformation.rotation = transform.rotation; // todo make this work w/ attributes better

    modassert(names.at(nodeId) == gameobj.name, "names do not match");
    auto addedId = sandboxAddToScene(scene, sceneId, std::nullopt, gameobj, prefabId);
    addedIds.push_back(addedId);
    scene.idToGameObjectsH.at(id).groupId = rootId;
  }

  for (auto [childId, parentId] : childToParent){
    auto realChildId = nodeIdToRealId.at(childId);
    auto realParentId = parentId == rootIdNode ? rootId : nodeIdToRealId.at(parentId);
    enforceParentRelationship(scene, realChildId, realParentId);
  }
  for (auto id : addedIds){
    addObjectToCache(sandbox, id);
  }
  return nameToAdditionalFields;
} 

void addGameObjectToScene(SceneSandbox& sandbox, objid sceneId, std::string name, GameObject& gameobjectObj, std::vector<std::string> children, std::optional<objid> prefabId){
  modassert(name == gameobjectObj.name, "names do not match");
  auto addedId = sandboxAddToScene(sandbox.mainScene, sceneId, std::nullopt, gameobjectObj, prefabId);      
  for (auto child : children){
    if (sandbox.mainScene.sceneToNameToId.at(sceneId).find(child) == sandbox.mainScene.sceneToNameToId.at(sceneId).end()){
       // @TODO - shouldn't be an error should automatically create instead
      std::cout << "ERROR: NOT YET IMPLEMENTED : ADDING OBJECT WITH CHILD THAT DOES NOT EXIST IN THE SCENE" << std::endl;
      assert(false);
    }
    enforceParentRelationship(sandbox.mainScene, sandbox.mainScene.sceneToNameToId.at(sceneId).at(child), sandbox.mainScene.sceneToNameToId.at(sceneId).at(name));  
  }
  addObjectToCache(sandbox, addedId);
}

void traverseNodes(Scene& scene, objid id, std::function<void(objid)> onAddObject){
  auto parentObjH = scene.idToGameObjectsH.at(id);
  onAddObject(parentObjH.id);
  for (objid id : parentObjH.children){
    traverseNodes(scene, id, onAddObject);
  }
}
std::set<objid> getChildrenIdsAndParent(Scene& scene,  objid id){
  std::set<objid> objectIds;
  auto onAddObject = [&objectIds](objid id) -> void {
    objectIds.insert(id);
  };
  traverseNodes(scene, id, onAddObject);
  return objectIds;
}

// Conceptually needs => get all ids for this scene starting from this id
std::vector<objid> idsToRemoveFromScenegraph(SceneSandbox& sandbox, objid id){
  modassert(id != 0, "cannot remove the root node");
  std::vector<objid> allObjects;
  auto objects = getChildrenIdsAndParent(sandbox.mainScene, id);
  for (auto id : objects){
    allObjects.push_back(id);
  }
  return allObjects;
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
    modlog("sandbox remove id", std::to_string(id));
    removeObjectFromCache(sandbox, id);

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

std::vector<objid> listObjAndDescInScene(SceneSandbox& sandbox, objid sceneId){
  std::set<objid> ids;

  auto allIdsInScene = listObjInScene(sandbox, sceneId);
  for (auto id : allIdsInScene){
    auto allDescendents = getChildrenIdsAndParent(sandbox.mainScene, id);
    for (auto desc : allDescendents){
      ids.insert(desc);
    }
  }

  std::vector<objid> finalIds;
  for (auto id : ids){
    finalIds.push_back(id);
  }
  return finalIds;

}


// TODO PEROBJECT
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
  modassert(scene.idToGameObjects.find(id) != scene.idToGameObjects.end(), "getGameObject gameobj does not exist");
  return scene.idToGameObjects.at(id);
}
GameObjectH& getGameObjectH(Scene& scene, objid id){
  modassert(scene.idToGameObjectsH.find(id) != scene.idToGameObjectsH.end(), "getGameObjectH gameobjh does not exist");
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
std::optional<objid> parentId(Scene& scene, objid id){
  return scene.idToGameObjectsH.at(id).parentId;
}

std::vector<std::string> childnamesNoPrefabs(SceneSandbox& sandbox, GameObjectH& gameobjecth){   
  std::vector<std::string> childnames;
  for (auto childid : gameobjecth.children){
    auto childH = getGameObjectH(sandbox, childid);
    if (!childH.prefabId.has_value() &&  childH.groupId == childid){
      childnames.push_back(getGameObject(sandbox, childid).name);
    }
  }
  return childnames;
}
std::string serializeScene(SceneSandbox& sandbox, objid sceneId, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds){
  std::string sceneData = "# Generated scene \n";
  modassert(sceneId != 0, "cannot serialize the main scene");
  for (auto [id, obj]: sandbox.mainScene.idToGameObjectsH){
    if (obj.sceneId != sceneId){
      continue;
    }
    auto gameobj = getGameObject(sandbox, id);
    auto gameobjecth = getGameObjectH(sandbox, id);
    auto additionalFields = getAdditionalFields(id); 
    auto children = childnamesNoPrefabs(sandbox, gameobjecth);
    sceneData = sceneData + serializeObj(id, gameobjecth.groupId, gameobj, children, includeIds, additionalFields,  "");
  }
  return sceneData;
}

SceneSandbox createSceneSandbox(std::vector<LayerInfo> layers, std::function<std::set<std::string>(std::string&)> getObjautoserializerFields){
  Scene mainScene {
    .idToGameObjects = {},
    .idToGameObjectsH = {},
    .sceneToNameToId = {{ 0, {}}},
    .absoluteTransforms = {},
  };
  std::sort(std::begin(layers), std::end(layers), [](LayerInfo layer1, LayerInfo layer2) { return layer1.zIndex < layer2.zIndex; });

  std::string name = "root";
  auto rootObj = gameObjectFromFields(name, 0, GameobjAttributes {}, getObjautoserializerFields(name), false); 
  auto rootObjId = sandboxAddToScene(mainScene, 0, std::nullopt, rootObj, std::nullopt);

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
  for (auto [sceneId, _] : sandbox.sceneIdToSceneMetadata){
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
  if (enablePrefixMatch){ // TODO - can i get rid of this
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
  modassert(gameobj.has_value(), std::string("game obj does not exist: ") + name);
  GameObject& obj = *(gameobj.value()); 
  return obj;
}
GameObjectH& getGameObjectH(SceneSandbox& sandbox, objid id){
  return getGameObjectH(sandbox.mainScene, id);
}
GameObjectH& getGameObjectH(SceneSandbox& sandbox, std::string name, objid sceneId){
  auto gameobj = maybeGetGameObjectByName(sandbox, name, sceneId, false);
  modassert(gameobj.has_value(), std::string("game obj does not exist: ") + name);
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
  if (parentId == 0){
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

void updateAllChildrenPositions(SceneSandbox& sandbox, objid updatedId, bool justAdded){
  //std::cout << "should update id: " << updatedId << std::endl;
  // do a breath first search, and then update it in that order
  auto updatedIdElements = bfsElementAndChildren(sandbox, updatedId);
  //std::cout << "should update: " << print(updatedIdElements) << std::endl;
  for (auto id : updatedIdElements){
    if (id != updatedId){
      sandbox.updatedIds.insert(id);  
    }
    if (sandbox.mainScene.absoluteTransforms.find(id) == sandbox.mainScene.absoluteTransforms.end()){
      continue;
    }
    auto parentId = getGameObjectH(sandbox, id).parentId;
    if (parentId == 0){
      continue;
    }
    if (sandbox.mainScene.absoluteTransforms.find(parentId) == sandbox.mainScene.absoluteTransforms.end()){
      continue;
    }
    GameObject& gameobj = getGameObject(sandbox, id);
    if (gameobj.physicsOptions.isStatic || !gameobj.physicsOptions.enabled || justAdded /* this is so when you spawn it, the relative transform determines where it goes */){
       auto currentConstraint = gameobj.transformation;
       auto newTransform = calcAbsoluteTransform(sandbox, parentId, currentConstraint);
       sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
         .transform = newTransform,
         .matrix = matrixFromComponents(newTransform),
       };       
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
    .matrix = elementMatrix,
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
    .matrix = matrixFromComponents(transform),
  };

  if (parentId != 0){
    auto oldRelativeTransform = calcRelativeTransform(sandbox, id);
    getGameObject(sandbox, id).transformation = oldRelativeTransform;
    //modassert(!aboutEqual(oldRelativeTransform.scale.x, 0.f), "0 scale for x component");
    //modassert(!aboutEqual(oldRelativeTransform.scale.y, 0.f), "0 scale for y component");
    //modassert(!aboutEqual(oldRelativeTransform.scale.z, 0.f), "0 scale for z component");
  }else{
    getGameObject(sandbox, id).transformation = transform;
  }
  updateAllChildrenPositions(sandbox, id);
}
void updateRelativeTransform(SceneSandbox& sandbox, objid id, Transformation transform){
  auto parentId = getGameObjectH(sandbox, id).parentId;
  getGameObject(sandbox, id).transformation = transform;
  auto newTransform = parentId == 0 ? transform : calcAbsoluteTransform(sandbox, parentId, transform);
  sandbox.mainScene.absoluteTransforms.at(id) = TransformCacheElement {
    .transform = newTransform,
    .matrix = matrixFromComponents(newTransform),
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
  return element.matrix;
}
Transformation fullTransformation(SceneSandbox& sandbox, objid id){
  return sandbox.mainScene.absoluteTransforms.at(id).transform;
}

void removeScene(SceneSandbox& sandbox, objid sceneId){
  modassert(sceneId != 0, "cannot remove root scene");
  modassert (sandbox.mainScene.sceneToNameToId.find(sceneId) != sandbox.mainScene.sceneToNameToId.end(), "scene does not exist");
  //removeObjectsFromScenegraph(sandbox, listObjAndDescInScene(sandbox, sceneId)); // @TODO this should get children too
  sandbox.sceneIdToSceneMetadata.erase(sceneId);
  sandbox.mainScene.sceneToNameToId.erase(sceneId); 
}
bool sceneExists(SceneSandbox& sandbox, objid sceneId){
  return !(sandbox.sceneIdToSceneMetadata.find(sceneId) == sandbox.sceneIdToSceneMetadata.end());
}    

bool hasDescendent(SceneSandbox& sandbox, objid id, objid descendent){
  auto allDescendents = getChildrenIdsAndParent(sandbox.mainScene, id);
  return (allDescendents.count(descendent) > 0);
}

void makeParent(SceneSandbox& sandbox, objid child, objid parent){
  modassert(child != parent, "cannot parent a node to itself");
  modassert(!hasDescendent(sandbox, child, parent), "cannot parent a node to a descendent of itself");
  enforceParentRelationship(sandbox.mainScene, child, parent);
  updateAllChildrenPositions(sandbox, parent, true); // TODO - only update the newly parented children
}

std::optional<objid> getParent(SceneSandbox& sandbox, objid id){
  GameObjectH& gameobjecth = getGameObjectH(sandbox, id);
  if (gameobjecth.parentId == 0){
    return std::nullopt;
  }
  return gameobjecth.parentId;
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

std::optional<objid> sceneIdByName(SceneSandbox& sandbox, std::string name){
  for (auto &[sceneId, metadata] : sandbox.sceneIdToSceneMetadata){
    if (metadata.name == name){
      return sceneId;
    }
  }
  return std::nullopt;
}

objid rootSceneId(SceneSandbox& sandbox){
  return 0;
}