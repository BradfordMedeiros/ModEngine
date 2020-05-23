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

BoundInfo getMaxUnionBoundingInfo(std::vector<BoundInfo> boundings){    // Takes the biggest, assuming one physics object per collision.  Can be inaccurate with
  //assert(boundings.size() == 1);
  return boundings.at(0);
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

short getGameObjectByName(World& world, std::string name){
  for (int i = 0; i < world.scenes.size(); i++){
    for (auto [id, gameObj]: world.scenes.at(i).idToGameObjects){
      if (gameObj.name == name){
        return id;
      }
    }
  }
  return -1; 
}


PhysicsInfo getPhysicsInfoForGameObject(World& world, Scene& scene, short index){   // should be "for group id"
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
  if (meshObj != NULL){
    std::vector<BoundInfo> boundInfos;
    auto meshes = getMeshesForGroupId(world, index).meshes;
    std::cout << "INFO: number meshes for id: (" << std::to_string(index) << ")  -- " << std::to_string(meshes.size()) << std::endl;
    for (Mesh& mesh : meshes){
      boundInfos.push_back(mesh.boundInfo);
    }
    std::cout << "size: " << boundInfos.size() << std::endl;
    boundInfo = getMaxUnionBoundingInfo(boundInfos);
  }

  auto voxelObj = std::get_if<GameObjectVoxel>(&gameObjV);
  if (voxelObj != NULL){
    boundInfo = voxelObj -> voxel.mesh.boundInfo;
  }

  auto railObj = std::get_if<GameObjectRail>(&gameObjV);
  if (railObj != NULL){
    auto railMesh =  world.meshes.at("./res/models/ui/node.obj");
    std::vector<BoundInfo> infos;
    infos.push_back(railMesh.boundInfo);
    boundInfo = getMaxUnionBoundingInfo(infos);
  }

  PhysicsInfo info = {
    .boundInfo = boundInfo,
    .gameobject = obj,
    .collisionInfo =  obj.transformation.scale
  };

  return info;
}

struct GroupPhysicsInfo {
  bool isRoot;
  PhysicsInfo physicsInfo;
  physicsOpts physicsOptions;
};

GroupPhysicsInfo getPhysicsInfoForGroup(World& world, Scene& scene, short id){
  auto groupId = scene.idToGameObjectsH.at(id).groupId;
  bool isRoot = groupId == id;
  if (!isRoot){
    return GroupPhysicsInfo {
      .isRoot = isRoot,
    };
  }

  GroupPhysicsInfo groupInfo {
    .isRoot = isRoot,
    .physicsInfo = getPhysicsInfoForGameObject(world, scene, groupId),
    .physicsOptions = scene.idToGameObjects.at(id).physicsOptions,
  };  
  return groupInfo;
}


void addPhysicsBody(World& world, Scene& scene, short id, glm::vec3 initialScale){
  auto groupPhysicsInfo = getPhysicsInfoForGroup(world, scene, id);
  btRigidBody* rigidBody = NULL;

  GameObjectObj& toRender = world.objectMapping.at(id);
  bool isVoxelObj = std::get_if<GameObjectVoxel>(&toRender) != NULL;
 
  if (groupPhysicsInfo.isRoot){
    auto physicsOptions = groupPhysicsInfo.physicsOptions;
    auto physicsInfo = groupPhysicsInfo.physicsInfo;
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
        physicsInfo.collisionInfo, 
        physicsOptions.linearFactor,
        physicsOptions.angularFactor,
        physicsOptions.gravity
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
        physicsInfo.collisionInfo,
        physicsOptions.linearFactor,
        physicsOptions.angularFactor,
        physicsOptions.gravity
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
        initialScale,
        physicsOptions.linearFactor,
        physicsOptions.angularFactor,
        physicsOptions.gravity
      );
    }
  }

  if (rigidBody != NULL){
    world.rigidbodys[id] = rigidBody;   
  }
}
void rmRigidBody(World& world, short id){
  auto rigidBodyPtr = world.rigidbodys.at(id);
  assert(rigidBodyPtr != NULL);
  rmRigidBody(world.physicsEnvironment, rigidBodyPtr);
  world.rigidbodys.erase(id);
}

void updatePhysicsBody(World& world, Scene& scene, short id){
  auto rigidBody = world.rigidbodys.at(id);
  assert(rigidBody != NULL);
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
  RailSystem rails;
  World world = {
    .physicsEnvironment = initPhysics(onObjectEnter, onObjectLeave, debugDrawer),
    .objectMapping = objectMapping,
    .rails = rails,
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

glm::quat orientationFromPos(glm::vec3 fromPos, glm::vec3 targetPosition){
  // @TODO consider extracting a better up direction from current orientation
  // https://stackoverflow.com/questions/18151845/converting-glmlookat-matrix-to-quaternion-and-back/29992778
  return glm::conjugate(glm::quat_cast(glm::lookAt(fromPos, targetPosition, glm::vec3(0, 1, 0))));
}


glm::vec3 positionFromScene(Scene& scene, short id){
  return scene.idToGameObjects.at(id).transformation.position;
} 

// Need to take account proper dimensions of the mesh used obj -> should be derivable from mesh boundInfo
void setRailSizing(Scene& scene, BoundInfo info, short id, std::string from, std::string to){
  auto zLength = info.zMax - info.zMin;
  auto fromPosition = positionFromScene(scene, scene.nameToId.at(from));
  auto toPosition = positionFromScene(scene, scene.nameToId.at(to));
  auto distance = glm::distance(fromPosition, toPosition);
  auto orientation = orientationFromPos(fromPosition, toPosition);
  GameObject& obj = scene.idToGameObjects.at(id);

  glm::vec3 scale = glm::vec3(1.f, 1.f, distance / zLength);
  obj.transformation.scale = scale;
  glm::vec3 meshOffset = orientation * (glm::vec3(0.f, 0.f, 1.f) * distance * 0.5f);
  obj.transformation.position = fromPosition - meshOffset;  
  obj.transformation.rotation = orientation;

}
void addObjects(World& world, Scene& scene, std::map<std::string, SerializationObject>& serialObjs, bool shouldLoadModel, std::function<void(std::string)> loadClip, std::function<short()> getId){
  for (auto [_, serialObj] : serialObjs){
    auto id =  scene.nameToId.at(serialObj.name);
    auto type = serialObj.type;
    auto additionalFields = serialObj.additionalFields;

    world.idToScene[id] = sceneId;
    auto localSceneId = sceneId;

    addObject(id, type, additionalFields, world.objectMapping, world.meshes, "./res/models/ui/node.obj",  loadClip, 
      [&world, &scene, loadClip, id, shouldLoadModel, getId](std::string meshName) -> bool {  // This is a weird function, it might be better considered "ensure model l"
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
            getId
          );
          addObjects(world, scene, newSerialObjs, false, loadClip, getId);
          return hasMesh;
        }
        return true;   // This is basically ensure model loaded so by definition this was already loaded. 
      }, 
      [&world, localSceneId, id]() -> void {
        updatePhysicsBody(world, world.scenes.at(sceneId), id);
      },
      [&world, &scene](short id, std::string from, std::string to) -> void {
        addRail(world.rails, scene.idToGameObjects.at(id).name, from, to);
        auto railMesh =  world.meshes.at("./res/models/ui/node.obj");
        setRailSizing(scene, railMesh.boundInfo, id, from, to);
      }
    );
  }
}

std::string serializeScene(World& world, short sceneId){
  std::cout << "scene size: " << world.scenes.size() << std::endl;
  Scene& scene = world.scenes.at(sceneId);
  return serializeScene(scene, [&world](short objectId)-> std::vector<std::pair<std::string, std::string>> {
    return getAdditionalFields(objectId, world.objectMapping);
  });
}

short addSceneToWorld(World& world, std::string sceneFile, std::function<void(std::string)> loadClip, std::function<void(std::string, short)> loadScript){
  auto sceneId = getSceneId();
  SceneDeserialization deserializedScene = deserializeScene(loadFile(sceneFile), fields, getObjectId);
  world.scenes[sceneId] = deserializedScene.scene;
  addObjects(world, world.scenes.at(sceneId), deserializedScene.serialObjs, true, loadClip, getObjectId);

  addPhysicsBodies(world, world.scenes.at(sceneId));
  for (auto &[id, obj] : world.scenes.at(sceneId).idToGameObjects){
    if (obj.script != ""){
      loadScript(obj.script, id);
    }
  }
  return sceneId;
}

void removeObjectById(World& world, short objectId, std::function<void(std::string)> unloadClip){
  auto rigidBody = world.rigidbodys.at(objectId);
  assert(rigidBody != NULL);
  rmRigidBody(world.physicsEnvironment, rigidBody);
  world.rigidbodys.erase(objectId);
  removeObject(world.objectMapping, objectId, unloadClip, []() -> void {
    std::cout << "INFO: remove rail -- not yet implemented" << std::endl;
    assert(false);
  });
  world.idToScene.erase(objectId);
  // @TODO IMPORTANT : remove free meshes (no way to tell currently if free -> need counting probably) from meshes
  std::cout << "TODO: MESH MANAGEMENT HORRIBLE NEED TO REMOVE AND NOT BE DUMB ABOUT LOADING THEM" << std::endl;
}
void removeSceneFromWorld(World& world, short sceneId, std::function<void(std::string)> unloadClip){
  if (world.scenes.find(sceneId) == world.scenes.end()) {
    std::cout << "INFO: SCENE MANAGEMENT: tried to remove (" << sceneId << ") but it does not exist" << std::endl;
    return;   // @todo maybe better to throw error instead
  }

  Scene& scene = world.scenes.at(sceneId);
  for (auto objectId : listObjInScene(scene)){
    removeObjectById(world, objectId, unloadClip);
  }
  world.scenes.erase(sceneId);
}

void addObject(World& world, short sceneId, std::string name, std::string meshName, glm::vec3 pos, std::function<void(std::string)> loadClip, std::function<void(std::string, short)> loadScript){
  // @TODO consolidate with addSceneToWorld.  Duplicate code.
  std::vector<short> idsAdded;
  auto getId = [&idsAdded]() -> short {      // kind of hackey, this could just be returned from add objects, but flow control is tricky.
    auto newId = getObjectId();
    idsAdded.push_back(newId);
    return newId;
  };

  auto serialObj = makeObjectInScene(
    world.scenes.at(sceneId), 
    name, 
    meshName, 
    pos, 
    "default",
    getId,
    fields
  );

  std::map<std::string, SerializationObject> serialObjs;
  serialObjs[name] = serialObj;

  addObjects(world, world.scenes.at(sceneId), serialObjs, true, loadClip, getId);
  auto gameobjId = world.scenes.at(sceneId).nameToId.at(name);
  auto gameobj = world.scenes.at(sceneId).idToGameObjects.at(gameobjId);
  
  for (auto id : idsAdded){
    std::cout << "adding physics body for id: " << std::to_string(id) << std::endl;
    addPhysicsBody(world, world.scenes.at(sceneId), id, glm::vec3(1.f, 1.f, 1.f));
  }
  if (gameobj.script != ""){
    loadScript(gameobj.script, gameobjId);
  }
}
void removeObject(World& world, short objectId, std::function<void(std::string)> unloadClip){  // this needs to also delete all children objects. 
  Scene& scene = world.scenes.at(world.idToScene.at(objectId));
  auto groupId = scene.idToGameObjectsH.at(objectId).groupId;
  for (auto gameobjId : getIdsInGroup(scene, groupId)){
    if (scene.idToGameObjects.find(gameobjId) == scene.idToGameObjects.end()){
      continue;
    }
    auto removedObjects = removeObjectFromScene(scene, gameobjId);  
    for (auto id : removedObjects){
      removeObjectById(world, id, unloadClip);
    }
  }
}

void physicsTranslate(World& world, short index, float x, float y, float z, bool moveRelativeEnabled){
  Scene& scene = world.scenes.at(world.idToScene.at(index));

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

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setPosition(body, newPosition);
  }
}
void physicsTranslateSet(World& world, short index, glm::vec3 pos){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  scene.idToGameObjects.at(index).transformation.position = pos;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setPosition(body, pos);
  }
}

void physicsRotate(World& world, short index, float x, float y, float z){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  glm::quat rotation = setFrontDelta(scene.idToGameObjects.at(index).transformation.rotation, x, y, z, 5);
  scene.idToGameObjects.at(index).transformation.rotation  = rotation;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setRotation(body, rotation);
  }
}
void physicsRotateSet(World& world, short index, glm::quat rotation){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  scene.idToGameObjects.at(index).transformation.rotation = rotation;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setRotation(body, rotation);
  }
}

void physicsScale(World& world, short index, float x, float y, float z){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  auto oldScale = scene.idToGameObjects.at(index).transformation.scale;
  glm::vec3 newScale = glm::vec3(oldScale.x + x, oldScale.y + y, oldScale.z + z);
  scene.idToGameObjects.at(index).transformation.scale = newScale;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto collisionInfo = getPhysicsInfoForGameObject(world, scene, index).collisionInfo;
    auto body =  world.rigidbodys.at(index);
    setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
  }
}

void applyPhysicsTranslation(World& world, short index, glm::vec3 position, float offsetX, float offsetY, Axis manipulatorAxis){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  auto newPosition = applyTranslation(position, offsetX, offsetY, manipulatorAxis);
  scene.idToGameObjects.at(index).transformation.position = newPosition;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setPosition(body, newPosition);
  }
}

void applyPhysicsRotation(World& world, short index, glm::quat currentOrientation, float offsetX, float offsetY, Axis manipulatorAxis){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  auto newRotation = applyRotation(currentOrientation, offsetX, offsetY, manipulatorAxis);
  scene.idToGameObjects.at(index).transformation.rotation = newRotation;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setRotation(body, newRotation);
  }
}

void applyPhysicsScaling(World& world, short index, glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, Axis manipulatorAxis){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  auto newScale = applyScaling(position, initialScale, lastX, lastY, offsetX, offsetY, manipulatorAxis);
  scene.idToGameObjects.at(index).transformation.scale = newScale;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto collisionInfo = getPhysicsInfoForGameObject(world, scene, index).collisionInfo;
    auto body =  world.rigidbodys.at(index);
    setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
  }
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
        gameobj.transformation.rotation =  orientationFromPos(fromPos, targetPosition);;
      }
    }
  }
}

void updateEntities(World& world){
  for (auto &activeRail : world.rails.activeRails){
    auto entityId = activeRail.id;
    auto entityPosition = world.scenes.at(world.idToScene.at(entityId)).idToGameObjects.at(entityId).transformation.position;
    auto entityOrientation = world.scenes.at(world.idToScene.at(entityId)).idToGameObjects.at(entityId).transformation.rotation;

    if (activeRail.rail != ""){
      auto nextRail = nextPosition(
        world.rails, 
        [&world](std::string value) -> glm::vec3 { 
          auto objectId = getGameObjectByName(world, value);
          return world.scenes.at(world.idToScene.at(objectId)).idToGameObjects.at(objectId).transformation.position;
        }, 
        activeRail.rail, 
        entityPosition, 
        entityOrientation
      ); 
      activeRail.rail = nextRail.rail;
      physicsTranslateSet(world, entityId, nextRail.position);
    }
  }
}

void onWorldFrame(World& world, float timestep, bool enablePhysics, bool dumpPhysics){
  updateEntities(world);  
  if (enablePhysics){
    if (dumpPhysics){
      dumpPhysicsInfo(world.rigidbodys);
    }
    stepPhysicsSimulation(world.physicsEnvironment, timestep);
    updatePhysicsPositions(world, world.rigidbodys);     
  }
  enforceLookAt(world);   // probably should have physicsTranslateSet, so might be broken
}

std::map<std::string, std::string> getAttributes(World& world, short id){
  // @TODO handle types better
  std::map<std::string, std::string> attr;
  auto objAttrs = objectAttributes(world.objectMapping, id);
  auto sceneAttrs = scenegraphAttributes(world.scenes.at(world.idToScene.at(id)), id);

  for (auto [attrName, attrValue] : objAttrs){
    attr[attrName] = attrValue;
  }
  for (auto [attrName, attrValue] : sceneAttrs){
    attr[attrName] = attrValue;
  }
  return attr;
}

std::map<std::string, std::string> extractAttributes(std::map<std::string, std::string>& attr,  std::vector<std::string> attributes){
  std::map<std::string, std::string> attrToSet;
  for (auto attribute : attributes){
    if (attr.find(attribute) != attr.end()){
      attrToSet[attribute] = attr.at(attribute);
    }    
  }
  return attrToSet;
}

void setAttributes(World& world, short id, std::map<std::string, std::string> attr){
  // @TODO create complete lists for attributes. 
  setObjectAttributes(world.objectMapping, id, extractAttributes(attr, { "mesh", "isDisabled", "clip", "from", "to", "color" }));
  setScenegraphAttributes(world.scenes.at(world.idToScene.at(id)), id, extractAttributes(attr, { "position", "scale", "rotation", "lookat", "layer", "script" }));
}

bool idInGroup(World& world, short id, short groupId){
  return groupId == world.scenes.at(world.idToScene.at(id)).idToGameObjectsH.at(id).groupId;
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

