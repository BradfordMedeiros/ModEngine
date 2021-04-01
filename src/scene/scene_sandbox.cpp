#include "./scene_sandbox.h"


//////////////////////
void addObjectToScene(Scene& scene, objid parentId, std::string name, GameObject& gameobjectObj){
  auto gameobjectH = GameObjectH {
    .id = gameobjectObj.id,
    .parentId = parentId,
    .groupId = gameobjectObj.id,
  };
  assert(scene.idToGameObjectsH.find(gameobjectObj.id) == scene.idToGameObjectsH.end());
  scene.idToGameObjectsH[gameobjectObj.id] = gameobjectH;
  scene.idToGameObjects[gameobjectObj.id] = gameobjectObj;
  assert(scene.nameToId.find(name) == scene.nameToId.end());
  scene.nameToId[name] = gameobjectObj.id;
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

SceneDeserialization createSceneFromParsedContent(
  std::vector<Token> tokens,  
  std::function<objid()> getNewObjectId, 
  std::vector<LayerInfo> layers
){
  Scene scene;
  std::sort(std::begin(layers), std::end(layers), [](LayerInfo layer1, LayerInfo layer2) { return layer1.zIndex < layer2.zIndex; });
  scene.layers = layers;

  auto serialGameAttrs = deserializeSceneTokens(tokens);

  auto rootId = getNewObjectId();
  auto rootName = "~root:" + std::to_string(rootId);
  scene.rootId = rootId;

  assert(serialGameAttrs.find(rootName) == serialGameAttrs.end());
  assert(rootName.find(',') == std::string::npos);
  serialGameAttrs[rootName] = GameobjAttributes{ .stringAttributes = {{"physics", "disabled"}}};

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
    addObjectToScene(scene, -1, name, gameobjectObj);
  }

  for (auto [name, gameobj] : serialGameAttrs){
    for (auto childName : gameobj.children){
      enforceParentRelationship(scene, scene.nameToId.at(childName), name);
    }
  }
  enforceRootObjects(scene);

  scene.isNested = false;

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

SceneDeserialization deserializeScene(std::string content, std::function<objid()> getNewObjectId, std::vector<LayerInfo> layers){
  std::cout << "INFO: Deserialization: " << std::endl;
  return createSceneFromParsedContent(parseFormat(content), getNewObjectId, layers);
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

    addObjectToScene(scene, -1, names.at(nodeId), gameobj);
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

std::vector<std::string> childnames(Scene& scene, GameObjectH& gameobjecth){   
  std::vector<std::string> childnames;
  for (auto childid : gameobjecth.children){
    auto childH = scene.idToGameObjectsH.at(childid);
    if (childH.groupId == childid){
      childnames.push_back(scene.idToGameObjects.at(childid).name);
    }
  }
  return childnames;
}

std::string serializeObject(Scene& scene, objid id, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds, std::string overridename){
  auto gameobjecth = scene.idToGameObjectsH.at(id);
  auto gameobj = scene.idToGameObjects.at(id);
  auto objectSerialization = serializeObj(
    id, 
    gameobjecth.groupId, 
    gameobj, 
    childnames(scene, gameobjecth), 
    includeIds, 
    getAdditionalFields(id), 
    overridename
  );
  return objectSerialization;
}

void addGameObjectToScene(Scene& scene, std::string name, GameObject& gameobjectObj, std::vector<std::string> children){
   // @TODO - this is a bug sort of.  If this layer does not exist in the scene already, it should be added. 
  // Result if it doesn't exist is that it just doesn't get rendered, so nbd, but it really probably should be rendered (probably as a new layer with max depth?)
  addObjectToScene(scene, -1, name, gameobjectObj);      
  for (auto child : children){
    if (scene.nameToId.find(child) == scene.nameToId.end()){
       // @TODO - shouldn't be an error should automatically create instead
      std::cout << "ERROR: NOT YET IMPLEMENTED : ADDING OBJECT WITH CHILD THAT DOES NOT EXIST IN THE SCENE" << std::endl;
      assert(false);
    }
    enforceParentRelationship(scene, scene.nameToId.at(child), name);  
  }
  enforceRootObjects(scene);
}

void traverseNodes(Scene& scene, objid id, std::function<void(objid)> onAddObject){
  auto parentObjH = scene.idToGameObjectsH.at(id);
  onAddObject(parentObjH.id);
  for (objid id : parentObjH.children){
    traverseNodes(scene, id, onAddObject);
  }
}

std::vector<objid> getChildrenIdsAndParent(Scene& scene, objid id){
  std::vector<objid> objectIds;
  auto onAddObject = [&objectIds](objid id) -> void {
    objectIds.push_back(id);
  };
  traverseNodes(scene, id, onAddObject);
  return objectIds;
}

std::vector<objid> idsToRemoveFromScenegraph(Scene& scene, objid id){
  auto objects = getChildrenIdsAndParent(scene, id);
  assert(id != scene.rootId);
  return objects;
}

void removeObjectsFromScenegraph(Scene& scene, std::vector<objid> objects){  // it might make sense to check if any layers here are not present and then 
  for (auto id : objects){
    std::string objectName = scene.idToGameObjects.at(id).name;
    scene.idToGameObjects.erase(id);
    scene.idToGameObjectsH.erase(id);
    scene.nameToId.erase(objectName);
    for (auto &[_, objh] : scene.idToGameObjectsH){
      objh.children.erase(id);
    }
  }
}

std::vector<objid> listObjInScene(Scene& scene){
  std::vector<objid> allObjects;
  for (auto const&[id, _] : scene.idToGameObjects){
    allObjects.push_back(id);
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
void traverseScene(Scene& scene, glm::mat4 initialModel, glm::vec3 totalScale, std::function<void(objid, glm::mat4, glm::mat4, bool, bool, std::string)> onObject){
  std::vector<traversalData> datum;

  objid id = scene.rootId;
  traverseScene(id, scene.idToGameObjectsH.at(id), scene, initialModel, totalScale, [&datum](objid foundId, glm::mat4 modelMatrix, glm::mat4 parentMatrix, std::string) -> void {
    datum.push_back(traversalData{
      .id = foundId,
      .modelMatrix = modelMatrix,
      .parentMatrix = parentMatrix,
    });
  });
  

  for (auto layer : scene.layers){      // @TODO could organize this before to not require pass on each frame
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
objid getGroupId(Scene& scene, objid id){
  return scene.idToGameObjectsH.at(id).groupId; 
}
bool idExists(Scene& scene, objid id){
  return scene.idToGameObjects.find(id) != scene.idToGameObjects.end();
}
objid parentId(Scene& scene, objid id){
  return scene.idToGameObjectsH.at(id).parentId;
}




///////////////////////////////////////////
std::string serializeScene(SceneSandbox& sandbox, objid sceneId, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds){
  Scene& scene = sandbox.scenes.at(sceneId);

  std::string sceneData = "# Generated scene \n";
  for (auto [id, _]: scene.idToGameObjectsH){
    if (id == scene.rootId){
      continue;
    }
    sceneData = sceneData + serializeObject(scene, id, getAdditionalFields, includeIds, "");
  }
  return sceneData;
}

///////////////////////


SceneSandbox createSceneSandbox(){
  SceneSandbox sandbox {};
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
GameObject& getGameObject(SceneSandbox& sandbox, std::string name){
  auto gameobj = maybeGetGameObjectByName(sandbox, name);
  GameObject& obj = *(gameobj.value()); 
  return obj;
}

void traverseScene(SceneSandbox& sandbox, Scene& scene, glm::mat4 initialModel, glm::vec3 scale, std::function<void(objid, glm::mat4, glm::mat4, bool, bool, std::string)> onObject){
  traverseScene(scene, initialModel, scale, onObject);
}

void traverseScene(SceneSandbox& sandbox, Scene& scene, std::function<void(objid, glm::mat4, glm::mat4, bool, bool, std::string)> onObject){
  if (scene.isNested){
    return;
  }
  traverseScene(sandbox, scene, glm::mat4(1.f), glm::vec3(1.f, 1.f, 1.f), onObject);
}

void traverseSandbox(SceneSandbox& sandbox, std::function<void(objid, glm::mat4, glm::mat4, bool, bool, std::string)> onObject){
  for (auto &[_, scene] : sandbox.scenes){
    traverseScene(sandbox, scene, onObject);
  }
}

glm::mat4 fullModelTransform(SceneSandbox& sandbox, objid id){
  Scene& scene = sceneForId(sandbox, id);
  while(scene.isNested){
    scene = sceneForId(sandbox, parentId(scene, scene.rootId));
  }
  
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

AddSceneDataValues addSceneDataToScenebox(SceneSandbox& sandbox, objid sceneId, std::string sceneData, std::vector<LayerInfo> layers){
  assert(sandbox.scenes.find(sceneId) == sandbox.scenes.end());
  SceneDeserialization deserializedScene = deserializeScene(sceneData, getUniqueObjId, layers);
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

std::map<std::string,  std::map<std::string, std::string>> multiObjAdd(
  SceneSandbox& sandbox,
  objid rootId,
  objid rootIdNode, 
  std::map<objid, objid> childToParent, 
  std::map<objid, Transformation> gameobjTransforms, 
  std::map<objid, std::string> names, 
  std::map<objid, std::map<std::string, std::string>> additionalFields,
  std::function<objid()> getNewObjectId){
  Scene& scene = sceneForId(sandbox, rootId);
  return addSubsceneToRoot(scene, rootId, rootIdNode, childToParent, gameobjTransforms, names, additionalFields, getNewObjectId);
}