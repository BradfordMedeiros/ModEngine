#include "./scenegraph.h"

void addObjectToScene(Scene& scene, objid parentId, std::string name, GameObject& gameobjectObj){
  auto gameobjectH = GameObjectH {
    .id = gameobjectObj.id,
    .parentId = parentId,
    .groupId = gameobjectObj.id,
    .linkOnly = false
  };
  scene.idToGameObjectsH[gameobjectObj.id] = gameobjectH;
  scene.idToGameObjects[gameobjectObj.id] = gameobjectObj;
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

std::string layerForObject(ParsedContent& parseFormat, std::string objname){
  for (auto token : parseFormat.tokens){
    if (token.target == objname){
      return token.layer;
    }
  }
  return "default";
}

SceneDeserialization createSceneFromParsedContent(
  ParsedContent parsedContent,  
  std::function<objid()> getNewObjectId
){
  Scene scene;
  auto tokens = parsedContent.tokens;
  std::sort(std::begin(parsedContent.layers), std::end(parsedContent.layers), [](LayerInfo layer1, LayerInfo layer2) { return layer1.zIndex < layer2.zIndex; });
  scene.layers = parsedContent.layers;

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
      gameobjs[name] = gameObjectFromFields(name, layerForObject(parsedContent, name), id, gameAttr);
    }else{
      gameobjs[name] = gameObjectFromFields(name, layerForObject(parsedContent, name), scene.rootId, gameAttr); 
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

SceneDeserialization deserializeScene(std::string content, std::function<objid()> getNewObjectId){
  std::cout << "INFO: Deserialization: " << std::endl;
  return createSceneFromParsedContent(parseFormat(content), getNewObjectId);
}

GameobjAttributes defaultAttributesForMultiObj(Transformation transform, GameObject& gameobj){
  GameobjAttributes attributes {
    .stringAttributes = {
      {"fragshader", gameobj.fragshader},
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

    auto gameobj = gameObjectFromFields(names.at(nodeId), rootObj.layer, id, defaultAttributesForMultiObj(transform, rootObj));
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

std::string serializeScene(Scene& scene, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds){
  std::string sceneData = "# Generated scene \n";
  for (auto [id, _]: scene.idToGameObjectsH){
    if (id == scene.rootId){
      continue;
    }
    sceneData = sceneData + serializeObject(scene, id, getAdditionalFields, includeIds, "");
  }
  return sceneData;
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

void addChildLink(Scene& scene, objid childId, objid parentId){
  auto gameobjectH = GameObjectH {
    .id = childId,
    .parentId = parentId,
    .linkOnly = true,
  };
  scene.idToGameObjectsH[gameobjectH.id] = gameobjectH;
  enforceParentRelationship(scene, gameobjectH.id, scene.idToGameObjects.at(parentId).name);
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

void traverseScene(objid id, GameObjectH objectH, Scene& scene, glm::mat4 model, glm::vec3 totalScale, std::function<void(objid, glm::mat4, glm::mat4, std::string)> onObject, std::function<void(objid, glm::mat4, glm::vec3)> traverseLink){
  if (objectH.linkOnly){
    traverseLink(id, model, totalScale);
    return;
  }

  GameObject object = scene.idToGameObjects.at(objectH.id);
  glm::mat4 modelMatrix = glm::translate(model, object.transformation.position);
  modelMatrix = modelMatrix * glm::toMat4(object.transformation.rotation);
  
  glm::vec3 scaling = object.transformation.scale * totalScale;  // having trouble with doing the scaling here so putting out of band.   Anyone in the ether please help if more elegant. 
  glm::mat4 scaledModelMatrix = modelMatrix * glm::scale(glm::mat4(1.f), scaling);

  onObject(id, scaledModelMatrix, model, "");

  for (objid id: objectH.children){
    traverseScene(id, scene.idToGameObjectsH.at(id), scene, modelMatrix, scaling, onObject, traverseLink);
  }
}

struct traversalData {
  objid id;
  glm::mat4 modelMatrix;
  glm::mat4 parentMatrix;
};
void traverseScene(Scene& scene, glm::mat4 initialModel, glm::vec3 totalScale, std::function<void(objid, glm::mat4, glm::mat4, bool, std::string)> onObject, std::function<void(objid, glm::mat4, glm::vec3)> traverseLink){
  std::vector<traversalData> datum;

  objid id = scene.rootId;
  traverseScene(id, scene.idToGameObjectsH.at(id), scene, initialModel, totalScale, [&datum](objid foundId, glm::mat4 modelMatrix, glm::mat4 parentMatrix, std::string) -> void {
    datum.push_back(traversalData{
      .id = foundId,
      .modelMatrix = modelMatrix,
      .parentMatrix = parentMatrix,
    });
  }, traverseLink);
  

  for (auto layer : scene.layers){      // @TODO could organize this before to not require pass on each frame
    for (auto data : datum){
      auto gameobject = scene.idToGameObjects.at(data.id);
      if (gameobject.layer == layer.name){
        onObject(data.id, data.modelMatrix, data.parentMatrix, layer.orthographic, gameobject.fragshader);
      }
    }  
  }
}

Transformation getTransformationFromMatrix(glm::mat4 matrix){
  glm::vec3 scale;
  glm::quat rotation;
  glm::vec3 translation;
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::decompose(matrix, scale, rotation, translation, skew, perspective);
  Transformation transform = {
    .position = translation,
    .scale = scale,
    .rotation = rotation,
  };
  return transform;  
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