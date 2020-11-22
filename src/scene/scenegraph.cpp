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

SceneDeserialization createSceneFromParsedContent(
  ParsedContent parsedContent,  
  std::function<objid()> getNewObjectId
){
  Scene scene;
  auto tokens = parsedContent.tokens;
  std::sort(std::begin(parsedContent.layers), std::end(parsedContent.layers), [](LayerInfo layer1, LayerInfo layer2) { return layer1.zIndex < layer2.zIndex; });
  scene.layers = parsedContent.layers;

  std::map<std::string, SerializationObject>  serialObjs = deserializeSerialObjs(tokens);
  
  auto rootId = getNewObjectId();
  auto rootName = "~root:" + std::to_string(rootId);
  scene.rootId = rootId;
  assert(serialObjs.find(rootName) == serialObjs.end());

  assert(rootName.find(',') == std::string::npos);
  serialObjs[rootName] = getDefaultObject("default");
  serialObjs[rootName].physics.enabled = false; // todo see if this can be removed

  std::map<std::string, GameObject> gameobjs;

  for (auto [name, serialObj] : serialObjs){
    if (name != rootName){
      objid id = serialObj.hasId ? serialObj.id : getNewObjectId();
      auto gameobjectObj = gameObjectFromParam(name, id, serialObj);
      gameobjs[name] = gameobjectObj;
    }else{
      auto gameobjectObj = gameObjectFromParam(name, scene.rootId, serialObj);
      gameobjs[name] = gameobjectObj;
    }
  }

  for (auto [name, gameobjectObj] : gameobjs){
    addObjectToScene(scene, -1, name, gameobjectObj);
  }

  for (auto [name, serialObj] : serialObjs){
    for (auto childName : serialObj.children){
      enforceParentRelationship(scene, scene.nameToId.at(childName), name);
    }
  }
  enforceRootObjects(scene);

  scene.isNested = false;

  std::map<std::string, std::map<std::string, std::string>> additionalFields;
  std::map<std::string, glm::vec3>  tints; 
  for (auto &[name, serialObj] : serialObjs){
    additionalFields[name] = serialObj.additionalFields;
    tints[name] = serialObj.tint;
  }

  SceneDeserialization deserializedScene {
    .scene = scene,
    .additionalFields = additionalFields,
    .tints = tints,
  };
  return deserializedScene;
}

SceneDeserialization deserializeScene(std::string content, std::function<objid()> getNewObjectId){
  std::cout << "INFO: Deserialization: " << std::endl;
  return createSceneFromParsedContent(parseFormat(content), getNewObjectId);
}

std::map<std::string, SerializationObject> addSubsceneToRoot(
  Scene& scene, 
  objid rootId, 
  objid rootIdNode, 
  std::map<objid, objid> childToParent, 
  std::map<objid, Transformation> gameobjTransforms, 
  std::map<objid, std::string> names,
  std::map<objid, std::map<std::string, std::string>> additionalFields,
  std::function<objid()> getNewObjectId,
  glm::vec3 parentTint
){
  std::map<std::string, SerializationObject> serialObjs;
  std::map<objid, objid> nodeIdToRealId;
  for (auto [nodeId, transform] : gameobjTransforms){
    if (nodeId == rootIdNode){
      continue;
    }
    objid id = getNewObjectId();
    nodeIdToRealId[nodeId] = id;

    auto rootObj = scene.idToGameObjects.at(rootId);
  
    std::map<std::string, std::string> stringAttributes;
    std::map<std::string, double> numAttributes;
    std::map<std::string, glm::vec3> vecAttributes;  

    GameobjAttributes attributes {
      .stringAttributes = stringAttributes,
      .numAttributes = numAttributes,
      .vecAttributes = vecAttributes,
    };

    auto defaultObject = getDefaultObject("default");
    setSerialObjFromAttr(defaultObject, attributes);

    SerializationObject obj {
      .position = transform.position,
      .scale = transform.scale,
      .rotation = transform.rotation,
      .physics = defaultObject.physics, // see notes around nocollide + physics enabled 
      .lookat = "",
      .layer = rootObj.layer,
      .script = "",
      .fragshader = rootObj.fragshader,
      .netsynchronize = false,
      .tint = parentTint,
      .additionalFields = additionalFields.at(nodeId)
    };
    serialObjs[names.at(nodeId)] = obj;

    auto gameobjectObj = gameObjectFromParam(names.at(nodeId), id, obj);
    addObjectToScene(scene, -1, names.at(nodeId), gameobjectObj);
    scene.idToGameObjectsH.at(id).groupId = rootId;
  }

  for (auto [childId, parentId] : childToParent){
    auto realChildId = nodeIdToRealId.at(childId);
    auto realParentId = parentId == rootIdNode ? rootId : nodeIdToRealId.at(parentId);
    enforceParentRelationship(scene, realChildId, scene.idToGameObjects.at(realParentId).name);
  }
  enforceRootObjects(scene);

  return serialObjs;
} 

bool isDefaultPosition(glm::vec3 pos){
  return pos.x == 0 && pos.y == 0 && pos.z == 0;
}
bool isIdentityVec(glm::vec3 scale){
  return scale.x = 1 && scale.y == 1 && scale.z == 1;
}
bool isDefaultGravity(glm::vec3 gravity){
  return gravity.x == 0 && (gravity.y < -9.80 && gravity.y > -9.82) && gravity.z == 0;
}

std::string serializeObject(Scene& scene, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds, objid id, std::string name){
  std::string sceneData = "";
  auto gameobjecth = scene.idToGameObjectsH.at(id);
  if (gameobjecth.groupId != id){
    return sceneData;
  }
  GameObject gameobject = scene.idToGameObjects.at(id);
  std::string gameobjectName = name == "" ? gameobject.name : name;
  
  if (gameobjecth.children.size() > 0){
    std::vector<std::string> childnames;
    for (auto childid : gameobjecth.children){
      childnames.push_back(scene.idToGameObjects.at(childid).name);
    }
    sceneData = sceneData + gameobjectName + ":child:" + join(childnames, ',') + "\n";
  }

  if (includeIds){
    sceneData = sceneData + gameobjectName + ":id:" + std::to_string(gameobject.id) + "\n";
  }
  if (!isDefaultPosition(gameobject.transformation.position)){
    sceneData = sceneData + gameobjectName + ":position:" + serializeVec(gameobject.transformation.position) + "\n";
  }
  if (!isIdentityVec(gameobject.transformation.scale)){
    sceneData = sceneData + gameobjectName + ":scale:" + serializeVec(gameobject.transformation.scale) + "\n";
  }
  sceneData = sceneData + gameobjectName + ":rotation:" + serializeRotation(gameobject.transformation.rotation) + "\n";

  if (!gameobject.physicsOptions.enabled){
    sceneData = sceneData + gameobjectName + ":physics:disabled" + "\n"; 
  }
  if (!gameobject.physicsOptions.isStatic){
    sceneData = sceneData + gameobjectName + ":physics_type:dynamic" + "\n"; 
  }
  if (!gameobject.physicsOptions.hasCollisions){
    sceneData = sceneData + gameobjectName + ":physics_collision:nocollide" + "\n"; 
  }
  if (gameobject.physicsOptions.shape == BOX){
    sceneData = sceneData + gameobjectName + ":physics_shape:shape_box" + "\n"; 
  }
  if (gameobject.physicsOptions.shape == SPHERE){
    sceneData = sceneData + gameobjectName + ":physics_shape:shape_sphere" + "\n"; 
  }
  if (!isIdentityVec(gameobject.physicsOptions.linearFactor)){
    sceneData = sceneData + gameobjectName + ":physics_linear:" + serializeVec(gameobject.physicsOptions.linearFactor) + "\n"; 
  }
  if (!isIdentityVec(gameobject.physicsOptions.angularFactor)){
    sceneData = sceneData + gameobjectName + ":physics_angle:" + serializeVec(gameobject.physicsOptions.angularFactor) + "\n"; 
  }
  if (!isDefaultGravity(gameobject.physicsOptions.gravity)){
    sceneData = sceneData + gameobjectName + ":physics_gravity:" + serializeVec(gameobject.physicsOptions.gravity) + "\n"; 
  }
  if (gameobject.lookat != ""){
    sceneData = sceneData + gameobjectName + ":lookat:" + gameobject.lookat + "\n"; 
  }
  if (gameobject.script != ""){
    sceneData = sceneData + gameobjectName + ":script:" + gameobject.script + "\n"; 
  }
  if (gameobject.fragshader != ""){
    sceneData = sceneData + gameobjectName + ":fragshader:" + gameobject.fragshader + "\n";
  }
  if (gameobject.netsynchronize){
    sceneData = sceneData + gameobjectName + ":net:sync" + "\n";
  }
  if (!isIdentityVec(gameobject.tint)){
    sceneData = sceneData + gameobjectName + ":tint:" + serializeVec(gameobject.tint) + "\n";
  }

  for (auto additionalFields : getAdditionalFields(id)){
    sceneData = sceneData + gameobjectName + ":" + additionalFields.first + ":" + additionalFields.second + "\n";
  }
  return sceneData;
}

std::string serializeScene(Scene& scene, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds){
  std::string sceneData = "# Generated scene \n";
  for (auto [id, _]: scene.idToGameObjectsH){
    if (id == scene.rootId){
      continue;
    }
    sceneData = sceneData + serializeObject(scene, getAdditionalFields, includeIds, id, "");
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

void traverseScene(objid id, GameObjectH objectH, Scene& scene, glm::mat4 model, glm::vec3 totalScale, std::function<void(objid, glm::mat4, glm::mat4, std::string, glm::vec3)> onObject, std::function<void(objid, glm::mat4, glm::vec3)> traverseLink){
  if (objectH.linkOnly){
    traverseLink(id, model, totalScale);
    return;
  }

  GameObject object = scene.idToGameObjects.at(objectH.id);
  glm::mat4 modelMatrix = glm::translate(model, object.transformation.position);
  modelMatrix = modelMatrix * glm::toMat4(object.transformation.rotation);
  
  glm::vec3 scaling = object.transformation.scale * totalScale;  // having trouble with doing the scaling here so putting out of band.   Anyone in the ether please help if more elegant. 
  glm::mat4 scaledModelMatrix = modelMatrix * glm::scale(glm::mat4(1.f), scaling);

  onObject(id, scaledModelMatrix, model, "", object.tint);

  for (objid id: objectH.children){
    traverseScene(id, scene.idToGameObjectsH.at(id), scene, modelMatrix, scaling, onObject, traverseLink);
  }
}

struct traversalData {
  objid id;
  glm::mat4 modelMatrix;
  glm::mat4 parentMatrix;
};
void traverseScene(Scene& scene, glm::mat4 initialModel, glm::vec3 totalScale, std::function<void(objid, glm::mat4, glm::mat4, bool, std::string, glm::vec3)> onObject, std::function<void(objid, glm::mat4, glm::vec3)> traverseLink){
  std::vector<traversalData> datum;

  objid id = scene.rootId;
  traverseScene(id, scene.idToGameObjectsH.at(id), scene, initialModel, totalScale, [&datum](objid foundId, glm::mat4 modelMatrix, glm::mat4 parentMatrix, std::string, glm::vec3) -> void {
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
        onObject(data.id, data.modelMatrix, data.parentMatrix, layer.orthographic, gameobject.fragshader, gameobject.tint);
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

std::map<std::string, std::string> scenegraphAttributes(Scene& scene, objid id){
  std::map<std::string, std::string> attributes;

  auto gameobj = scene.idToGameObjects.at(id);

  // todo missing physics 
  // TODO I should expose the real types here not strings
  attributes["position"] = print(gameobj.transformation.position);
  attributes["scale"] = print(gameobj.transformation.scale);
  attributes["rotation"] = print(gameobj.transformation.rotation);

  if (gameobj.lookat != ""){
    attributes["lookat"] = gameobj.lookat;
  }
  if (gameobj.layer != ""){
    attributes["layer"] = gameobj.layer;
  }
  if (gameobj.script != ""){
    attributes["script"] = gameobj.script;
  }
  if (gameobj.fragshader != ""){
    attributes["fragshader"] = gameobj.fragshader;
  }
  return attributes;
}
void setScenegraphAttributes(Scene& scene, objid id, std::map<std::string, std::string> attributes){
  GameObject& obj = scene.idToGameObjects.at(id);

  if (attributes.find("position") != attributes.end()){
    obj.transformation.position = parseVec(attributes.at("position"));
  }
  if (attributes.find("scale") != attributes.end()){
    obj.transformation.scale = parseVec(attributes.at("scale"));
  }
  // @TODO add more overrideable fields
}

std::map<std::string, SerializationObject> deserializeSerialObjs(std::vector<Token> tokens){
  std::map<std::string, SerializationObject> objects;
  auto objectAttributes = deserializeSceneTokens(tokens);

  for (auto &[name, attributes] : objectAttributes){
    SerializationObject object { };
    setSerialObjFromAttr(object, attributes);
    objects[name] = object;
  }
  return objects;
}
