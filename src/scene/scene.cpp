#include "./scene.h"

void printPhysicsInfo(PhysicsInfo physicsInfo){
  BoundInfo info = physicsInfo.boundInfo;
  std::cout << "x: [ " << info.xMin << ", " << info.xMax << "]" << std::endl;
  std::cout << "y: [ " << info.yMin << ", " << info.yMax << "]" << std::endl;
  std::cout << "z: [ " << info.zMin << ", " << info.zMax << "]" << std::endl;
  std::cout << "pos: (" << physicsInfo.gameobject.transformation.position.x << ", " << physicsInfo.gameobject.transformation.position.y << ", " << physicsInfo.gameobject.transformation.position.z << ")" << std::endl;
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

BoundInfo getMaxUnionBoundingInfo(std::vector<Mesh> boundings){
  assert(boundings.size() > 0);   // temporary, obviously this should actually calculate it, not just return the first
  return boundings.at(0).boundInfo;
}


PhysicsInfo getPhysicsInfoForGameObject(World& world, FullScene& fullscene, short index){
  GameObject obj = fullscene.scene.idToGameObjects.at(index);
  auto gameObjV = world.objectMapping.at(index); 

  BoundInfo boundInfo = {
    .xMin = -1, 
    .xMax = 1,
    .yMin = -1, 
    .yMax = 1,
    .zMin = -1,
    .zMax = 1,
    .isNotCentered = false,
  };

  auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); 
  if (meshObj != NULL){
    std::vector<BoundInfo> info;
    boundInfo = getMaxUnionBoundingInfo(meshObj -> meshesToRender);
  }

  auto voxelObj = std::get_if<GameObjectVoxel>(&gameObjV);
  if (voxelObj != NULL){
    boundInfo = voxelObj -> voxel.mesh.boundInfo;
  }


  PhysicsInfo info = {
    .boundInfo = boundInfo,
    .gameobject = obj,
    .collisionInfo =  obj.transformation.scale
  };

  return info;
}

void addPhysicsBody(World& world, FullScene& fullscene, short id, glm::vec3 initialScale){
  auto obj = fullscene.scene.idToGameObjects.at(id);
  auto physicsInfo = getPhysicsInfoForGameObject(world, fullscene, id);

  auto physicsOptions = obj.physicsOptions;
  btRigidBody* rigidBody = NULL;

  GameObjectObj& toRender = world.objectMapping.at(id);
  bool isVoxelObj = std::get_if<GameObjectVoxel>(&toRender) != NULL;
  if (physicsOptions.shape == BOX || (!isVoxelObj && physicsOptions.shape == AUTOSHAPE)){
    std::cout << "INFO: PHYSICS: ADDING BOX RIGID BODY (" << id << ") -- " << (physicsInfo.boundInfo.isNotCentered ? "notcentered" : "centered") << std::endl;
    rigidBody = addRigidBody(
      world.physicsEnvironment, 
      physicsInfo.gameobject.transformation.position, 
      (physicsInfo.boundInfo.xMax - physicsInfo.boundInfo.xMin), (physicsInfo.boundInfo.yMax - physicsInfo.boundInfo.yMin) , (physicsInfo.boundInfo.zMax - physicsInfo.boundInfo.zMin),
      physicsInfo.gameobject.transformation.rotation,
      physicsOptions.isStatic,
      physicsOptions.hasCollisions,
      !physicsInfo.boundInfo.isNotCentered,
      physicsInfo.collisionInfo
    );
  }else if (physicsOptions.shape == SPHERE){
    std::cout << "INFO: PHYSICS: ADDING SPHERE RIGID BODY" << std::endl;
    rigidBody = addRigidBody(
      world.physicsEnvironment, 
      physicsInfo.gameobject.transformation.position,
      maxvalue((physicsInfo.boundInfo.xMax - physicsInfo.boundInfo.xMin), (physicsInfo.boundInfo.yMax - physicsInfo.boundInfo.yMin) , (physicsInfo.boundInfo.zMax - physicsInfo.boundInfo.zMin)),                             
      physicsInfo.gameobject.transformation.rotation,
      physicsOptions.isStatic,
      physicsOptions.hasCollisions,
      physicsInfo.collisionInfo
    );
  }else if (physicsOptions.shape == AUTOSHAPE && isVoxelObj){
    std::cout << "INFO: PHYSICS: ADDING AUTOSHAPE VOXEL RIGID BODY" << std::endl;
    rigidBody = addRigidBody(
      world.physicsEnvironment, 
      physicsInfo.gameobject.transformation.position,
      physicsInfo.gameobject.transformation.rotation,
      getVoxelBodies(std::get_if<GameObjectVoxel>(&toRender) -> voxel),
      physicsOptions.isStatic,
      physicsOptions.hasCollisions,
      initialScale
    );
  }

  assert(rigidBody != NULL);
  world.rigidbodys[id] = rigidBody;
}
void rmRigidBody(World& world, short id){
  auto rigidBodyPtr = world.rigidbodys.at(id);
  assert(rigidBodyPtr != NULL);
  rmRigidBody(world.physicsEnvironment, rigidBodyPtr);
  world.rigidbodys.erase(id);
}

void updatePhysicsBody(World& world, FullScene& scene, short id){
  auto rigidBody = world.rigidbodys.at(id);
  glm::vec3 oldScale = getScale(rigidBody);
  rmRigidBody(world, id);
  addPhysicsBody(world, world.scenes.at(world.idToScene.at(id)), id, oldScale);
}

// @todo - this currently adds a physics body for every single object, probably should default to this not being the case (I think)
void addPhysicsBodies(World& world, FullScene& fullscene){
  for (auto &[id, _] : fullscene.scene.idToGameObjects){
    addPhysicsBody(world, fullscene, id, glm::vec3(1.f, 1.f, 1.f));
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

World createWorld(collisionPairFn onObjectEnter, collisionPairFn onObjectLeave, btIDebugDraw* debugDrawer){
  auto objectMapping = getObjectMapping();
  World world = {
    .physicsEnvironment = initPhysics(onObjectEnter, onObjectLeave, debugDrawer),
    .objectMapping = objectMapping,
  };
  ModelData data = loadModel("./res/models/ui/node.obj");
  assert(data.meshIdToMeshData.size() ==  1);
  auto meshData = data.meshIdToMeshData.begin() -> second;
  world.meshes["./res/models/ui/node.obj"] =  loadMesh("./res/textures/default.jpg", meshData);     
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

void addObjects(World& world, Scene& scene, std::map<std::string, SerializationObject>& serialObjs){
  for (auto [_, serialObj] : serialObjs){
    auto id =  scene.nameToId.at(serialObj.name);
    auto type = serialObj.type;
    auto additionalFields = serialObj.additionalFields;

    world.idToScene[id] = sceneId;
    auto localSceneId = sceneId;

    addObject(id, type, additionalFields, world.objectMapping, world.meshes, "./res/models/box/box.obj", 
      [&world, &scene, id](std::string meshName) -> void {  // @todo this is duplicate with commented below
        std::cout << "********** ensure: " << meshName << std::endl;
        if (world.meshes.find(meshName) == world.meshes.end()){
          std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$ ensure mesh loaded called for " << meshName << std::endl;
          ModelData data = loadModel(meshName);        

          auto meshesForId = data.nodeToMeshId.at(0);
          std::cout << "meshes for id size: " << meshesForId.size() << std::endl;

          bool hasMesh = meshesForId.size() > 0;
          if (hasMesh){
            auto meshId = meshesForId.at(0);
            MeshData model =  data.meshIdToMeshData.at(meshId);
            world.meshes[meshName] = loadMesh("./res/textures/default.jpg", model);     // @todo protect against loading this mesh many times. 
          }else{
            std::cout << "loading default node mesh for: " << meshName << std::endl;
            world.meshes[meshName] = world.meshes.at("./res/models/ui/node.obj");    // temporary this shouldn't load a unique mesh.  Really this just needs to say "no mesh"
          }
          
          for (auto [meshId, meshData] : data.meshIdToMeshData){
            auto meshRef = meshName + "::" + std::to_string(meshId);
            std::cout << "loading mesh: " << meshRef << std::endl;
            world.meshes[meshRef] = loadMesh("./res/textures/default.jpg", meshData);     // @todo protect against loading this mesh many times. 
          } 

          std::map<short, std::map<std::string, std::string>> additionalFieldsMap;
          for (auto [nodeId, _] : data.nodeTransform){
            std::map<std::string, std::string> emptyFields;
            additionalFieldsMap[nodeId] = emptyFields;
          }

          std::cout << "about to set mesh ref" << std::endl;
          for (auto [nodeId, meshListIds] : data.nodeToMeshId){
            if (meshListIds.size() == 1){
              auto meshRef = meshName + "::" + std::to_string(meshListIds.at(0));
              additionalFieldsMap.at(nodeId)["mesh"] = meshRef;
            }else if (meshListIds.size() > 1){
              std::vector<std::string> meshRefNames;
              for (auto id : meshListIds){
                auto meshRef = meshName + "::" + std::to_string(id);
                meshRefNames.push_back(meshRef);
              }
              additionalFieldsMap.at(nodeId)["meshes"] = join(meshRefNames, ',');
            }else{
              std::cout << "WARNING: node: " << data.names.at(nodeId) << " has no meshes" << std::endl;
            }
          }

          auto newSerialObjs = addSubsceneToRoot(
            scene, 
            id,
            0,
            data.childToParent, 
            data.nodeTransform, 
            data.names, 
            additionalFieldsMap,
            getObjectId
          );
          for (auto [serialObjName, serialObj] : newSerialObjs){
            if (serialObj.additionalFields.find("mesh") == serialObj.additionalFields.end()){
              std::cout << "additionalmesh name is not present" << std::endl;
            }else{
              std::cout << "additionalmesh name is: " << serialObj.additionalFields.at("mesh") << std::endl;
            }
          }
          addObjects(world, scene, newSerialObjs);

        }
      }, 
      [&world, localSceneId, id]() -> void {
        updatePhysicsBody(world, world.scenes.at(sceneId), id);
      }
    );
  }
}
FullScene deserializeFullScene(World& world, short sceneId, std::string content){
  SceneDeserialization deserializedScene = deserializeScene(content, fields, getObjectId);
  addObjects(world, deserializedScene.scene, deserializedScene.serialObjs);
  FullScene fullscene = {
    .scene = deserializedScene.scene,
  };

  return fullscene;
}

std::string serializeFullScene(Scene& scene, std::map<short, GameObjectObj> objectMapping){
  return serializeScene(scene, [&objectMapping](short objectId)-> std::vector<std::pair<std::string, std::string>> {
    return getAdditionalFields(objectId, objectMapping);
  });
}

short addSceneToWorld(World& world, std::string sceneFile){
  auto sceneId = getSceneId();
  world.scenes[sceneId] = deserializeFullScene(world, sceneId, loadFile(sceneFile));
  addPhysicsBodies(world, world.scenes.at(sceneId));
  return sceneId;
}
void removeSceneFromWorld(World& world, short sceneId){
  if (world.scenes.find(sceneId) == world.scenes.end()) {
    std::cout << "INFO: SCENE MANAGEMENT: tried to remove (" << sceneId << ") but it does not exist" << std::endl;
    return;   // @todo maybe better to throw error instead
  }

  auto fullscene = world.scenes.at(sceneId);
  for (auto objectId : listObjInScene(fullscene.scene)){
    auto rigidBody = world.rigidbodys.at(objectId);
    assert(rigidBody != NULL);
    rmRigidBody(world.physicsEnvironment, rigidBody);
    world.rigidbodys.erase(objectId);
    world.objectMapping.erase(objectId);
    world.idToScene.erase(objectId);

    // @TODO IMPORTANT : remove free meshes (no way to tell currently if free -> need counting probably) from meshes
    std::cout << "TODO: MESH MANAGEMENT HORRIBLE NEED TO REMOVE AND NOT BE DUMB ABOUT LOADING THEM" << std::endl;
  }
  world.scenes.erase(sceneId);
}

void addObjectToFullScene(World& world, short sceneId, std::string name, std::string meshName, glm::vec3 pos){
  // @todo dup with commented above      world.meshes[meshName] = loadMesh(meshName, "./res/textures/default.jpg");      // @todo protect against loading
  /*addObjectToScene(world.scenes[sceneId].scene, name, meshName, pos, getObjectId, [&world, &sceneId](short id, std::string type, std::string field, std::string payload) -> void {
    world.idToScene[id] = sceneId;
    addPhysicsBody(world, world.scenes.at(sceneId), id);  // this is different than in deserialize only because it uses it to find the gameobject, not b/c good reasons
    addObject(id, type, field, payload, world.objectMapping, world.meshes, "./res/models/box/box.obj", [&world](std::string meshName) -> void { 
      world.meshes[meshName] = loadMesh(meshName, "./res/textures/default.jpg");      // @todo protect against loading
    });
  });*/
}

void physicsTranslate(FullScene& fullscene, btRigidBody* body, float x, float y, float z, bool moveRelativeEnabled, short index){
  const int SPEED = 5;
  auto offset = glm::vec3(x * SPEED, y * SPEED, z * SPEED);

  glm::vec3 newPosition;
  if (moveRelativeEnabled){
    auto oldGameObject = fullscene.scene.idToGameObjects.at(index);
    newPosition= moveRelative(oldGameObject.transformation.position, oldGameObject.transformation.rotation, offset);
  }else{
    newPosition = move(fullscene.scene.idToGameObjects.at(index).transformation.position, offset);   
  }
  fullscene.scene.idToGameObjects.at(index).transformation.position = newPosition;
  setPosition(body, newPosition);
}
void physicsTranslateSet(FullScene& fullscene, btRigidBody* body, glm::vec3 pos, short index){
  fullscene.scene.idToGameObjects.at(index).transformation.position = pos;
  setPosition(body, pos);
}

void physicsRotate(FullScene& fullscene, btRigidBody* body, float x, float y, float z, short index){
  glm::quat rotation = setFrontDelta(fullscene.scene.idToGameObjects.at(index).transformation.rotation, x, y, z, 5);
  fullscene.scene.idToGameObjects.at(index).transformation.rotation  = rotation;
  setRotation(body, rotation);
}
void physicsRotateSet(FullScene& fullscene, btRigidBody* body, glm::quat rotation, short index){
  fullscene.scene.idToGameObjects.at(index).transformation.rotation = rotation;
  setRotation(body, rotation);
}

void physicsScale(World& world, FullScene& fullscene, btRigidBody* body, short index, float x, float y, float z){
  auto oldScale = fullscene.scene.idToGameObjects.at(index).transformation.scale;
  glm::vec3 newScale = glm::vec3(oldScale.x + x, oldScale.y + y, oldScale.z + z);
  fullscene.scene.idToGameObjects.at(index).transformation.scale = newScale;
  auto collisionInfo = getPhysicsInfoForGameObject(world, fullscene, index).collisionInfo;
  setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
}

void applyPhysicsTranslation(FullScene& scene, btRigidBody* body, short index, glm::vec3 position, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis){
  auto newPosition = applyTranslation(position, offsetX, offsetY, manipulatorAxis);
  scene.scene.idToGameObjects.at(index).transformation.position = newPosition;
  setPosition(body, newPosition);
}

void applyPhysicsRotation(FullScene& scene, btRigidBody* body, short index, glm::quat currentOrientation, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis){
  auto newRotation = applyRotation(currentOrientation, offsetX, offsetY, manipulatorAxis);
  scene.scene.idToGameObjects.at(index).transformation.rotation = newRotation;
  setRotation(body, newRotation);
}

void applyPhysicsScaling(World& world, FullScene& scene, btRigidBody* body, short index, glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis){
  auto newScale = applyScaling(position, initialScale, lastX, lastY, offsetX, offsetY, manipulatorAxis);
  scene.scene.idToGameObjects.at(index).transformation.scale = newScale;
  auto collisionInfo = getPhysicsInfoForGameObject(world, scene, index).collisionInfo;
  setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
}

void updatePhysicsPositions(World& world, std::map<short, btRigidBody*>& rigidbodys){
  for (auto [i, rigidBody]: rigidbodys){
    auto sceneId = world.idToScene.at(i);
    world.scenes.at(sceneId).scene.idToGameObjects.at(i).transformation.rotation = getRotation(rigidBody);
    world.scenes.at(sceneId).scene.idToGameObjects.at(i).transformation.position = getPosition(rigidBody);
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