#include "./scene_sandbox.h"

const bool enableTransformLogging = false;
bool transformLoggingEnabled(){
  return enableTransformLogging;
}

const bool assertOnStale = false;


void addObjectToCache(SceneSandbox& sandbox, objid id);
void removeObjectFromCache(SceneSandbox& sandbox, objid id);

struct TransformUpdateValue {
  glm::vec3 position;
  glm::vec3 scale;
  glm::quat rotation;
  Transformation transform;
};

struct TransformUpdate2 {
  bool relative;
  objid id;
  bool hasPosition = false;
  bool hasScale = false;
  bool hasRotation = false;
  TransformUpdateValue transform;
  Hint hint;

  std::optional<objid> directIndex;
};


std::string print(TransformUpdateValue& update){
  return "placeholder for TransformUpdate2";
}


GameObject& getGameObjectDirectIndex(SceneSandbox& sandbox, objid id){
  GameObjectBuffer& buffer = sandbox.mainScene.gameobjects[id];
  //modassert(buffer.inUse, "gameobject is stale");
  return buffer.gameobj;
}

// This means I just keep on expanding larger. 
// I might want to just make this fixed size to begin with but the budget is hard to say. 
void addNextFree(Scene& scene, GameObject& gameobj, GameObjectH& gameobjh){
  std::cout << "change gameobject: add " << gameobj.id << std::endl;

  modassert(scene.idToGameObjectsH.find(gameobj.id) == scene.idToGameObjectsH.end(), "duplicate id");

  scene.idToGameObjectsH[gameobj.id] = gameobjh;

  for (int i = 0; i < scene.gameobjects.size(); i++){
    if (!scene.gameobjects.at(i).inUse){
      scene.gameobjects.at(i).gameobj = gameobj;
      scene.gameobjects.at(i).inUse = true;
      scene.idToGameObjectsH.at(gameobj.id).directIndex = i;
      scene.idToDirectIndex[gameobj.id] = i;
      return;
    }
  }
  scene.gameobjects.push_back(GameObjectBuffer {
    .inUse = true,
    .gameobj = gameobj,
  });
  scene.idToGameObjectsH.at(gameobj.id).directIndex = (scene.gameobjects.size() - 1);
  scene.idToDirectIndex[gameobj.id] = (scene.gameobjects.size() - 1);
}
void freeGameObject(GameObjectBuffer& obj){
  std::cout << "change gameobject: remove " << obj.gameobj.id << std::endl;
  obj.inUse = false;
}

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
  modassert(scene.sceneToNameToId.find(sceneId) != scene.sceneToNameToId.end(), std::string("scene does not exist: ") + std::to_string(sceneId));

  addNextFree(scene, gameobjectObj, gameobjectH);

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


  std::unordered_map<std::string, GameObject> gameobjs;
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

  std::unordered_map<std::string, GameobjAttributes> additionalFields;
  for (auto [name, attrWithChildren] : serialGameAttrs){
    for (auto childName : attrWithChildren.children){
      auto parentId = scene.sceneToNameToId.at(sceneId).at(name);
      enforceParentRelationship(scene, scene.sceneToNameToId.at(sceneId).at(childName), parentId);
    }
    additionalFields[name] = attrWithChildren.attr;
  }

  std::unordered_map<std::string, GameobjAttributes> subelementAttributes;
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

  for (auto &obj : deserializedScene.scene.gameobjects){
    if (!obj.inUse){
      continue;
    }
    addNextFree(sandbox.mainScene, obj.gameobj, getGameObjectH(deserializedScene.scene, obj.gameobj.id));
  }


  for (auto &[id, obj] : deserializedScene.scene.idToGameObjectsH){
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

  for (auto &obj : deserializedScene.scene.gameobjects){
    if (!obj.inUse){
      continue;
    }
    addObjectToCache(sandbox, obj.gameobj.id);
  }


  std::unordered_map<std::string, GameobjAttributesWithId>  additionalFields;
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

GameObject defaultObj{};
GameObject& getGameObject(Scene& scene, objid id){
  for (auto &obj: scene.gameobjects){
    if (!obj.inUse){
      continue;
    }
    if (obj.gameobj.id == id){
      return obj.gameobj;
    }
  }
  modassert(false, "cannot find object");
  return defaultObj;
}
GameObject& getGameObject(Scene& scene, objid id, objid* gameobjIndex){
  for (int i = 0; i < scene.gameobjects.size(); i++){
    auto& obj = scene.gameobjects.at(i);
    if (!obj.inUse){
      continue;
    }
    if (obj.gameobj.id == id){
      *gameobjIndex = i;
      return obj.gameobj;
    }
  }
  modassert(false, "cannot find object");
  return defaultObj;
}

GameObjectH& getGameObjectH(Scene& scene, objid id){
  auto it = scene.idToGameObjectsH.find(id);
  modassert(it != scene.idToGameObjectsH.end(), "getGameObjectH gameobjh does not exist");
  return it -> second;
}

std::optional<GameObject*> maybeGetGameObjectByName(SceneSandbox& sandbox, std::string& name, objid sceneId){
  for (auto &gameObj : sandbox.mainScene.gameobjects){
    if (!gameObj.inUse){
      continue;
    }
    auto id = gameObj.gameobj.id;
    if (gameObj.gameobj.name == name && getGameObjectH(sandbox.mainScene, id).sceneId == sceneId){
      return &gameObj.gameobj;      
    }
  }
  return std::nullopt;
}

GameObject& getGameObject(SceneSandbox& sandbox, objid id){
  return getGameObject(sandbox.mainScene, id);
}
GameObject& getGameObject(SceneSandbox& sandbox, std::string name, objid sceneId){
  auto gameobj = maybeGetGameObjectByName(sandbox, name, sceneId);
  modassert(gameobj.has_value(), std::string("game obj does not exist: ") + name);
  GameObject& obj = *(gameobj.value()); 
  return obj;
}
GameObjectH& getGameObjectH(SceneSandbox& sandbox, objid id){
  return getGameObjectH(sandbox.mainScene, id);
}
GameObjectH& getGameObjectH(SceneSandbox& sandbox, std::string name, objid sceneId){
  auto gameobj = maybeGetGameObjectByName(sandbox, name, sceneId);
  modassert(gameobj.has_value(), std::string("game obj does not exist: ") + name);
  GameObject& obj = *(gameobj.value()); 
  GameObjectH& objh = getGameObjectH(sandbox, obj.id);
  return objh;
}
/////////////////////////////////////////////////////////////


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
){
  Scene& scene = sandbox.mainScene; 
  std::vector<LayerInfo>& layers = sandbox.layers;

  std::unordered_map<std::string,  GameobjAttributesWithId> nameToAdditionalFields;
  std::unordered_map<objid, objid> nodeIdToRealId;
  auto rootObj = getGameObject(scene, rootId);
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

void traverseNodes(Scene& scene, objid id, std::set<objid>& objectIds){
  auto parentObjH = scene.idToGameObjectsH.at(id);
  objectIds.insert(parentObjH.id);
  for (objid id : parentObjH.children){
    traverseNodes(scene, id, objectIds);
  }
}
std::set<objid> getChildrenIdsAndParent(Scene& scene,  objid id){
  std::set<objid> objectIds;
  traverseNodes(scene, id, objectIds);
  return objectIds;
}

// Conceptually needs => get all ids for this scene starting from this id
std::set<objid> idsToRemoveFromScenegraph(SceneSandbox& sandbox, objid id){
  modassert(id != 0, "cannot remove the root node");
  auto objects = getChildrenIdsAndParent(sandbox.mainScene, id);
  return objects;
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

std::set<objid> listObjAndDescInScene(SceneSandbox& sandbox, objid sceneId){
  std::set<objid> ids;
  auto allIdsInScene = listObjInScene(sandbox, sceneId);
  for (auto id : allIdsInScene){
    auto allDescendents = getChildrenIdsAndParent(sandbox.mainScene, id);
    for (auto desc : allDescendents){
      ids.insert(desc);
    }
  }
  return ids;
}

void removeObjectsFromScenegraph(SceneSandbox& sandbox, std::set<objid> objects){  
  for (auto id : objects){
    Scene& scene = sandbox.mainScene;

    std::string& objectName = getGameObject(scene, id).name;
    auto sceneId = scene.idToGameObjectsH.at(id).sceneId;
    for (auto &obj : scene.gameobjects){
      if (obj.gameobj.id == id){
        freeGameObject(obj);
      }
    }
    scene.idToGameObjectsH.erase(id);
    scene.idToDirectIndex.erase(id);
    modlog("sandbox remove id", std::to_string(id));
    removeObjectFromCache(sandbox, id);

    scene.sceneToNameToId.at(sceneId).erase(objectName);
    for (auto &[_, objh] : scene.idToGameObjectsH){  
      objh.children.erase(id);
    }
  }
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
    auto& gameobj = getGameObject(sandbox, id);
    auto& gameobjecth = getGameObjectH(sandbox, id);
    auto additionalFields = getAdditionalFields(id); 
    auto children = childnamesNoPrefabs(sandbox, gameobjecth);
    sceneData = sceneData + serializeObj(id, gameobjecth.groupId, gameobj, children, includeIds, additionalFields,  "");
  }
  return sceneData;
}

SceneSandbox createSceneSandbox(std::vector<LayerInfo> layers, std::function<std::set<std::string>(std::string&)> getObjautoserializerFields){
  Scene mainScene {
    .gameobjects = {},
    .idToGameObjectsH = {},
    .sceneToNameToId = {{ 0, {}}},
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

Transformation calcRelativeTransform(Transformation& child, Transformation& parent){
  auto absTransformCache = matrixFromComponents(child); 
  auto parentTransform = matrixFromComponents(parent);
  auto relativeTransform = glm::affineInverse(parentTransform) * absTransformCache;
  return getTransformationFromMatrix(relativeTransform);
}

// What position should the gameobject be based upon the two absolute transform of parent and child
Transformation calcRelativeTransform(SceneSandbox& sandbox, objid childId, objid parentId){
  return calcRelativeTransform(getAbsoluteById(sandbox, childId).transform, getAbsoluteById(sandbox, parentId).transform);
}

Transformation calcRelativeTransform(SceneSandbox& sandbox, objid childId){
  GameObjectH& childObjH = getGameObjectH(sandbox, childId);
  auto parentId = childObjH.parentId;
  if (parentId == 0){
    return sandbox.mainScene.gameobjects.at(childObjH.directIndex).absoluteTransform.transform;
  
  }
  return calcRelativeTransform(sandbox, childId, parentId);
}

Transformation calcAbsoluteTransform(SceneSandbox& sandbox, objid parentId, Transformation child){
    auto& parent = getAbsoluteById(sandbox, parentId).transform;
    Transformation result;
    result.rotation = parent.rotation * child.rotation;
    result.scale = parent.scale * child.scale;
    result.position = parent.position + (parent.rotation * (parent.scale * child.position));
    return result;
}


std::vector<objid> bfsElementAndChildren(SceneSandbox& sandbox, objid updatedId){
  std::vector<objid> ids;
  std::queue<objid> idsToVisit;

  auto currentId = updatedId;
  idsToVisit.push(currentId);
  
  while (idsToVisit.size() > 0){
    auto idToVisit = idsToVisit.front();
    ids.push_back(idToVisit);
    idsToVisit.pop();
    GameObjectH& objH = getGameObjectH(sandbox.mainScene, idToVisit);
    for (objid id : objH.children){
      idsToVisit.push(id);
    }    
  }

  return ids;
}

int getDepth(SceneSandbox& sandbox, objid id){
  if (id == 0){
    return 0;
  }
  objid currentId = id;
  int currDepth = 0;
  while(true){
    currDepth++;
    auto parentId = getParent(sandbox, currentId);
    if (!parentId.has_value()){
      return currDepth;
    }
    currentId = parentId.value();
  }
  return currDepth;
}

void updateAllChildrenPositions(SceneSandbox& sandbox, objid updatedId){
  auto updatedIdElements = bfsElementAndChildren(sandbox, updatedId);
  //std::cout << "should update: " << print(updatedIdElements) << std::endl;
  for (auto id : updatedIdElements){
    if (id != updatedId){
      sandbox.updatedIds.insert(id);  
    }
    if (sandbox.mainScene.idToDirectIndex.find(id) == sandbox.mainScene.idToDirectIndex.end()){
      continue;
    }
    auto parentId = getGameObjectH(sandbox, id).parentId;
    if (parentId == 0){
      continue;
    }
    if (sandbox.mainScene.idToDirectIndex.find(parentId) == sandbox.mainScene.idToDirectIndex.end()){
      continue;
    }

    objid gameobjIndex = 0;
    auto currentConstraint = getGameObject(sandbox.mainScene, id, &gameobjIndex).transformation;
    auto newTransform = calcAbsoluteTransform(sandbox, parentId, currentConstraint);
    if (enableTransformLogging){
      std::cout << inColor("> ", CONSOLE_COLOR_YELLOW) << "hint [child update]          [" << id << " " << inColor(getGameObject(sandbox, id).name, CONSOLE_COLOR_BLUE) << "]" << std::endl; 
      std::cout << inColor("> ", CONSOLE_COLOR_YELLOW) << "hint [child update]               new rel: " << print(currentConstraint) << std::endl; 
      std::cout << inColor("> ", CONSOLE_COLOR_YELLOW) << "hint [child update]               new abs: " << print(newTransform) << std::endl; 
    }

    sandbox.mainScene.gameobjects.at(gameobjIndex).absoluteTransform = TransformCacheElement {
      .transform = newTransform,
    };
  }

  for (auto id : updatedIdElements){
    auto depth = getDepth(sandbox, id);
    getGameObjectH(sandbox, id).depth = depth;
  }
}


int getDirectIndexForId(SceneSandbox& sandbox, objid id){
  return getGameObjectH(sandbox, id).directIndex;
}

std::vector<TransformUpdate2> updates;
int currentFrameTick = 0;

void updateNodes(SceneSandbox& sandbox, objid id){
  std::queue<objid> idsToVisit;  // shouldn't actually be needed since no common children
  auto currentId = id;

  idsToVisit.push(currentId);

  while (idsToVisit.size() > 0){
    auto idToVisit = idsToVisit.front();
    idsToVisit.pop();

    //std::cout << "updateNodes children for: " << id << ", " << objH.children.size() << std::endl;

    auto& objh = getGameObjectH(sandbox, idToVisit);
    if (objh.updateFrame == currentFrameTick){
      continue;
    }
    bool isAbsoluteUpdate = objh.updateAbsoluteFrame == currentFrameTick;
    auto& gameobj = getGameObjectDirectIndex(sandbox, objh.directIndex);

    if (isAbsoluteUpdate){
      auto oldRelativeTransform = calcRelativeTransform(sandbox, idToVisit);
      gameobj.transformation = oldRelativeTransform;
    }else{
      auto parentId = objh.parentId;
      auto newTransform = parentId == 0 ? gameobj.transformation : calcAbsoluteTransform(sandbox, parentId, gameobj.transformation);
      sandbox.mainScene.gameobjects.at(objh.directIndex).absoluteTransform.transform = newTransform;
    }
    objh.updateFrame = currentFrameTick;

    GameObjectH& objH = sandbox.mainScene.idToGameObjectsH.at(idToVisit);
    for (objid childId : objH.children){
      idsToVisit.push(childId);
    }    
  }
}

GameObject& getGameObjectFromUpdate(SceneSandbox& sandbox, TransformUpdate2& update){
  if (update.directIndex.has_value()){
    return getGameObjectDirectIndex(sandbox, update.directIndex.value());
  }
  auto& gameobj = getGameObject(sandbox, update.id);
  return gameobj;
}

std::set<objid> updateSandbox(SceneSandbox& sandbox){
  currentFrameTick++;
  if (enableTransformLogging){
    std::cout << inColor("hint---------- updateSandbox start------------------", CONSOLE_COLOR_RED) << std::endl;
  }

  // Update the relative location for relative updates on the gameobj
  for (auto &update : updates){
    if (update.relative){
      auto id = update.id;
      if (enableTransformLogging){
        std::cout << inColor("updateRelativeTransform hint: ", CONSOLE_COLOR_GREEN) << " [" << id << " " << inColor(getGameObject(sandbox, id).name, CONSOLE_COLOR_BLUE) << "]  " << (update.hint.hint ? update.hint.hint : "[no hint]") << " " <<  inColor(print(update.transform), CONSOLE_COLOR_YELLOW) <<  std::endl;
        std::cout << "updateRelativeTransform hint        old rel: " <<  print(getGameObject(sandbox, id).transformation) << std::endl;        
      }

      auto& gameobj = getGameObjectFromUpdate(sandbox, update);
      if (update.hasPosition){
        gameobj.transformation.position = update.transform.position;
      }
      if (update.hasScale){
        gameobj.transformation.scale = update.transform.scale;
      }
      if (update.hasRotation){
        gameobj.transformation.rotation = update.transform.rotation;
      }

      if (enableTransformLogging){
        std::cout << "updateRelativeTransform hint        new rel: " <<  print(getGameObject(sandbox, id).transformation) << std::endl;
      }
    }
  }

  // Update the absolute transform for absolute transsforms
  for (auto &update : updates){
    if (!update.relative){
      auto& transform = update.transform.transform;
      auto id = update.id;
      auto gameobjIndex = getDirectIndexForId(sandbox, id);

      if (enableTransformLogging){
        std::cout << inColor("updateAbsoluteTransform hint: ", CONSOLE_COLOR_GREEN) <<  " [" << id << " " << inColor(getGameObject(sandbox, id).name, CONSOLE_COLOR_BLUE) << "] " << (update.hint.hint ? update.hint.hint : "[no hint]") << " " << inColor(print(update.transform), CONSOLE_COLOR_YELLOW) << std::endl;
        std::cout << "updateAbsoluteTransform hint        old rel: " <<  print(getGameObject(sandbox, id).transformation) << std::endl;
        std::cout << "updateAbsoluteTransform hint        old abs: " <<  print(getAbsoluteById(sandbox, id).transform) << std::endl;        
      }

      Transformation& newAbsoluteTransform = getAbsoluteById(sandbox, id).transform;
      if (update.hasPosition){
        newAbsoluteTransform.position = update.transform.position;
      }
      if (update.hasScale){
        newAbsoluteTransform.scale = update.transform.scale;
      }
      if (update.hasRotation){
        newAbsoluteTransform.rotation = update.transform.rotation;
      }

      sandbox.mainScene.gameobjects.at(gameobjIndex).absoluteTransform = TransformCacheElement {
        .transform =  newAbsoluteTransform,
      };

      //if (enableTransformLogging){
      //  std::cout << "updateAbsoluteTransform hint        new rel: " <<  print(getGameObject(sandbox, id).transformation) << std::endl;
      //  std::cout << "updateAbsoluteTransform hint        new abs: " <<  print(sandbox.mainScene.absoluteTransforms.at(id).transform) << std::endl;
      //}
    }
  }

  std::vector<int> updateDepths;
  for (auto &update : updates){
    GameObjectH& gameobjh = getGameObjectH(sandbox, update.id);
    auto depth = gameobjh.depth;
    updateDepths.push_back(depth);

    if (!update.relative){
      gameobjh.updateAbsoluteFrame = currentFrameTick;
    }
  }

  {
    int numUpdates = 0;
    int currDepth = 0;
    while(numUpdates < updates.size()){
      for (int i = 0; i < updates.size(); i++){
        auto nodeDepth = updateDepths.at(i);
        if (nodeDepth == currDepth){
          numUpdates++;
          updateNodes(sandbox, updates.at(i).id);
        }
      }
      currDepth++;
    }
  }

  std::set<objid> oldUpdated = sandbox.updatedIds;
  sandbox.updatedIds = {};
  updates = {};

  if (enableTransformLogging){
    std::cout << inColor("hint---------- updateSandbox end------------------", CONSOLE_COLOR_RED) << std::endl;
  }
  return oldUpdated;
}

void addObjectToCache(SceneSandbox& sandbox, objid id){
  objid gameobjIndex = 0;
  GameObject& object = getGameObject(sandbox.mainScene, id, &gameobjIndex);
  auto elementMatrix = matrixFromComponents(
    glm::mat4(1.f),
    object.transformation.position, 
    object.transformation.scale, 
    object.transformation.rotation
  );
  sandbox.mainScene.gameobjects.at(gameobjIndex).absoluteTransform = TransformCacheElement {
    .transform = getTransformationFromMatrix(elementMatrix),
  };
  updateAllChildrenPositions(sandbox, id);
}
void removeObjectFromCache(SceneSandbox& sandbox, objid id){
  sandbox.updatedIds.erase(id);
}

void updateAbsoluteTransform(SceneSandbox& sandbox, objid id, Transformation transform, Hint hint){
  if (enableTransformLogging){
    std::cout << inColor("hint - queue updateAbsoluteTransform: ", CONSOLE_COLOR_GREEN) <<  " [" << id << " " << inColor(getGameObject(sandbox, id).name, CONSOLE_COLOR_BLUE) << "] " << (hint.hint ? hint.hint : "[no hint]") << " " << inColor(print(transform), CONSOLE_COLOR_YELLOW) << std::endl;
  }
  TransformUpdate2 updateValue { };
  updateValue = TransformUpdate2 {
    .relative = false,
    .id = id,
    .hint = hint,
  };
  updateValue.hasPosition = true;
  updateValue.hasScale = true;
  updateValue.hasRotation = true;
  updateValue.transform.position = transform.position;
  updateValue.transform.scale = transform.scale;
  updateValue.transform.rotation = transform.rotation;
  updates.push_back(updateValue);    
}
void updateRelativeTransform(SceneSandbox& sandbox, objid id, Transformation transform, Hint hint, std::optional<int> directIndex){
  if (enableTransformLogging){
    std::cout << inColor("hint updateRelativeTransform queue: ", CONSOLE_COLOR_GREEN) <<  " [" << id << " " << inColor(getGameObject(sandbox, id).name, CONSOLE_COLOR_BLUE) << "] " << (hint.hint ? hint.hint : "[no hint]") << " " << inColor(print(transform), CONSOLE_COLOR_YELLOW) << std::endl;
  }

  TransformUpdate2 updateValue { };
  updateValue =  TransformUpdate2 {
    .relative = true,
    .id = id,
    .hint = hint,
  };
  updateValue.hasPosition = true;
  updateValue.hasScale = true;
  updateValue.hasRotation = true;
  updateValue.transform.position = transform.position;
  updateValue.transform.scale = transform.scale;
  updateValue.transform.rotation = transform.rotation;
  updateValue.directIndex = directIndex;
  updates.push_back(updateValue);    
}

std::set<objid> getAncestors(SceneSandbox& sandbox, objid id){
  std::set<objid> ids;
  auto parentId = getParent(sandbox, id);
  while(parentId.has_value()){
    ids.insert(parentId.value());
    parentId = getParent(sandbox, parentId.value());
  }
  return ids;
}

bool isStale(SceneSandbox& sandbox, objid id, bool* indirect){
  bool stale = false;
  *indirect = false;
  auto ancestors = getAncestors(sandbox, id);
  for (auto &transform : updates){  // this should also check if a parent has a write 
    if ((transform.id == id)){
      stale = true;
      break;
    }
    if (ancestors.count(transform.id) > 0){
      stale = true;
      *indirect = true;
      break;
    }
  }
  return stale;
}

TransformCacheElement& getAbsoluteById(SceneSandbox& sandbox, objid id){
  auto directIndex = getDirectIndexForId(sandbox, id);
  return sandbox.mainScene.gameobjects.at(directIndex).absoluteTransform;
}

glm::mat4 fullModelTransform(SceneSandbox& sandbox, objid id, const char* hint){
  if (enableTransformLogging){
    bool indirect = false;
    auto stale = isStale(sandbox, id, &indirect);
    std::cout << inColor("hint - readTransform", CONSOLE_COLOR_GREEN) << ": [" << id << ", " << inColor(getGameObject(sandbox, id).name, CONSOLE_COLOR_BLUE) << "]" " " << (hint ? hint : "[no hint]") << (stale ? inColor("  WARNING - STALE READ", CONSOLE_COLOR_RED) : "") << ((stale && indirect) ? inColor(" - INDIRECT ", CONSOLE_COLOR_RED) : "") << std::endl;
    modassert(!assertOnStale || (assertOnStale && !stale), "stale transform");
  }
  TransformCacheElement& element = getAbsoluteById(sandbox, id);
  return matrixFromComponents(element.transform);
}
glm::mat4 fullModelTransformDirect(SceneSandbox& sandbox, objid directIndex, const char* hint){
  if (enableTransformLogging){
    modassert(false, "enableTransformLogging not implemented for fullModelTransformDirect");
    //bool indirect = false;
    //auto stale = isStale(sandbox, id, &indirect);
    //std::cout << inColor("hint - readTransform", CONSOLE_COLOR_GREEN) << ": [" << id << ", " << inColor(getGameObject(sandbox, id).name, CONSOLE_COLOR_BLUE) << "]" " " << (hint ? hint : "[no hint]") << (stale ? inColor("  WARNING - STALE READ", CONSOLE_COLOR_RED) : "") << ((stale && indirect) ? inColor(" - INDIRECT ", CONSOLE_COLOR_RED) : "") << std::endl;
    //modassert(!assertOnStale || (assertOnStale && !stale), "stale transform");
  }
  TransformCacheElement& element = sandbox.mainScene.gameobjects.at(directIndex).absoluteTransform;
  return matrixFromComponents(element.transform);
}

Transformation& fullTransformation(SceneSandbox& sandbox, objid id, const char* hint){
  if (enableTransformLogging){
    bool indirect = false;
    auto stale = isStale(sandbox, id, &indirect);
    std::cout << inColor("hint - readTransform", CONSOLE_COLOR_GREEN) << ": [" << id << ", " << inColor(getGameObject(sandbox, id).name, CONSOLE_COLOR_BLUE) << "]" " " << (hint ? hint : "[no hint]") << (stale ? inColor("  WARNING - STALE READ", CONSOLE_COLOR_RED) : "") << ((stale && indirect) ? inColor(" - INDIRECT ", CONSOLE_COLOR_RED) : "") << std::endl;
    modassert(!assertOnStale || (assertOnStale && !stale), "stale transform");
  }
  return getAbsoluteById(sandbox, id).transform;
}

void removeScene(SceneSandbox& sandbox, objid sceneId){
  modassert(sceneId != 0, "cannot remove root scene");
  modassert (sandbox.mainScene.sceneToNameToId.find(sceneId) != sandbox.mainScene.sceneToNameToId.end(), "scene does not exist");
  //removeObjectsFromScenegraph(sandbox, listObjAndDescInScene(sandbox, sceneId)); // @TODO this should get children too
  sandbox.sceneIdToSceneMetadata.erase(sceneId);
  sandbox.mainScene.sceneToNameToId.erase(sceneId); 
}

bool hasDescendent(SceneSandbox& sandbox, objid id, objid descendent){
  auto allDescendents = getChildrenIdsAndParent(sandbox.mainScene, id);
  return (allDescendents.count(descendent) > 0);
}

void makeParent(SceneSandbox& sandbox, objid child, objid parent){
  modassert(child != parent, "cannot parent a node to itself");
  modassert(!hasDescendent(sandbox, child, parent), "cannot parent a node to a descendent of itself");
  enforceParentRelationship(sandbox.mainScene, child, parent);
  updateAllChildrenPositions(sandbox, parent); // TODO - only update the newly parented children
}



///////////////////////////////////////////////////////////////////////////////


// Misc utility fns 
////////////////////////////////////////////////////////////////////////
void forEveryGameobj(SceneSandbox& sandbox, std::function<void(objid id, GameObject& gameobj)> onElement){
  for (auto &gameObj : sandbox.mainScene.gameobjects){
    if (gameObj.inUse){
      onElement(gameObj.gameobj.id, gameObj.gameobj); 
    }
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

std::optional<objid> getParent(SceneSandbox& sandbox, objid id){
  GameObjectH& gameobjecth = getGameObjectH(sandbox, id);
  if (gameobjecth.parentId == 0){
    return std::nullopt;
  }
  return gameobjecth.parentId;
}

objid getIdForName(SceneSandbox& sandbox, std::string name, objid sceneId){
  auto gameobj = maybeGetGameObjectByName(sandbox, name, sceneId);
  return gameobj.value() -> id;
}

objid getGroupId(Scene& scene, objid id){
  return getGameObjectH(scene, id).groupId; 
}
objid getGroupId(SceneSandbox& sandbox, objid id){
  return getGroupId(sandbox.mainScene, id); 
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
std::vector<objid> getIdsInGroupByObjId(SceneSandbox& sandbox, objid index){
  return getIdsInGroup(sandbox.mainScene, getGroupId(sandbox, index));
}

bool idExists(Scene& scene, objid id){
  for (auto &obj : scene.gameobjects){
    if (!obj.inUse){
      continue;
    }
    if (obj.gameobj.id == id){
      return true;
    }
  }
  return false;
}
bool idExists(SceneSandbox& sandbox, objid id){
  return sandbox.mainScene.idToGameObjectsH.find(id) != sandbox.mainScene.idToGameObjectsH.end();
}
bool idExists(SceneSandbox& sandbox, std::string name, objid sceneId){
  return maybeGetGameObjectByName(sandbox, name, sceneId).has_value();
}

bool sceneExists(SceneSandbox& sandbox, objid sceneId){
  return !(sandbox.sceneIdToSceneMetadata.find(sceneId) == sandbox.sceneIdToSceneMetadata.end());
}    
objid sceneId(SceneSandbox& sandbox, objid id){
  return getGameObjectH(sandbox.mainScene, id).sceneId;
}

std::vector<objid> getByName(SceneSandbox& sandbox, std::string name){
  std::vector<objid> ids;
  for (auto &gameobj : sandbox.mainScene.gameobjects){
    if (!gameobj.inUse){
      continue;
    }
    if (gameobj.gameobj.name == name){
      ids.push_back(gameobj.gameobj.id);
    }
  }
  return ids;
}

objid rootSceneId(SceneSandbox& sandbox){
  return 0;
}

std::optional<objid> sceneIdByName(SceneSandbox& sandbox, std::string name){
  for (auto &[sceneId, metadata] : sandbox.sceneIdToSceneMetadata){
    if (metadata.name == name){
      return sceneId;
    }
  }
  return std::nullopt;
}

int getNumberOfObjects(SceneSandbox& sandbox){
  return sandbox.mainScene.gameobjects.size();
}
int getNumberScenesLoaded(SceneSandbox& sandbox){
  return sandbox.mainScene.sceneToNameToId.size();
}


// For convenience, just use the absolute / relative transform
////////////////////////////////////////////////////////////////////////
void updateAbsolutePosition(SceneSandbox& sandbox, objid id, glm::vec3 position, Hint hint){
  if (enableTransformLogging){
    std::cout << inColor("hint updateAbsolutePosition queue: ", CONSOLE_COLOR_GREEN) <<  " [" << id << " " << inColor(getGameObject(sandbox, id).name, CONSOLE_COLOR_BLUE) << "] " << (hint.hint ? hint.hint : "[no hint]") << " " << inColor(print(position), CONSOLE_COLOR_YELLOW) << std::endl;
  }
  TransformUpdate2 updateValue { };
  updateValue = TransformUpdate2 {
    .relative = false,
    .id = id,
    .hint = hint,
  };
  updateValue.hasPosition = true;
  updateValue.transform.position = position;
  updates.push_back(updateValue);    
}
void updateRelativePosition(SceneSandbox& sandbox, objid id, glm::vec3 position, Hint hint){
  if (enableTransformLogging){
    std::cout << inColor("hint updateRelativePosition queue: ", CONSOLE_COLOR_GREEN) <<  " [" << id << " " << inColor(getGameObject(sandbox, id).name, CONSOLE_COLOR_BLUE) << "] " << (hint.hint ? hint.hint : "[no hint]") << " " << inColor(print(position), CONSOLE_COLOR_YELLOW) << std::endl;
  }
  TransformUpdate2 updateValue { };
  updateValue = TransformUpdate2 {
    .relative = true,
    .id = id,
    .hint = hint,
  };
  updateValue.hasPosition = true;
  updateValue.transform.position = position;
  updates.push_back(updateValue);    
};;
void updateAbsoluteScale(SceneSandbox& sandbox, objid id, glm::vec3 scale, Hint hint){
  if (enableTransformLogging){
    std::cout << inColor("hint updateAbsoluteScale queue: ", CONSOLE_COLOR_GREEN) <<  " [" << id << " " << inColor(getGameObject(sandbox, id).name, CONSOLE_COLOR_BLUE) << "] " << (hint.hint ? hint.hint : "[no hint]") << " " << inColor(print(scale), CONSOLE_COLOR_YELLOW) << std::endl;
  }
  TransformUpdate2 updateValue { };
  updateValue = TransformUpdate2 {
    .relative = false,
    .id = id,
    .hint = hint,
  };
  updateValue.hasScale = true;
  updateValue.transform.scale = scale;
  updates.push_back(updateValue);    
}
void updateAbsoluteRotation(SceneSandbox& sandbox, objid id, glm::quat rotation, Hint hint){
  if (enableTransformLogging){
    std::cout << inColor("hint updateAbsoluteRotation queue: ", CONSOLE_COLOR_GREEN) <<  " [" << id << " " << inColor(getGameObject(sandbox, id).name, CONSOLE_COLOR_BLUE) << "] " << (hint.hint ? hint.hint : "[no hint]") << " " << inColor(print(rotation), CONSOLE_COLOR_YELLOW) << std::endl;
  }
  TransformUpdate2 updateValue { };
  updateValue = TransformUpdate2 {
    .relative = false,
    .id = id,
    .hint = hint,
  };
  updateValue.hasRotation = true;
  updateValue.transform.rotation = rotation;;
  updates.push_back(updateValue);    
}
void updateRelativeRotation(SceneSandbox& sandbox, objid id, glm::quat rotation, Hint hint){
  if (enableTransformLogging){
    std::cout << inColor("hint updateRelativeRotation queue: ", CONSOLE_COLOR_GREEN) <<  " [" << id << " " << inColor(getGameObject(sandbox, id).name, CONSOLE_COLOR_BLUE) << "] " << (hint.hint ? hint.hint : "[no hint]") << " " << inColor(print(rotation), CONSOLE_COLOR_YELLOW) << std::endl;
  }
  TransformUpdate2 updateValue { };
  updateValue = TransformUpdate2 {
    .relative = true,
    .id = id,
    .hint = hint,
  };
  updateValue.hasRotation = true;
  updateValue.transform.rotation = rotation;
  updates.push_back(updateValue);    
  
}