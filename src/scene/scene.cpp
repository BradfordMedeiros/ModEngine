#include "./scene.h"

void printPhysicsInfo(PhysicsInfo physicsInfo){
  BoundInfo info = physicsInfo.boundInfo;
  std::cout << "x: [ " << info.xMin << ", " << info.xMax << "]" << std::endl;
  std::cout << "y: [ " << info.yMin << ", " << info.yMax << "]" << std::endl;
  std::cout << "z: [ " << info.zMin << ", " << info.zMax << "]" << std::endl;
  std::cout << "pos: (" << physicsInfo.transformation.position.x << ", " << physicsInfo.transformation.position.y << ", " << physicsInfo.transformation.position.z << ")" << std::endl;
  std::cout << "box: (" << physicsInfo.collisionInfo.x << ", " << physicsInfo.collisionInfo.y << ", " << physicsInfo.collisionInfo.z << ")" << std::endl;
}

void dumpPhysicsInfo(std::map<objid, btRigidBody*>& rigidbodys){
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

NameAndMesh getMeshesForGroupId(World& world, objid groupId){
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

objid getGameObjectByName(World& world, std::string name){
  for (auto [sceneId, _] : world.scenes){
    for (auto [id, gameObj]: world.scenes.at(sceneId).idToGameObjects){
      if (gameObj.name == name){
        return id;
      }
    }
  }
  return -1; 
}


PhysicsInfo getPhysicsInfoForGameObject(World& world, Scene& scene, objid index){   // should be "for group id"
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
    .collisionInfo =  obj.transformation.scale,
    .transformation = obj.transformation,
  };

  return info;
}

struct GroupPhysicsInfo {
  bool isRoot;
  PhysicsInfo physicsInfo;
  physicsOpts physicsOptions;
};

GroupPhysicsInfo getPhysicsInfoForGroup(World& world, Scene& scene, objid id){
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


// TODO - physics bug - physicsOptions location/rotation/scale is not relative to parent 
void addPhysicsBody(World& world, Scene& scene, objid id, glm::vec3 initialScale){
  auto groupPhysicsInfo = getPhysicsInfoForGroup(world, scene, id);
  btRigidBody* rigidBody = NULL;

  GameObjectObj& toRender = world.objectMapping.at(id);
  bool isVoxelObj = std::get_if<GameObjectVoxel>(&toRender) != NULL;

  if (groupPhysicsInfo.isRoot){
    auto physicsOptions = groupPhysicsInfo.physicsOptions;
    auto physicsInfo = groupPhysicsInfo.physicsInfo;

    rigidBodyOpts opts = {
      .linear = physicsOptions.linearFactor,
      .angular = physicsOptions.angularFactor,
      .gravity = physicsOptions.gravity,
      .friction = physicsOptions.friction,
      .restitution = physicsOptions.restitution,
      .mass = physicsOptions.mass,
    };

    if (physicsOptions.shape == BOX || (!isVoxelObj && physicsOptions.shape == AUTOSHAPE)){
      std::cout << "INFO: PHYSICS: ADDING BOX RIGID BODY (" << id << ") -- " << (physicsInfo.boundInfo.isNotCentered ? "notcentered" : "centered") << std::endl;
      rigidBody = addRigidBody(
        world.physicsEnvironment, 
        physicsInfo.transformation.position, 
        (physicsInfo.boundInfo.xMax - physicsInfo.boundInfo.xMin), (physicsInfo.boundInfo.yMax - physicsInfo.boundInfo.yMin) , (physicsInfo.boundInfo.zMax - physicsInfo.boundInfo.zMin),
        physicsInfo.transformation.rotation,
        physicsOptions.isStatic,
        physicsOptions.hasCollisions,
        !physicsInfo.boundInfo.isNotCentered,
        physicsInfo.collisionInfo, 
        opts
      );
    }else if (physicsOptions.shape == SPHERE){
      std::cout << "INFO: PHYSICS: ADDING SPHERE RIGID BODY" << std::endl;
      rigidBody = addRigidBody(
        world.physicsEnvironment, 
        physicsInfo.transformation.position,
        maxvalue((physicsInfo.boundInfo.xMax - physicsInfo.boundInfo.xMin), (physicsInfo.boundInfo.yMax - physicsInfo.boundInfo.yMin) , (physicsInfo.boundInfo.zMax - physicsInfo.boundInfo.zMin)),                             
        physicsInfo.transformation.rotation,
        physicsOptions.isStatic,
        physicsOptions.hasCollisions,
        physicsInfo.collisionInfo,
        opts
      );
    }else if (physicsOptions.shape == AUTOSHAPE && isVoxelObj){
      std::cout << "INFO: PHYSICS: ADDING AUTOSHAPE VOXEL RIGID BODY" << std::endl;
      rigidBody = addRigidBody(
        world.physicsEnvironment, 
        physicsInfo.transformation.position,
        physicsInfo.transformation.rotation,
        getVoxelBodies(std::get_if<GameObjectVoxel>(&toRender) -> voxel),
        physicsOptions.isStatic,
        physicsOptions.hasCollisions,
        initialScale,
        opts
      );
    }
  }

  if (rigidBody != NULL){
    world.rigidbodys[id] = rigidBody;   
  }
}
void rmRigidBody(World& world, objid id){
  auto rigidBodyPtr = world.rigidbodys.at(id);
  assert(rigidBodyPtr != NULL);
  rmRigidBody(world.physicsEnvironment, rigidBodyPtr);
  world.rigidbodys.erase(id);
}

void updatePhysicsBody(World& world, Scene& scene, objid id){
  auto rigidBody = world.rigidbodys.at(id);
  assert(rigidBody != NULL);
  glm::vec3 oldScale = getScale(rigidBody);
  rmRigidBody(world, id);
  addPhysicsBody(world, world.scenes.at(world.idToScene.at(id)), id, oldScale);
}

objid getIdForCollisionObject(World& world, const btCollisionObject* body){
  for (auto const&[id, rigidbody] : world.rigidbodys){
    if (rigidbody == body){
      return id;
    }
  }
  return -1;
}

Texture loadTextureWorld(World& world, std::string texturepath){
  if (world.textures.find(texturepath) != world.textures.end()){
    return world.textures.at(texturepath);
  }
  Texture texture = loadTexture(texturepath);
  world.textures[texturepath] = texture;
  return texture;
}
void addMesh(World& world, std::string meshpath){
  ModelData data = loadModel(meshpath);
  assert(data.meshIdToMeshData.size() ==  1);
  auto meshData = data.meshIdToMeshData.begin() -> second;
  world.meshes[meshpath] =  loadMesh("./res/textures/default.jpg", meshData, [&world](std::string texture) -> Texture {
    return loadTextureWorld(world, texture);
  });     
  world.meshnameToBoneToParent[meshpath] = data.boneToParent;
  std::cout << "WARNING: add mesh does not load animations, bones for default meshes" << std::endl;
}

World createWorld(
  collisionPairPosFn onObjectEnter, 
  collisionPairFn onObjectLeave, 
  std::function<void(GameObject&)> onObjectUpdate,  
  std::function<void(GameObject&)> onObjectCreate, 
  std::function<void(objid)> onObjectDelete, 
  btIDebugDraw* debugDrawer
){
  auto objectMapping = getObjectMapping();
  RailSystem rails;
  std::set<objid> entitiesToUpdate;

  World world = {
    .physicsEnvironment = initPhysics(onObjectEnter, onObjectLeave, debugDrawer),
    .objectMapping = objectMapping,
    .rails = rails,
    .onObjectUpdate = onObjectUpdate,
    .onObjectCreate = onObjectCreate,
    .onObjectDelete = onObjectDelete,
    .entitiesToUpdate = entitiesToUpdate,
  };

  // Default meshes that are silently loaded in the background
  addMesh(world, "./res/models/ui/node.obj");
  addMesh(world, "./res/models/boundingbox/boundingbox.obj");
  addMesh(world, "./res/models/cone/cone.obj");
  addMesh(world, "./res/models/camera/camera.dae");

  return world;
}

std::map<objid, std::map<std::string, std::string>> generateAdditionalFields(std::string meshName, ModelData& data, std::map<std::string, std::string> additionalFields, std::vector<std::string> fieldsToCopy){
  std::map<objid, std::map<std::string, std::string>> additionalFieldsMap;
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

  for (auto &[_, fields] : additionalFieldsMap){
    for (auto field : fieldsToCopy){
      assert(fields.find(field) == fields.end());
      if (additionalFields.find(field) != additionalFields.end()){
        fields[field] = additionalFields.at(field);
      }
    }
  }

  return additionalFieldsMap;
}

glm::quat orientationFromPos(glm::vec3 fromPos, glm::vec3 targetPosition){
  // @TODO consider extracting a better up direction from current orientation
  // https://stackoverflow.com/questions/18151845/converting-glmlookat-matrix-to-quaternion-and-back/29992778
  return glm::conjugate(glm::quat_cast(glm::lookAt(fromPos, targetPosition, glm::vec3(0, 1, 0))));
}


glm::vec3 positionFromScene(Scene& scene, objid id){
  return scene.idToGameObjects.at(id).transformation.position;
} 

// Need to take account proper dimensions of the mesh used obj -> should be derivable from mesh boundInfo
void setRailSizing(Scene& scene, BoundInfo info, objid id, std::string from, std::string to){
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
void addObjectToWorld(World& world, Scene& scene, objid sceneId, SerializationObject& serialObj, bool shouldLoadModel, std::function<void(std::string)> loadClip, std::function<objid()> getId){
    auto id =  scene.nameToId.at(serialObj.name);
    auto type = serialObj.type;
    auto additionalFields = serialObj.additionalFields;

    assert(world.idToScene.find(id) == world.idToScene.end());
    world.idToScene[id] = sceneId;
    auto localSceneId = sceneId;

    addObject(id, type, additionalFields, world.objectMapping, world.meshes, "./res/models/ui/node.obj",  loadClip, 
      [&world, &scene, sceneId, loadClip, id, shouldLoadModel, getId, &additionalFields](std::string meshName, std::vector<std::string> fieldsToCopy) -> bool {  // This is a weird function, it might be better considered "ensure model l"
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
            world.meshes[meshPath] = loadMesh("./res/textures/default.jpg", meshData, [&world](std::string texture) -> Texture {
              return loadTextureWorld(world, texture);
            });    
            world.meshnameToBoneToParent[meshPath] = data.boneToParent;  
          } 

          auto newSerialObjs = addSubsceneToRoot(
            scene, 
            id,
            0,
            data.childToParent, 
            data.nodeTransform, 
            data.names, 
            generateAdditionalFields(meshName, data, additionalFields, fieldsToCopy),
            getId
          );

          for (auto &[_, newSerialObj] : newSerialObjs){
            addObjectToWorld(world, scene, sceneId, newSerialObj, false, loadClip, getId);
          }
          return hasMesh;
        }
        return true;   // This is basically ensure model loaded so by definition this was already loaded. 
      }, 
      [&world](std::string texturepath) -> int {
        std::cout << "Custom texture loading: " << texturepath << std::endl;
        return loadTextureWorld(world, texturepath).textureId;
      },
      [&world, localSceneId, id]() -> void {
        updatePhysicsBody(world, world.scenes.at(localSceneId), id);
      },
      [&world, &scene](objid id, std::string from, std::string to) -> void {
        addRail(world.rails, scene.idToGameObjects.at(id).name, from, to);
        auto railMesh =  world.meshes.at("./res/models/ui/node.obj");
        setRailSizing(scene, railMesh.boundInfo, id, from, to);
      }
    );
}

std::string serializeScene(World& world, objid sceneId, bool includeIds){
  std::cout << "scene size: " << world.scenes.size() << std::endl;
  Scene& scene = world.scenes.at(sceneId);
  return serializeScene(scene, [&world](objid objectId)-> std::vector<std::pair<std::string, std::string>> {
    return getAdditionalFields(objectId, world.objectMapping);
  }, includeIds);
}

std::string serializeObject(World& world, objid id){
  Scene& scene = world.scenes.at(world.idToScene.at(id));
  return serializeObject(scene, [&world](objid objectId)-> std::vector<std::pair<std::string, std::string>> {
    return getAdditionalFields(objectId, world.objectMapping);
  }, true, id);
}

objid addSceneToWorldFromData(World& world, objid sceneId, std::string sceneData, std::function<void(std::string)> loadClip, std::function<void(std::string, objid)> loadScript){
  assert(world.scenes.find(sceneId) == world.scenes.end());

  SceneDeserialization deserializedScene = deserializeScene(sceneData, fields, getUniqueObjId);
  world.scenes[sceneId] = deserializedScene.scene;

  for (auto &[_, serialObj] : deserializedScene.serialObjs){
    addObjectToWorld(world, world.scenes.at(sceneId), sceneId, serialObj, true, loadClip, getUniqueObjId);
  }
  for (auto &[id, _] :  world.scenes.at(sceneId).idToGameObjects){
    addPhysicsBody(world,  world.scenes.at(sceneId), id, glm::vec3(1.f, 1.f, 1.f));
  }
  for (auto &[id, obj] : world.scenes.at(sceneId).idToGameObjects){
    if (obj.script != ""){
      loadScript(obj.script, id);
    }
  }
  for (auto &[_, gameobj] : world.scenes.at(sceneId).idToGameObjects){
    world.onObjectCreate(gameobj);
  }
  return sceneId;
}
objid addSceneToWorld(World& world, std::string sceneFile, std::function<void(std::string)> loadClip, std::function<void(std::string, objid)> loadScript){
  return addSceneToWorldFromData(world, getUniqueObjId(), loadFile(sceneFile), loadClip, loadScript);
}

GameObject& getGameObject(World& world, objid id){
  return world.scenes.at(world.idToScene.at(id)).idToGameObjects.at(id);
}

void removeObjectById(World& world, objid objectId, std::function<void(std::string)> unloadClip, std::function<void(std::string, objid)> unloadScript){
  if (world.rigidbodys.find(objectId) != world.rigidbodys.end()){
    auto rigidBody = world.rigidbodys.at(objectId);
    assert(rigidBody != NULL);
    rmRigidBody(world.physicsEnvironment, rigidBody);
    world.rigidbodys.erase(objectId);
  }

  auto scriptName = getGameObject(world, objectId).script;
  if (scriptName != ""){
    unloadScript(scriptName, objectId);
  }
  removeObject(world.objectMapping, objectId, unloadClip, []() -> void {
    std::cout << "INFO: remove rail -- not yet implemented" << std::endl;
    assert(false);
  });
  world.idToScene.erase(objectId);
  world.onObjectDelete(objectId);

  // @TODO IMPORTANT : remove free meshes (no way to tell currently if free -> need counting probably) from meshes
  std::cout << "TODO: MESH MANAGEMENT HORRIBLE NEED TO REMOVE AND NOT BE DUMB ABOUT LOADING THEM" << std::endl;
}
void removeSceneFromWorld(World& world, objid sceneId, std::function<void(std::string)> unloadClip, std::function<void(std::string, objid)> unloadScript){
  if (world.scenes.find(sceneId) == world.scenes.end()) {
    std::cout << "INFO: SCENE MANAGEMENT: tried to remove (" << sceneId << ") but it does not exist" << std::endl;
    return;   // @todo maybe better to throw error instead
  }

  Scene& scene = world.scenes.at(sceneId);
  for (auto objectId : listObjInScene(scene)){
    removeObjectById(world, objectId, unloadClip, unloadScript);
  }
  world.scenes.erase(sceneId);
}
void removeAllScenesFromWorld(World& world, std::function<void(std::string)> unloadClip, std::function<void(std::string, objid)> unloadScript){
  std::vector<objid> sceneIds; 
  for (auto [sceneId, _] : world.scenes){
    sceneIds.push_back(sceneId);
  }
  for (auto sceneId : sceneIds){
    removeSceneFromWorld(world, sceneId, unloadClip, unloadScript);
  }
}

// @TODO remove this code since (makeObject deserialized can express this)
objid addObjectToScene(World& world, objid sceneId, std::string name, std::string meshName, glm::vec3 pos, objid id, bool useObjId, std::function<void(std::string)> loadClip, std::function<void(std::string, objid)> loadScript){
  // @TODO consolidate with addSceneToWorld.  Duplicate code.
  std::vector<objid> idsAdded;
  int numIdsGenerated = 0;
  auto getId = [&idsAdded, &numIdsGenerated, &id, &useObjId]() -> objid {      // kind of hackey, this could just be returned from add objects, but flow control is tricky.
    auto newId = -1;
    if (numIdsGenerated == 0 && useObjId){
      newId = id;
    }else{
      newId = getUniqueObjId();
    }
    numIdsGenerated++;
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

  addObjectToWorld(world, world.scenes.at(sceneId), sceneId, serialObj, true, loadClip, getId);
  auto gameobjId = world.scenes.at(sceneId).nameToId.at(name);
  auto gameobj = world.scenes.at(sceneId).idToGameObjects.at(gameobjId);
  
  for (auto id : idsAdded){
    std::cout << "adding physics body for id: " << std::to_string(id) << std::endl;
    addPhysicsBody(world, world.scenes.at(sceneId), id, glm::vec3(1.f, 1.f, 1.f));
  }
  if (gameobj.script != ""){
    loadScript(gameobj.script, gameobjId);
  }

  world.onObjectCreate(gameobj);
  return gameobjId;
}

objid addObjectToScene(World& world, objid sceneId, std::string serializedObj, objid id, bool useObjId, std::function<void(std::string)> loadClip, std::function<void(std::string, objid)> loadScript){
  std::vector<objid> idsAdded;
  
  int numIdsGenerated = 0;
  auto getId = [&idsAdded, &numIdsGenerated, &id, &useObjId]() -> objid {      // kind of hackey, this could just be returned from add objects, but flow control is tricky.
    auto newId = -1;
    if (numIdsGenerated == 0 && useObjId){
      newId = id;
    }else{
      newId = getUniqueObjId();
    }
    numIdsGenerated++;
    idsAdded.push_back(newId);
    return newId;
  };

  std::cout << "INFO: SCENE - addObjectToScene - start deserialize object, scene id: " << sceneId << std::endl;
  auto serialObj = makeObjectInScene(
    world.scenes.at(sceneId),
    serializedObj,
    getId,   
    fields
  );
  std::cout << "INFO: SCENE - addObjectToScene - start add object to world" << std::endl;
  addObjectToWorld(world, world.scenes.at(sceneId), sceneId, serialObj, true, loadClip, getUniqueObjId);
  auto serialObjId = world.scenes.at(sceneId).nameToId.at(serialObj.name);
  GameObject& gameobj = world.scenes.at(sceneId).idToGameObjects.at(serialObjId);

  std::cout << "INFO: SCENE - addObjectToScene - start add physics bodys" << std::endl;
  for (auto id : idsAdded){
    std::cout << "adding physics body for id: " << std::to_string(id) << std::endl;
    addPhysicsBody(world, world.scenes.at(sceneId), id, glm::vec3(1.f, 1.f, 1.f));
  }

  std::cout << "INFO: SCENE - addObjectToScene - load scripts" << std::endl;
  if (gameobj.script != ""){
    loadScript(gameobj.script, gameobj.id);
  }

  world.onObjectCreate(gameobj);
  return id;
}

objid addObjectToScene(
  World& world, 
  objid sceneId, 
  std::map<std::string, std::string> stringAttributes,
  std::map<std::string, double> numAttributes, 
  std::map<std::string, glm::vec3> vecAttributes
){
  std::cout << "make object placeholder" << std::endl;
  return -1;
}

// this needs to also delete all children objects. 
void removeObjectFromScene(World& world, objid objectId, std::function<void(std::string)> unloadClip, std::function<void(std::string, objid)> unloadScript){  
  Scene& scene = world.scenes.at(world.idToScene.at(objectId));
  auto groupId = scene.idToGameObjectsH.at(objectId).groupId;
  for (auto gameobjId : getIdsInGroup(scene, groupId)){
    if (scene.idToGameObjects.find(gameobjId) == scene.idToGameObjects.end()){
      continue;
    }
    auto removedObjects = removeObjectFromScenegraph(scene, gameobjId);  
    for (auto id : removedObjects){
      removeObjectById(world, id, unloadClip, unloadScript);
    }
  }
}


Properties getProperties(World& world, objid id){
  Properties properties {
    .transformation = getGameObject(world, id).transformation,
  };
  return properties;
}
void setProperties(World& world, objid id, Properties& properties){
  getGameObject(world, id).transformation = properties.transformation;
}

std::map<std::string, std::string> getAttributes(World& world, objid id){
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

void setAttributes(World& world, objid id, std::map<std::string, std::string> attr){
  // @TODO create complete lists for attributes. 
  setObjectAttributes(world.objectMapping, id, extractAttributes(attr, { "mesh", "isDisabled", "clip", "from", "to", "color" }));
  setScenegraphAttributes(world.scenes.at(world.idToScene.at(id)), id, extractAttributes(attr, { "position", "scale", "rotation", "lookat", "layer", "script" }));
}

void physicsTranslate(World& world, objid index, float x, float y, float z, bool moveRelativeEnabled){
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
  world.entitiesToUpdate.insert(index);
}
void physicsTranslateSet(World& world, objid index, glm::vec3 pos){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  scene.idToGameObjects.at(index).transformation.position = pos;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setPosition(body, pos);
  }
  world.entitiesToUpdate.insert(index);
}

void physicsRotate(World& world, objid index, float x, float y, float z){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  glm::quat rotation = setFrontDelta(scene.idToGameObjects.at(index).transformation.rotation, x, y, z, 5);
  scene.idToGameObjects.at(index).transformation.rotation  = rotation;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setRotation(body, rotation);
  }
  world.entitiesToUpdate.insert(index);
}
void physicsRotateSet(World& world, objid index, glm::quat rotation){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  scene.idToGameObjects.at(index).transformation.rotation = rotation;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setRotation(body, rotation);
  }
  world.entitiesToUpdate.insert(index);
}


void physicsScale(World& world, objid index, float x, float y, float z){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  auto oldScale = scene.idToGameObjects.at(index).transformation.scale;
  glm::vec3 newScale = glm::vec3(oldScale.x + x, oldScale.y + y, oldScale.z + z);
  scene.idToGameObjects.at(index).transformation.scale = newScale;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto collisionInfo = getPhysicsInfoForGameObject(world, scene, index).collisionInfo;
    auto body =  world.rigidbodys.at(index);
    setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
  }
  world.entitiesToUpdate.insert(index);
}
void physicsScaleSet(World& world, objid index, glm::vec3 scale){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  scene.idToGameObjects.at(index).transformation.scale = scale;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto collisionInfo = getPhysicsInfoForGameObject(world, scene, index).collisionInfo;
    auto body =  world.rigidbodys.at(index);
    setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
  }
  world.entitiesToUpdate.insert(index);
}

void applyPhysicsTranslation(World& world, objid index, glm::vec3 position, float offsetX, float offsetY, Axis manipulatorAxis){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  auto newPosition = applyTranslation(position, offsetX, offsetY, manipulatorAxis);
  scene.idToGameObjects.at(index).transformation.position = newPosition;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setPosition(body, newPosition);
  }
  world.entitiesToUpdate.insert(index);
}

void applyPhysicsRotation(World& world, objid index, glm::quat currentOrientation, float offsetX, float offsetY, Axis manipulatorAxis){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  auto newRotation = applyRotation(currentOrientation, offsetX, offsetY, manipulatorAxis);
  scene.idToGameObjects.at(index).transformation.rotation = newRotation;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setRotation(body, newRotation);
  }
  world.entitiesToUpdate.insert(index);
}

void applyPhysicsScaling(World& world, objid index, glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, Axis manipulatorAxis){
  Scene& scene = world.scenes.at(world.idToScene.at(index));
  auto newScale = applyScaling(position, initialScale, lastX, lastY, offsetX, offsetY, manipulatorAxis);
  scene.idToGameObjects.at(index).transformation.scale = newScale;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto collisionInfo = getPhysicsInfoForGameObject(world, scene, index).collisionInfo;
    auto body =  world.rigidbodys.at(index);
    setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
  }
  world.entitiesToUpdate.insert(index);
}

void updatePhysicsPositionsAndClampVelocity(World& world, std::map<objid, btRigidBody*>& rigidbodys){
  for (auto [i, rigidBody]: rigidbodys){
    auto sceneId = world.idToScene.at(i);

    // @TODO - physics bug -  getPosition/Rotatin is in world space, need to translate this back relative to parent
    world.scenes.at(sceneId).idToGameObjects.at(i).transformation.rotation = getRotation(rigidBody);   
    world.scenes.at(sceneId).idToGameObjects.at(i).transformation.position = getPosition(rigidBody);

    clampMaxVelocity(rigidBody, world.scenes.at(sceneId).idToGameObjects.at(i).physicsOptions.maxspeed);
  }
}

void enforceLookAt(World& world){
 for (auto &[_, scene] : world.scenes){
    for (auto &[id, gameobj] : scene.idToGameObjects){
      std::string lookAt = gameobj.lookat;                      
      if (lookAt == "" || lookAt == gameobj.name){
        continue;
      }
      if(scene.nameToId.find(lookAt) != scene.nameToId.end()){
        objid lookatId = scene.nameToId.at(lookAt);
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

void callbackEntities(World& world){
  for (auto &[_, scene] : world.scenes){
    for (auto &[id, gameobj] : scene.idToGameObjects){
      if (id == scene.idToGameObjectsH.at(id).groupId && world.entitiesToUpdate.find(id) != world.entitiesToUpdate.end()){
        world.onObjectUpdate(gameobj);
      }
    }
  }
  world.entitiesToUpdate.clear();
}

void onWorldFrame(World& world, float timestep, bool enablePhysics, bool dumpPhysics){
  updateEntities(world);  
  if (enablePhysics){
    if (dumpPhysics){
      dumpPhysicsInfo(world.rigidbodys);
    }
    stepPhysicsSimulation(world.physicsEnvironment, timestep);
    updatePhysicsPositionsAndClampVelocity(world, world.rigidbodys);     
  }
  enforceLookAt(world);   // probably should have physicsTranslateSet, so might be broken
  callbackEntities(world);
}

bool idInGroup(World& world, objid id, objid groupId){
  return groupId == world.scenes.at(world.idToScene.at(id)).idToGameObjectsH.at(id).groupId;
}
bool idExists(World& world, objid id){
  return world.idToScene.find(id) != world.idToScene.end();
}

std::vector<HitObject> raycast(World& world, glm::vec3 posFrom, glm::quat direction, float maxDistance){
  std::vector<HitObject> hitobjects;
  btCollisionWorld::AllHitsRayResultCallback result(glmToBt(posFrom),glmToBt(posFrom));
  auto posTo = moveRelative(posFrom, direction, glm::vec3(0.f, 0.f, -1 * maxDistance), false);
  world.physicsEnvironment.dynamicsWorld -> rayTest(glmToBt(posFrom), glmToBt(posTo), result);

  for (int i = 0; i < result.m_hitFractions.size(); i++){
    const btCollisionObject* obj = result.m_collisionObjects[i];
    for (auto [objid, rigidbody] : world.rigidbodys){
      if (rigidbody == obj){
        hitobjects.push_back(HitObject{
          .id = objid,
        });
      }
    }
  } 
  assert(hitobjects.size() == result.m_hitFractions.size());
  return hitobjects;
}

std::string getDotInfoForNode(std::string nodeName, int nodeId, objid groupId, std::vector<std::string> meshes){
  return std::string("\"") + nodeName + "(" + std::to_string(nodeId) + ")" + " meshes: [" + join(meshes, ' ') + "] groupId: " + std::to_string(groupId) + "\"";
}
std::string scenegraphAsDotFormat(Scene& scene, std::map<objid, GameObjectObj>& objectMapping){
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

// @TODO - this is wrong, needs to get the lights in world space, relative to the parent
std::vector<LightInfo> getLightInfo(World& world){
  auto lightsIndexs = getGameObjectsIndex<GameObjectLight>(world.objectMapping);
  std::vector<LightInfo> lights;
  for (int i = 0; i < lightsIndexs.size(); i++){
    auto objectId =  lightsIndexs.at(i);
    auto objectLight = world.objectMapping.at(objectId);
    auto lightObject = std::get_if<GameObjectLight>(&objectLight);

    auto lightTransform = fullTransformation(world.scenes.at(world.idToScene.at(objectId)), objectId);
    LightInfo light {
      .pos = lightTransform.position,
      .rotation = lightTransform.rotation,
      .color = lightObject -> color,
    };
    lights.push_back(light);
  }
  return lights;
}