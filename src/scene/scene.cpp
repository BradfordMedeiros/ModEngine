#include "./scene.h"

FullScene deserializeFullScene(std::string content){
  std::map<std::string, Mesh> meshes;
  auto objectMapping = getObjectMapping();

  auto addObjectAndLoadMesh = [&meshes, &objectMapping](short id, std::string type, std::string field, std::string payload) -> void {
    addObject(id, type, field, payload, objectMapping, meshes, "./res/models/box/box.obj", [&meshes](std::string meshName) -> void {  // @todo this is duplicate with commented below
      meshes[meshName] = loadMesh(meshName, "./res/textures/default.jpg");
    });
  };

  Scene scene = deserializeScene(content, addObjectAndLoadMesh, fields);
  FullScene fullscene = {
    .scene = scene,
    .meshes = meshes,
    .objectMapping = objectMapping,
  };
  return fullscene;
}

std::string serializeFullScene(Scene& scene, std::map<short, GameObjectObj> objectMapping){
  return serializeScene(scene, [&objectMapping](short objectId)-> std::vector<std::pair<std::string, std::string>> {
    return getAdditionalFields(objectId, objectMapping);
  });
}

void addObjectToFullScene(FullScene& fullscene, std::string name, std::string meshName, glm::vec3 pos){
  addObjectToScene(fullscene.scene, name, meshName, pos, [&fullscene](short id, std::string type, std::string field, std::string payload) -> void {
    addObject(id, type, field, payload, fullscene.objectMapping, fullscene.meshes, "./res/models/box/box.obj", [&fullscene](std::string meshName) -> void { // @todo dup with commented above
      fullscene.meshes[meshName] = loadMesh(meshName, "./res/textures/default.jpg");
    });
  });
}

glm::vec3 getScaledCollisionBounds(BoundInfo boundInfo, glm::vec3 scale){
  float x = scale.x * (boundInfo.xMax - boundInfo.xMin);
  float y = scale.y * (boundInfo.yMax - boundInfo.yMin);
  float z = scale.z * (boundInfo.zMax - boundInfo.zMin);
  return glm::vec3(x, y, z);
}

PhysicsInfo getPhysicsInfoForGameObject(FullScene& fullscene, short index){
  GameObject obj = fullscene.scene.idToGameObjects[index];
  auto gameObjV = fullscene.objectMapping[index]; 

  BoundInfo boundInfo = {
    .xMin = -1, 
    .xMax = 1,
    .yMin = -1, 
    .yMax = 1,
    .zMin = -1,
    .zMax = 1,
  };

  auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); 

  if (meshObj != NULL){
    boundInfo = meshObj->mesh.boundInfo;
  }

  PhysicsInfo info = {
    .boundInfo = boundInfo,
    .gameobject = obj,
    .collisionInfo =  getScaledCollisionBounds(boundInfo, obj.scale),
  };

  return info;
}

void physicsTranslate(FullScene& fullscene, btRigidBody* body, float x, float y, float z, bool moveRelativeEnabled, short index){
  const int SPEED = 5;
  auto offset = glm::vec3(x * SPEED, y * SPEED, z * SPEED);

  glm::vec3 newPosition;
  if (moveRelativeEnabled){
    auto oldGameObject = fullscene.scene.idToGameObjects[index];
    newPosition= moveRelative(oldGameObject.position, oldGameObject.rotation, offset);
  }else{
    newPosition = move(fullscene.scene.idToGameObjects[index].position, offset);   
  }
  fullscene.scene.idToGameObjects[index].position = newPosition;
  setPosition(body, newPosition.x, newPosition.y, newPosition.z);
}

void physicsRotate(FullScene& fullscene, btRigidBody* body, float x, float y, float z, short index){
  glm::quat rotation = setFrontDelta(fullscene.scene.idToGameObjects[index].rotation, x, y, z, 15);
  fullscene.scene.idToGameObjects[index].rotation  = rotation;
  setRotation(body, rotation);
}