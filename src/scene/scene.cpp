#include "./scene.h"

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
void dumpPhysicsInfo(std::map<short, btRigidBody*>& rigidbodys){
  for (auto [i, rigidBody]: rigidbodys){
    printVec3("PHYSICS:" + std::to_string(i) + ":", getPosition(rigidBody));
  }
}

glm::vec3 getScaledCollisionBounds(BoundInfo boundInfo, glm::vec3 scale){
  float x = scale.x * (boundInfo.xMax - boundInfo.xMin);
  float y = scale.y * (boundInfo.yMax - boundInfo.yMin);
  float z = scale.z * (boundInfo.zMax - boundInfo.zMin);
  return glm::vec3(x, y, z);
}

PhysicsInfo getPhysicsInfoForGameObject(World& world, FullScene& fullscene, short index){
  GameObject obj = fullscene.scene.idToGameObjects[index];
  auto gameObjV = world.objectMapping[index]; 

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

void addPhysicsBody(World& world, FullScene& fullscene, short id){
  auto obj = fullscene.scene.idToGameObjects[id];
  auto physicsInfo = getPhysicsInfoForGameObject(world, fullscene, id);
  printPhysicsInfo(physicsInfo);

  auto physicsOptions = obj.physicsOptions;
  btRigidBody* rigidBody;
  if (physicsOptions.shape == BOX){
    std::cout << "INFO: PHYSICS: ADDING BOX RIGID BODY" << std::endl;
    rigidBody = addRigidBody(
      world.physicsEnvironment, 
      physicsInfo.gameobject.position, 
      physicsInfo.collisionInfo.x, physicsInfo.collisionInfo.y, physicsInfo.collisionInfo.z,
      physicsInfo.gameobject.rotation,
      physicsOptions.isStatic,
      physicsOptions.hasCollisions
    );
  }else if (physicsOptions.shape == SPHERE){
    std::cout << "INFO: PHYSICS: ADDING SPHERE RIGID BODY" << std::endl;
    rigidBody = addRigidBody(
      world.physicsEnvironment, 
      physicsInfo.gameobject.position,
      maxvalue(physicsInfo.collisionInfo.x, physicsInfo.collisionInfo.y, physicsInfo.collisionInfo.z),                             
      physicsInfo.gameobject.rotation,
      physicsOptions.isStatic,
      physicsOptions.hasCollisions
    );
  }else{
    std::cerr << "CRITICAL ERROR: default case for physics shape type" << std::endl;
      exit(1);
  }

  world.rigidbodys[id] = rigidBody;
}

// @todo - this currently adds a physics body for every single object 
// no good, this should only add if enabled. 
void addPhysicsBodies(World& world, FullScene& fullscene){
  for (auto &[id, _] : fullscene.scene.idToGameObjects){
    addPhysicsBody(world, fullscene, id);
  }
}

short getIdForCollisionObject(World& world, const btCollisionObject* body){
  for (auto const&[id, rigidbody] : world.rigidbodys){
    if (rigidbody == body){
      return id;
    }
  }
  return -1;
}

World createWorld(collisionPairFn onObjectEnter, collisionPairFn onObjectLeave){
  auto objectMapping = getObjectMapping();

  World world = {
    .physicsEnvironment = initPhysics(onObjectEnter, onObjectLeave),
    .objectMapping = objectMapping,
  };
  return world;
}

////////////////  need unique values for these, there are probably better ways to pick these (maybe based on value of ptr?)
static short id = -1;
short getObjectId(){
  id++;
  return id;
}

static short sceneId = -1;
short getSceneId(){
  sceneId++;
  return sceneId;
}
//////////////////// 

FullScene deserializeFullScene(World& world, short sceneId, std::string content){
  auto addObjectAndLoadMesh = [&world, &sceneId](short id, std::string type, std::string field, std::string payload) -> void {
    addObject(id, type, field, payload, world.objectMapping, world.meshes, "./res/models/box/box.obj", [&world, &sceneId, &id](std::string meshName) -> void {  // @todo this is duplicate with commented below
      world.meshes[meshName] = loadMesh(meshName, "./res/textures/default.jpg");     // @todo protect against loading this mesh many times. 
      world.idToScene[id] = sceneId;
    });
  };

  Scene scene = deserializeScene(content, addObjectAndLoadMesh, fields, getObjectId);

  FullScene fullscene = {
    .scene = scene,
  };

  return fullscene;
}

std::string serializeFullScene(Scene& scene, std::map<short, GameObjectObj> objectMapping){
  return serializeScene(scene, [&objectMapping](short objectId)-> std::vector<std::pair<std::string, std::string>> {
    return getAdditionalFields(objectId, objectMapping);
  });
}

void addSceneToWorld(World& world, std::string sceneFile){
  auto sceneId = getSceneId();
  world.scenes[sceneId] = deserializeFullScene(world, sceneId, loadFile(sceneFile));
  addPhysicsBodies(world, world.scenes[sceneId]);
}
void removeSceneFromWorld(World& world, int sceneId){
  auto fullscene = world.scenes[sceneId];
  for (auto objectId : listObjInScene(fullscene.scene)){
    auto rigidBody = world.rigidbodys[objectId];
    rmRigidBody(world.physicsEnvironment, rigidBody);
    world.objectMapping.erase(objectId);
    world.idToScene.erase(objectId);
  }

  // remove scene from world.scenes
  // remove free meshes (no way to tell currently if free -> need counting probably) from meshes
}

void addObjectToFullScene(World& world, short sceneId, std::string name, std::string meshName, glm::vec3 pos){
  // @todo dup with commented above      world.meshes[meshName] = loadMesh(meshName, "./res/textures/default.jpg");      // @todo protect against loading
  addObjectToScene(world.scenes[sceneId].scene, name, meshName, pos, getObjectId, [&world, &sceneId](short id, std::string type, std::string field, std::string payload) -> void {
    addObject(id, type, field, payload, world.objectMapping, world.meshes, "./res/models/box/box.obj", [&world, &sceneId, &id](std::string meshName) -> void { 
      world.meshes[meshName] = loadMesh(meshName, "./res/textures/default.jpg");      // @todo protect against loading
      world.idToScene[id] = sceneId;
      addPhysicsBody(world, world.scenes[sceneId], id);  // this is diffedrent than in deserialize only because it uses it to find the gameobject, not b/c good reasons
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
void physicsTranslateSet(FullScene& fullscene, btRigidBody* body, glm::vec3 pos, short index){
  fullscene.scene.idToGameObjects[index].position = pos;
  setPosition(body, pos);
}

void physicsRotate(FullScene& fullscene, btRigidBody* body, float x, float y, float z, short index){
  glm::quat rotation = setFrontDelta(fullscene.scene.idToGameObjects[index].rotation, x, y, z, 5);
  fullscene.scene.idToGameObjects[index].rotation  = rotation;
  setRotation(body, rotation);
}
void physicsRotateSet(FullScene& fullscene, btRigidBody* body, glm::quat rotation, short index){
  fullscene.scene.idToGameObjects[index].rotation = rotation;
  setRotation(body, rotation);
}

void physicsScale(World& world, FullScene& fullscene, btRigidBody* body, short index, float x, float y, float z){
  auto oldScale = fullscene.scene.idToGameObjects[index].scale;
  glm::vec3 newScale = glm::vec3(oldScale.x + x, oldScale.y + y, oldScale.z + z);
  fullscene.scene.idToGameObjects[index].scale = newScale;
  auto collisionInfo = getPhysicsInfoForGameObject(world, fullscene, index).collisionInfo;
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

void applyPhysicsScaling(World& world, FullScene& scene, btRigidBody* body, short index, glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis){
  auto newScale = applyScaling(position, initialScale, lastX, lastY, offsetX, offsetY, manipulatorAxis);
  scene.scene.idToGameObjects[index].scale = newScale;
  auto collisionInfo = getPhysicsInfoForGameObject(world, scene, index).collisionInfo;
  setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
}

void updatePhysicsPositions(World& world, std::map<short, btRigidBody*>& rigidbodys){
  for (auto [i, rigidBody]: rigidbodys){
    auto sceneId = world.idToScene[i];
    world.scenes[sceneId].scene.idToGameObjects[i].rotation = getRotation(rigidBody);
    world.scenes[sceneId].scene.idToGameObjects[i].position = getPosition(rigidBody);
    // @note -> for consistency I would get the scale as well, but physics won't be rescaling so pointless right?
  }

}

void onPhysicsFrame(World& world, float timestep, bool dumpPhysics){
  if (dumpPhysics){
    dumpPhysicsInfo(world.rigidbodys);
  }
  stepPhysicsSimulation(world.physicsEnvironment, timestep);
  updatePhysicsPositions(world, world.rigidbodys);    
}