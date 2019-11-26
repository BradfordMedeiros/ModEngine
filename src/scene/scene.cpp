#include "./scene.h"

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2){
  std::cout << "on object enter: (" << obj1 << " , " << obj2 << ")" << std::endl;
}
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2){
  std::cout << "on object leave: (" << obj1 << " , " << obj2 << ")" << std::endl;
}

void printPhysicsInfo(PhysicsInfo physicsInfo){
  BoundInfo info = physicsInfo.boundInfo;
  std::cout << "x: [ " << info.xMin << ", " << info.xMax << "]" << std::endl;
  std::cout << "y: [ " << info.yMin << ", " << info.yMax << "]" << std::endl;
  std::cout << "z: [ " << info.zMin << ", " << info.zMax << "]" << std::endl;
  std::cout << "pos: (" << physicsInfo.gameobject.position.x << ", " << physicsInfo.gameobject.position.y << ", " << physicsInfo.gameobject.position.z << ")" << std::endl;
  std::cout << "box: (" << physicsInfo.collisionInfo.x << ", " << physicsInfo.collisionInfo.y << ", " << physicsInfo.collisionInfo.z << ")" << std::endl;
}

void printVec3(std::string prefix, glm::vec3 vec){
  std::cout << prefix << vec.x << "," << vec.y << "," << vec.z << std::endl;
}
void dumpPhysicsInfo(std::vector<btRigidBody*>& rigidbodies){
  for (unsigned int i = 0; i < rigidbodies.size(); i++){
    printVec3("PHYSICS:" + std::to_string(i) + ":", getPosition(rigidbodies[i]));
  }
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

void addPhysicsBodies(physicsEnv physicsEnv, FullScene& fullscene, std::vector<btRigidBody*>& rigidbodies){
  for (auto const& [id, _] : fullscene.scene.idToGameObjects){
    auto physicsInfo = getPhysicsInfoForGameObject(fullscene, id);
    printPhysicsInfo(physicsInfo);

    if (id == 2){
      auto rigidPtr = addRigidBody(physicsEnv, physicsInfo.gameobject.position, 1, physicsInfo.gameobject.rotation, false, true);
      rigidbodies.push_back(rigidPtr);
      std::cout << "ADDING PTR: " << rigidPtr << std::endl;
    }else{
      bool isCollisionVolumeOnly = id == 3;
      auto rigidPtr = addRigidBody(
        physicsEnv, 
        physicsInfo.gameobject.position,
        physicsInfo.collisionInfo.x, physicsInfo.collisionInfo.y, physicsInfo.collisionInfo.z,
        physicsInfo.gameobject.rotation,
        id == 1 || isCollisionVolumeOnly,
        !isCollisionVolumeOnly
      );
      rigidbodies.push_back(rigidPtr);
      std::cout << "ADDING PTR: " << rigidPtr << std::endl;
    }
  }
}

FullScene deserializeFullScene(std::string content){
  std::map<std::string, Mesh> meshes;
  auto objectMapping = getObjectMapping();

  auto addObjectAndLoadMesh = [&meshes, &objectMapping](short id, std::string type, std::string field, std::string payload) -> void {
    addObject(id, type, field, payload, objectMapping, meshes, "./res/models/box/box.obj", [&meshes](std::string meshName) -> void {  // @todo this is duplicate with commented below
      meshes[meshName] = loadMesh(meshName, "./res/textures/default.jpg");
    });
  };

  Scene scene = deserializeScene(content, addObjectAndLoadMesh, fields);

  auto physicsEnvironment = initPhysics(onObjectEnter, onObjectLeave);

  FullScene fullscene = {
    .scene = scene,
    .meshes = meshes,
    .objectMapping = objectMapping,
    .physicsEnvironment = physicsEnvironment,
  };
  addPhysicsBodies(fullscene.physicsEnvironment, fullscene, fullscene.rigidbodies);

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
  setPosition(body, newPosition);
}

void physicsRotate(FullScene& fullscene, btRigidBody* body, float x, float y, float z, short index){
  glm::quat rotation = setFrontDelta(fullscene.scene.idToGameObjects[index].rotation, x, y, z, 5);
  fullscene.scene.idToGameObjects[index].rotation  = rotation;
  setRotation(body, rotation);
}

void physicsScale(FullScene& fullscene, btRigidBody* body, short index, float x, float y, float z){
  auto oldScale = fullscene.scene.idToGameObjects[index].scale;
  glm::vec3 newScale = glm::vec3(oldScale.x + x, oldScale.y + y, oldScale.z + z);
  fullscene.scene.idToGameObjects[index].scale = newScale;
  auto collisionInfo = getPhysicsInfoForGameObject(fullscene, index).collisionInfo;
  setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
}

void applyPhysicsTranslation(FullScene& scene, btRigidBody* body, short index, glm::vec3 position, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis){
  auto newPosition = applyTranslation(position, offsetX, offsetY, manipulatorAxis);
  scene.scene.idToGameObjects[index].position = newPosition;
  setPosition(body, newPosition);
}

void applyPhysicsRotation(FullScene& scene, btRigidBody* body, short index, glm::quat currentOrientation, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis){
  auto newRotation = applyRotation(currentOrientation, offsetX, offsetY, manipulatorAxis);
  scene.scene.idToGameObjects[index].rotation = newRotation;
  setRotation(body, newRotation);
}

void applyPhysicsScaling(FullScene& scene, btRigidBody* body, short index, glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis){
  auto newScale = applyScaling(position, initialScale, lastX, lastY, offsetX, offsetY, manipulatorAxis);
  scene.scene.idToGameObjects[index].scale = newScale;
  auto collisionInfo = getPhysicsInfoForGameObject(scene, index).collisionInfo;
  setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
}

void updatePhysicsPositions(Scene& scene, std::vector<btRigidBody*>& rigidbodies){
  for (unsigned int i = 0; i < rigidbodies.size(); i++){
    scene.idToGameObjects[i].rotation = getRotation(rigidbodies[i]);
    scene.idToGameObjects[i].position = getPosition(rigidbodies[i]);
    // @note -> for consistency I would get the scale as well, but physics won't be rescaling so pointless right?
  }
}

void onPhysicsFrame(FullScene& fullscene, bool dumpPhysics){
  if (dumpPhysics){
    dumpPhysicsInfo(fullscene.rigidbodies);
  }
  stepPhysicsSimulation(fullscene.physicsEnvironment, 1.f / 60.f);
  updatePhysicsPositions(fullscene.scene, fullscene.rigidbodies);    
}