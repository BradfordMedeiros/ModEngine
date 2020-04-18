#include "./scene.h"

void printPhysicsInfo(PhysicsInfo physicsInfo){
  BoundInfo info = physicsInfo.boundInfo;
  std::cout << "x: [ " << info.xMin << ", " << info.xMax << "]" << std::endl;
  std::cout << "y: [ " << info.yMin << ", " << info.yMax << "]" << std::endl;
  std::cout << "z: [ " << info.zMin << ", " << info.zMax << "]" << std::endl;
  std::cout << "pos: (" << physicsInfo.gameobject.transformation.position.x << ", " << physicsInfo.gameobject.transformation.position.y << ", " << physicsInfo.gameobject.transformation.position.z << ")" << std::endl;
  std::cout << "box: (" << physicsInfo.collisionInfo.x << ", " << physicsInfo.collisionInfo.y << ", " << physicsInfo.collisionInfo.z << ")" << std::endl;
}


void dumpPhysicsInfo(std::map<short, btRigidBody*>& rigidbodys){
  for (auto [i, rigidBody]: rigidbodys){
    std::cout << "PHYSICS:" << std::to_string(i) << ":" <<  print(getPosition(rigidBody));
  }
}

glm::vec3 getScaledCollisionBounds(BoundInfo boundInfo, glm::vec3 scale){
  float x = scale.x * (boundInfo.xMax - boundInfo.xMin);
  float y = scale.y * (boundInfo.yMax - boundInfo.yMin);
  float z = scale.z * (boundInfo.zMax - boundInfo.zMin);
  return glm::vec3(x, y, z);
}

BoundInfo getMaxUnionBoundingInfo(std::vector<Mesh> boundings){
  return boundings.at(0).boundInfo;
}

PhysicsInfo getPhysicsInfoForGameObject(World& world, Scene& scene, short index){
  GameObject obj = scene.idToGameObjects.at(index);
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
  if (meshObj != NULL && meshObj -> meshesToRender.size() > 0){
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

void addPhysicsBody(World& world, Scene& scene, short id, glm::vec3 initialScale){
  auto obj = scene.idToGameObjects.at(id);
  auto physicsInfo = getPhysicsInfoForGameObject(world, scene, id);

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

void updatePhysicsBody(World& world, Scene& scene, short id){
  auto rigidBody = world.rigidbodys.at(id);
  glm::vec3 oldScale = getScale(rigidBody);
  rmRigidBody(world, id);
  addPhysicsBody(world, world.scenes.at(world.idToScene.at(id)), id, oldScale);
}

// @todo - this currently adds a physics body for every single object, probably should default to this not being the case (I think)
void addPhysicsBodies(World& world, Scene& scene){
  for (auto &[id, _] : scene.idToGameObjects){
    addPhysicsBody(world, scene, id, glm::vec3(1.f, 1.f, 1.f));
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

void addMesh(World& world, std::string meshpath){
  ModelData data = loadModel(meshpath);
  assert(data.meshIdToMeshData.size() ==  1);
  auto meshData = data.meshIdToMeshData.begin() -> second;
  world.meshes[meshpath] =  loadMesh("./res/textures/default.jpg", meshData);     
  world.meshnameToBoneToParent[meshpath] = data.boneToParent;
  std::cout << "WARNING: add mesh does not load animations, bones for default meshes" << std::endl;
}

World createWorld(collisionPairFn onObjectEnter, collisionPairFn onObjectLeave, btIDebugDraw* debugDrawer){
  auto objectMapping = getObjectMapping();
  World world = {
    .physicsEnvironment = initPhysics(onObjectEnter, onObjectLeave, debugDrawer),
    .objectMapping = objectMapping,
  };

  // Default meshes that are silently loaded in the background
  addMesh(world, "./res/models/ui/node.obj");
  addMesh(world, "./res/models/boundingbox/boundingbox.obj");
  addMesh(world, "./res/models/cone/cone.obj");
  addMesh(world, "./res/models/camera/camera.dae");

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

std::map<short, std::map<std::string, std::string>> generateAdditionalFields(std::string meshName, ModelData& data){
  std::map<short, std::map<std::string, std::string>> additionalFieldsMap;
  for (auto [nodeId, _] : data.nodeTransform){
    std::map<std::string, std::string> emptyFields;
    additionalFieldsMap[nodeId] = emptyFields;
  }

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
    }
  }
  return additionalFieldsMap;
}

void addObjects(World& world, Scene& scene, std::map<std::string, SerializationObject>& serialObjs, bool shouldLoadModel, std::function<void(std::string)> loadClip){
  for (auto [_, serialObj] : serialObjs){
    auto id =  scene.nameToId.at(serialObj.name);
    auto type = serialObj.type;
    auto additionalFields = serialObj.additionalFields;

    world.idToScene[id] = sceneId;
    auto localSceneId = sceneId;

    addObject(id, type, additionalFields, world.objectMapping, world.meshes, "./res/models/ui/node.obj",  loadClip, 
      [&world, &scene, loadClip, id, shouldLoadModel](std::string meshName) -> bool {  // This is a weird function, it might be better considered "ensure model l"
        if (shouldLoadModel){
          ModelData data = loadModel(meshName); 
          world.animations[id] = data.animations;

          bool hasMesh = data.nodeToMeshId.at(0).size() > 0;     // this is 0 node because we just loaded a mesh, so by definition is root node

          for (auto [meshId, meshData] : data.meshIdToMeshData){
            auto meshPath = meshName + "::" + std::to_string(meshId);
            bool meshAlreadyLoaded = !(world.meshes.find(meshPath) == world.meshes.end());
            if (meshAlreadyLoaded){
              continue;
            }
            world.meshes[meshPath] = loadMesh("./res/textures/default.jpg", meshData);    
            world.meshnameToBoneToParent[meshPath] = data.boneToParent;  
          } 

          auto newSerialObjs = addSubsceneToRoot(
            scene, 
            id,
            0,
            data.childToParent, 
            data.nodeTransform, 
            data.names, 
            generateAdditionalFields(meshName, data),
            getObjectId
          );
          addObjects(world, scene, newSerialObjs, false, loadClip);
          return hasMesh;
        }
        return true;   // This is basically ensure model loaded so by definition this was already loaded. 
      }, 
      [&world, localSceneId, id]() -> void {
        updatePhysicsBody(world, world.scenes.at(sceneId), id);
      }
    );
  }
}

Scene deserializeFullScene(World& world, short sceneId, std::string content, std::function<void(std::string)> loadClip){
  SceneDeserialization deserializedScene = deserializeScene(content, fields, getObjectId);
  addObjects(world, deserializedScene.scene, deserializedScene.serialObjs, true, loadClip);
  return deserializedScene.scene;
}

std::string serializeFullScene(Scene& scene, std::map<short, GameObjectObj> objectMapping){
  return serializeScene(scene, [&objectMapping](short objectId)-> std::vector<std::pair<std::string, std::string>> {
    return getAdditionalFields(objectId, objectMapping);
  });
}

short addSceneToWorld(World& world, std::string sceneFile, std::function<void(std::string)> loadClip){
  auto sceneId = getSceneId();
  world.scenes[sceneId] = deserializeFullScene(world, sceneId, loadFile(sceneFile), loadClip);
  addPhysicsBodies(world, world.scenes.at(sceneId));
  return sceneId;
}
void removeSceneFromWorld(World& world, short sceneId){
  if (world.scenes.find(sceneId) == world.scenes.end()) {
    std::cout << "INFO: SCENE MANAGEMENT: tried to remove (" << sceneId << ") but it does not exist" << std::endl;
    return;   // @todo maybe better to throw error instead
  }

  auto scene = world.scenes.at(sceneId);
  for (auto objectId : listObjInScene(scene)){
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

void physicsTranslate(Scene& scene, btRigidBody* body, float x, float y, float z, bool moveRelativeEnabled, short index){
  const int SPEED = 5;
  auto offset = glm::vec3(x * SPEED, y * SPEED, z * SPEED);

  glm::vec3 newPosition;
  if (moveRelativeEnabled){
    auto oldGameObject = scene.idToGameObjects.at(index);
    newPosition= moveRelative(oldGameObject.transformation.position, oldGameObject.transformation.rotation, offset, false);
  }else{
    newPosition = move(scene.idToGameObjects.at(index).transformation.position, offset);   
  }
  scene.idToGameObjects.at(index).transformation.position = newPosition;
  setPosition(body, newPosition);
}
void physicsTranslateSet(Scene& scene, btRigidBody* body, glm::vec3 pos, short index){
  scene.idToGameObjects.at(index).transformation.position = pos;
  setPosition(body, pos);
}

void physicsRotate(Scene& scene, btRigidBody* body, float x, float y, float z, short index){
  glm::quat rotation = setFrontDelta(scene.idToGameObjects.at(index).transformation.rotation, x, y, z, 5);
  scene.idToGameObjects.at(index).transformation.rotation  = rotation;
  setRotation(body, rotation);
}
void physicsRotateSet(Scene& scene, btRigidBody* body, glm::quat rotation, short index){
  scene.idToGameObjects.at(index).transformation.rotation = rotation;
  setRotation(body, rotation);
}

void physicsScale(World& world, Scene& scene, btRigidBody* body, short index, float x, float y, float z){
  auto oldScale = scene.idToGameObjects.at(index).transformation.scale;
  glm::vec3 newScale = glm::vec3(oldScale.x + x, oldScale.y + y, oldScale.z + z);
  scene.idToGameObjects.at(index).transformation.scale = newScale;
  auto collisionInfo = getPhysicsInfoForGameObject(world, scene, index).collisionInfo;
  setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
}

void applyPhysicsTranslation(Scene& scene, btRigidBody* body, short index, glm::vec3 position, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis){
  auto newPosition = applyTranslation(position, offsetX, offsetY, manipulatorAxis);
  scene.idToGameObjects.at(index).transformation.position = newPosition;
  setPosition(body, newPosition);
}

void applyPhysicsRotation(Scene& scene, btRigidBody* body, short index, glm::quat currentOrientation, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis){
  auto newRotation = applyRotation(currentOrientation, offsetX, offsetY, manipulatorAxis);
  scene.idToGameObjects.at(index).transformation.rotation = newRotation;
  setRotation(body, newRotation);
}

void applyPhysicsScaling(World& world, Scene& scene, btRigidBody* body, short index, glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis){
  auto newScale = applyScaling(position, initialScale, lastX, lastY, offsetX, offsetY, manipulatorAxis);
  scene.idToGameObjects.at(index).transformation.scale = newScale;
  auto collisionInfo = getPhysicsInfoForGameObject(world, scene, index).collisionInfo;
  setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
}

void updatePhysicsPositions(World& world, std::map<short, btRigidBody*>& rigidbodys){
  for (auto [i, rigidBody]: rigidbodys){
    auto sceneId = world.idToScene.at(i);
    world.scenes.at(sceneId).idToGameObjects.at(i).transformation.rotation = getRotation(rigidBody);
    world.scenes.at(sceneId).idToGameObjects.at(i).transformation.position = getPosition(rigidBody);
    // @note -> for consistency I would get the scale as well, but physics won't be rescaling so pointless right?
  }
}


void enforceLookAt(World& world){
 for (auto &[_, scene] : world.scenes){
    for (auto &[id, gameobj] : scene.idToGameObjects){
      std::string lookAt = gameobj.lookat;                          // @TODO this id could be calculated on loading the scene to save extra lookups
      if (lookAt == "" || lookAt == gameobj.name){
        continue;
      }
      if(scene.nameToId.find(lookAt) != scene.nameToId.end()){
        short lookatId = scene.nameToId.at(lookAt);
        glm::vec3 fromPos = gameobj.transformation.position;
        glm::vec3 targetPosition = scene.idToGameObjects.at(lookatId).transformation.position;
        // @TODO consider extracting a better up direction from current orientation
        // https://stackoverflow.com/questions/18151845/converting-glmlookat-matrix-to-quaternion-and-back/29992778
        gameobj.transformation.rotation  = glm::conjugate(glm::quat_cast(glm::lookAt(fromPos, targetPosition, glm::vec3(0, 1, 0))));
      }
    }
  }
}

void onWorldFrame(World& world, float timestep, bool enablePhysics, bool dumpPhysics){
  if (enablePhysics){
    if (dumpPhysics){
      dumpPhysicsInfo(world.rigidbodys);
    }
    stepPhysicsSimulation(world.physicsEnvironment, timestep);
    updatePhysicsPositions(world, world.rigidbodys);     
  }
  enforceLookAt(world);
}

NameAndMesh getMeshesForGroupId(World& world, short groupId){
  std::vector<std::reference_wrapper<std::string>> meshNames;
  std::vector<std::reference_wrapper<Mesh>> meshes;
  NameAndMesh nameAndMeshes = {
    .meshNames = meshNames,
    .meshes = meshes
  };
  for (auto id : getIdsInGroup(world.scenes.at(world.idToScene.at(groupId)), groupId)){
    auto meshesForId = getMeshesForId(world.objectMapping, id);
    for (int i = 0; i < meshesForId.meshes.size(); i++){
      nameAndMeshes.meshNames.push_back(meshesForId.meshNames.at(i));
      nameAndMeshes.meshes.push_back(meshesForId.meshes.at(i));
    }    
  }
  return nameAndMeshes;
}

std::string getDotInfoForNode(std::string nodeName, int nodeId, short groupId, std::vector<std::string> meshes){
  return std::string("\"") + nodeName + "(" + std::to_string(nodeId) + ")" + " meshes: [" + join(meshes, ' ') + "] groupId: " + std::to_string(groupId) + "\"";
}
std::string scenegraphAsDotFormat(Scene& scene, std::map<short, GameObjectObj>& objectMapping){
  std::string graph = "";
  std::string prefix = "strict graph {\n";
  std::string suffix = "}"; 

  std::string relations = "";
  for (auto [id, obj] : scene.idToGameObjectsH){
    auto childId = id;
    auto parentId = obj.parentId;
    auto groupId = obj.groupId;
    auto parentGroupId = parentId != - 1 ? scene.idToGameObjectsH.at(parentId).groupId : -1;

    auto childName = scene.idToGameObjects.at(childId).name;
    auto parentName = parentId == -1 ? "root" : scene.idToGameObjects.at(parentId).name;
        
    relations = relations + getDotInfoForNode(parentName, parentId, parentGroupId, getMeshNames(objectMapping, parentId)) + " -- " + getDotInfoForNode(childName, childId, groupId, getMeshNames(objectMapping, childId)) + "\n";
  }
  graph = graph + prefix + relations + suffix;
  return graph;
}