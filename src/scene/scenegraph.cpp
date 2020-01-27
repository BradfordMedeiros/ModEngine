#include "./scenegraph.h"

GameObject getGameObject(glm::vec3 position, std::string name, short id){
  auto physicsOptions = physicsOpts {
    .enabled = false,
    .isStatic = true,
    .hasCollisions = true,
    .shape = BOX,
  };
  GameObject gameObject = {
    .id = id,
    .name = name,
    .position = position,
    .scale = glm::vec3(1.0f, 1.0f, 1.0f),
    .rotation = glm::quat(0, 0, 0, 1.0f),
    .physicsOptions = physicsOptions,
  };
  return gameObject;
}

void addObjectToScene(Scene& scene, glm::vec3 position, std::string name, short id, short parentId){
  auto gameobjectObj = getGameObject(position, name, id);

  auto gameobjectH = GameObjectH {
    .id = gameobjectObj.id,
    .parentId = parentId,
  };

  scene.idToGameObjectsH[gameobjectObj.id] = gameobjectH;
  scene.idToGameObjects[gameobjectObj.id] = gameobjectObj;
  scene.nameToId[name] = gameobjectObj.id;
}

Scene createSceneFromTokens(
  std::vector<Token> tokens,  
  std::function<void(short, std::string, std::map<std::string, std::string> additionalFields)> addObject, 
  std::vector<Field> fields,
  short (*getNewObjectId)()
){
  Scene scene;

  std::map<std::string, SerializationObject>  serialObjs = deserializeScene(tokens, fields);
  for (auto [_, serialObj] : serialObjs){
    short id = getNewObjectId();
    addObjectToScene(scene, glm::vec3(1.f, 1.f, 1.f), serialObj.name, id, -1);
    scene.idToGameObjects.at(id).position = serialObj.position;
    scene.idToGameObjects.at(id).scale = serialObj.scale;
    scene.idToGameObjects.at(id).rotation = serialObj.rotation;
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

  for (auto [_, serialObj] : serialObjs){
    addObject(scene.nameToId.at(serialObj.name), serialObj.type, serialObj.additionalFields);
  }

  return scene;
}


// @todo this parsing is sloppy and buggy... obviously need to harden this..
Scene deserializeScene(
  std::string content,  
  std::function<void(short, std::string, std::map<std::string, std::string> additionalFields)> addObject, 
  std::vector<Field> fields,  
  short (*getNewObjectId)()
){
  std::cout << "INFO: Deserialization: " << std::endl;
  return createSceneFromTokens(getTokens(content), addObject, fields, getNewObjectId);
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

    sceneData = sceneData + gameobjectName + ":position:" + serializeVec(gameobject.position) + "\n";
    sceneData = sceneData + gameobjectName + ":scale:" + serializeVec(gameobject.scale) + "\n";
    sceneData = sceneData + gameobjectName + ":rotation:" + serializeRotation(gameobject.rotation) + "\n";

    for (auto additionalFields : getAdditionalFields(id)){
      sceneData = sceneData + gameobjectName + ":" + additionalFields.first + ":" + additionalFields.second + "\n";
    }

    if (parentName != ""){
      sceneData =  sceneData + gameobjectName + ":parent:" + parentName + "\n";
    }
  }
  return sceneData;
}

/*void addObjectToScene(Scene& scene, std::string name, std::string mesh, glm::vec3 position, short (*getNewObjectId)(), std::function<void(short, std::string, std::map<std::string, std::string> additionalFields)> addObject){
  addObjectToScene(scene, position, name, getNewObjectId(), -1);
  short objectId = scene.nameToId[name];
  std::map<std::string, std::string> additionalFields;
  addObject(objectId, "default", additionalFields);
  scene.rootGameObjectsH.push_back(objectId);
}*/


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

void traverseScene(short id, GameObjectH objectH, Scene& scene, glm::mat4 model, std::function<void(short, glm::mat4)> onObject){
  GameObject object = scene.idToGameObjects[objectH.id];
  glm::mat4 modelMatrix = glm::translate(model, object.position);
  modelMatrix = modelMatrix * glm::toMat4(object.rotation);
  modelMatrix = glm::scale(modelMatrix, object.scale);

  onObject(id, modelMatrix);

  for (short id: objectH.children){
    traverseScene(id, scene.idToGameObjectsH.at(id), scene, modelMatrix, onObject);
  }
}

void traverseScene(Scene& scene, std::function<void(short, glm::mat4)> onObject){
  for (unsigned int i = 0; i < scene.rootGameObjectsH.size(); i++){
    short id = scene.rootGameObjectsH.at(i);
    traverseScene(id, scene.idToGameObjectsH.at(id), scene, glm::mat4(1.f), onObject);
  }  
}