#include "./scenegraph.h"

physicsOpts defaultPhysicsOpts(){
  physicsOpts defaultOption = {
    .enabled = false,
    .isStatic = true,
    .hasCollisions = true,
    .shape = BOX,
  };
  return defaultOption;
}

GameObject getGameObject(glm::vec3 position, std::string name, short id, std::string lookat, std::string layer, std::string script){
  GameObject gameObject = {
    .id = id,
    .name = name,
    .transformation = Transformation {
      .position = position,
      .scale = glm::vec3(1.0f, 1.0f, 1.0f),
      .rotation = glm::identity<glm::quat>(),
    },
    .physicsOptions = defaultPhysicsOpts(),
    .lookat =  lookat,
    .layer = layer,
    .script = script,
  };
  return gameObject;
}

void addObjectToScene(Scene& scene, glm::vec3 position, std::string name, short id, short parentId, std::string lookat, std::string layer, std::string script){
  auto gameobjectObj = getGameObject(position, name, id, lookat, layer, script);

  auto gameobjectH = GameObjectH {
    .id = gameobjectObj.id,
    .parentId = parentId,
    .groupId = gameobjectObj.id
  };

  scene.idToGameObjectsH[gameobjectObj.id] = gameobjectH;
  scene.idToGameObjects[gameobjectObj.id] = gameobjectObj;
  scene.nameToId[name] = gameobjectObj.id;
}

SceneDeserialization createSceneFromParsedContent(
  ParsedContent parsedContent,  
  std::vector<Field> fields,
  short (*getNewObjectId)()
){
  Scene scene;
  auto tokens = parsedContent.tokens;
  std::sort(std::begin(parsedContent.layers), std::end(parsedContent.layers), [](LayerInfo layer1, LayerInfo layer2) { return layer1.zIndex < layer2.zIndex; });
  scene.layers = parsedContent.layers;

  std::map<std::string, SerializationObject>  serialObjs = deserializeSceneTokens(tokens, fields);
  for (auto [_, serialObj] : serialObjs){
    short id = getNewObjectId();
    addObjectToScene(scene, glm::vec3(1.f, 1.f, 1.f), serialObj.name, id, -1, serialObj.lookat, serialObj.layer, serialObj.script);
    scene.idToGameObjects.at(id).transformation.position = serialObj.position;
    scene.idToGameObjects.at(id).transformation.scale = serialObj.scale;
    scene.idToGameObjects.at(id).transformation.rotation = serialObj.rotation;
    scene.idToGameObjects.at(id).physicsOptions = serialObj.physics;
  }

  for (auto [_, serialObj] : serialObjs){
    if (serialObj.hasParent){
      scene.idToGameObjectsH.at(scene.nameToId.at(serialObj.name)).parentId = scene.nameToId.at(serialObj.parentName);
      scene.idToGameObjectsH.at(scene.nameToId.at(serialObj.parentName)).children.insert(scene.idToGameObjectsH.at(scene.nameToId.at(serialObj.name)).id);
    }
  }

  for( auto const& [id, gameobjectH] : scene.idToGameObjectsH ){
    if (gameobjectH.parentId == -1){
      scene.rootGameObjectsH.push_back(gameobjectH.id);
    }
  }

  SceneDeserialization deserializedScene {
    .scene = scene,
    .serialObjs = serialObjs
  };
  return deserializedScene;
}


SceneDeserialization deserializeScene(
  std::string content,  
  std::vector<Field> fields,  
  short (*getNewObjectId)()
){
  std::cout << "INFO: Deserialization: " << std::endl;
  return createSceneFromParsedContent(parseFormat(content), fields, getNewObjectId);
}

std::map<std::string, SerializationObject> addSubsceneToRoot(
  Scene& scene, 
  short rootId, 
  short rootIdNode, 
  std::map<short, short> childToParent, 
  std::map<short, Transformation> gameobjTransforms, 
  std::map<short, std::string> names,
  std::map<short, std::map<std::string, std::string>> additionalFields,
  short (*getNewObjectId)()
){

  std::map<std::string, SerializationObject> serialObjs;
  std::map<short, short> nodeIdToRealId;
  for (auto [nodeId, transform] : gameobjTransforms){
    if (nodeId == rootIdNode){
      continue;
    }
    short id = getNewObjectId();
    nodeIdToRealId[nodeId] = id;

    auto layer = scene.idToGameObjects.at(rootId).layer;
    addObjectToScene(scene, glm::vec3(1.f, 1.f, 1.f), names.at(nodeId), id, -1, "", layer, "");
    scene.idToGameObjectsH.at(id).groupId = rootId;
    scene.idToGameObjects.at(id).transformation.position = transform.position;
    scene.idToGameObjects.at(id).transformation.scale = transform.scale;
    scene.idToGameObjects.at(id).transformation.rotation = transform.rotation;
    //  default physics options here  scene.idToGameObjects.at(id).physicsOptions

    // @TODO this is duplicated, should just be fn (gameobj -> serializationObject)
    serialObjs[names.at(nodeId)] = SerializationObject {
      .name = names.at(nodeId),  // @TODO this is probably not unique name so probably will be bad
      .position = transform.position,
      .scale = transform.scale,
      .rotation = transform.rotation,
      .hasParent = false,
      .parentName = "-",
      .physics = defaultPhysicsOpts(),
      .type = "default",
      .layer = layer,
      .additionalFields = additionalFields.at(nodeId)
    };  
  }

  for (auto [childId, parentId] : childToParent){
    auto realChildId = nodeIdToRealId.at(childId);
    auto realParentId = parentId == rootIdNode ? rootId : nodeIdToRealId.at(parentId);
    scene.idToGameObjectsH.at(realChildId).parentId = realParentId;
    scene.idToGameObjectsH.at(realParentId).children.insert(realChildId);
  }

  return serialObjs;
} 

std::string serializeVec(glm::vec3 vec){
  return std::to_string(vec.x) + " " + std::to_string(vec.y) + " " + std::to_string(vec.z);
}
std::string serializeRotation(glm::quat rotation){
  float xx = rotation.x;
  float yy = rotation.y;
  float zz = rotation.z;
  glm::vec3 angles = eulerAngles(rotation);
  return std::to_string(angles.x) + " " + std::to_string(angles.y) + " " + std::to_string(angles.z - M_PI); 
}
std::string serializeScene(Scene& scene, std::function<std::vector<std::pair<std::string, std::string>>(short)> getAdditionalFields){
  std::string sceneData = "# Generated scene \n";
  for (auto [id, gameobjecth]: scene.idToGameObjectsH){
    GameObject gameobject = scene.idToGameObjects[id];
    std::string gameobjectName = gameobject.name;
    std::string parentName = scene.idToGameObjects[gameobjecth.parentId].name;

    sceneData = sceneData + gameobjectName + ":position:" + serializeVec(gameobject.transformation.position) + "\n";
    sceneData = sceneData + gameobjectName + ":scale:" + serializeVec(gameobject.transformation.scale) + "\n";
    sceneData = sceneData + gameobjectName + ":rotation:" + serializeRotation(gameobject.transformation.rotation) + "\n";

    for (auto additionalFields : getAdditionalFields(id)){
      sceneData = sceneData + gameobjectName + ":" + additionalFields.first + ":" + additionalFields.second + "\n";
    }

    if (parentName != ""){
      sceneData =  sceneData + gameobjectName + ":parent:" + parentName + "\n";
    }
  }
  return sceneData;
}

SerializationObject  makeObjectInScene(
  Scene& scene, 
  std::string name, 
  std::string mesh, 
  glm::vec3 position, 
  std::string layer,
  short (*getNewObjectId)(),
  std::vector<Field> fields
){
  auto objectId = getNewObjectId();

   // @TODO - this is a bug sort of.  If this layer does not exist in the scene already, it should be added. 
  // Result if it doesn't exist is that it just doesn't get rendered, so nbd, but it really probably should be rendered (probably as a new layer with max depth?)
  addObjectToScene(scene, position, name, objectId, -1, "", layer, "");      

  std::map<std::string, std::string> additionalFields;   // This is a hack, this needs to come from serialization
  additionalFields["mesh"] = mesh;  

  scene.rootGameObjectsH.push_back(objectId);

  auto objectAdded = scene.idToGameObjects.at(objectId);
  SerializationObject serialObj {
    .name = objectAdded.name,
    .position = objectAdded.transformation.position,
    .scale = objectAdded.transformation.scale,
    .rotation = objectAdded.transformation.rotation,
    .hasParent = false, 
    .parentName = "",
    .physics = objectAdded.physicsOptions,
    .type = getType(objectAdded.name, fields),
    .lookat = objectAdded.lookat,
    .layer = objectAdded.layer,
    .script = objectAdded.script,
    .additionalFields = additionalFields,
  };
  return serialObj;
}


void traverseNodes(Scene& scene, short id, std::function<void(short)> onAddObject){
  auto parentObjH = scene.idToGameObjectsH[id];
  onAddObject(parentObjH.id);
  for (short id : parentObjH.children){
    traverseNodes(scene, id, onAddObject);
  }
}

std::vector<short> getChildrenIdsAndParent(Scene& scene, short id){
  std::vector<short> objectIds;
  auto onAddObject = [&objectIds](short id) -> void {
    objectIds.push_back(id);
  };
  traverseNodes(scene, id, onAddObject);
  return objectIds;
}

void removeObjectFromScene(Scene& scene, short id){
  auto objects = getChildrenIdsAndParent(scene, id);
  for (auto id : objects){
    std::string objectName = scene.idToGameObjects[id].name;
    scene.rootGameObjectsH.erase(std::remove(scene.rootGameObjectsH.begin(), scene.rootGameObjectsH.end(), id), scene.rootGameObjectsH.end());  
    scene.idToGameObjects.erase(id);
    scene.idToGameObjectsH.erase(id);
    scene.nameToId.erase(objectName);
  }
}

std::vector<short> listObjInScene(Scene& scene){
  std::vector<short> allObjects;
  for (auto const&[id, _] : scene.idToGameObjects){
    allObjects.push_back(id);
  }
  return allObjects;
}

void traverseScene(short id, GameObjectH objectH, Scene& scene, glm::mat4 model, glm::vec3 totalScale, std::function<void(short, glm::mat4, glm::mat4)> onObject){
  GameObject object = scene.idToGameObjects[objectH.id];
  glm::mat4 modelMatrix = glm::translate(model, object.transformation.position);
  modelMatrix = modelMatrix * glm::toMat4(object.transformation.rotation);
  
  glm::vec3 scaling = object.transformation.scale * totalScale;  // having trouble with doing the scaling here so putting out of band.   Anyone in the ether please help if more elegant. 
  glm::mat4 scaledModelMatrix = modelMatrix * glm::scale(glm::mat4(1.f), scaling);

  onObject(id, scaledModelMatrix, model);

  for (short id: objectH.children){
    traverseScene(id, scene.idToGameObjectsH.at(id), scene, modelMatrix, scaling, onObject);
  }
}

struct traversalData {
  short id;
  glm::mat4 modelMatrix;
  glm::mat4 parentMatrix;
};
void traverseScene(Scene& scene, std::function<void(short, glm::mat4, glm::mat4, bool)> onObject){
  std::vector<traversalData> datum;

  for (unsigned int i = 0; i < scene.rootGameObjectsH.size(); i++){
    short id = scene.rootGameObjectsH.at(i);
    traverseScene(id, scene.idToGameObjectsH.at(id), scene, glm::mat4(1.f), glm::vec3(1.f, 1.f, 1.f), [&datum](short id, glm::mat4 modelMatrix, glm::mat4 parentMatrix) -> void {
      datum.push_back(traversalData{
        .id = id,
        .modelMatrix = modelMatrix,
        .parentMatrix = parentMatrix,
      });
    });
  }

  for (auto layer : scene.layers){      // @TODO could organize this before to not require pass on each frame
    for (auto data : datum){
      if (scene.idToGameObjects.at(data.id).layer == layer.name){
        onObject(data.id, data.modelMatrix, data.parentMatrix, layer.orthographic);
      }
    }  
  }
}

std::vector<short> getIdsInGroup(Scene& scene, short groupId){
  std::vector<short> ids;
  for (auto [_, gameobj] : scene.idToGameObjectsH){
    if (gameobj.groupId == groupId){
      ids.push_back(gameobj.id);
    }
  }
  return ids;
}