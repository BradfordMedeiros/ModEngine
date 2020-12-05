#include "./scene.h"

void forEveryGameobj(World& world, std::function<void(objid id, Scene& scene, GameObject& gameobj)> onElement){
  for (auto &[_, scene] : world.scenes){
    for (auto [id, gameObj]: scene.idToGameObjects){
      onElement(id, scene, gameObj);
    }
  }
}
Scene& sceneForId(World& world, objid id){
  return world.scenes.at(world.idToScene.at(id));
}
std::vector<objid> allSceneIds(World& world){
  std::vector<objid> sceneIds;
  for (auto [sceneId, _] : world.scenes){
    sceneIds.push_back(sceneId);
  }
  return sceneIds;
} 

GameObject& getGameObject(World& world, objid id){
  return getGameObject(sceneForId(world, id), id);
}
std::optional<GameObject*> maybeGetGameObjectByName(World& world, std::string name){
  for (auto [sceneId, _] : world.scenes){
    for (auto &[id, gameObj]: world.scenes.at(sceneId).idToGameObjects){
      if (gameObj.name == name){
       return &gameObj;
      }
    }
  }
  return std::nullopt;
}
std::optional<objid> getGameObjectByName(World& world, std::string name){
  auto obj = maybeGetGameObjectByName(world, name);
  if (obj.has_value()){
    return obj.value() -> id;
  }
  return std::nullopt;
}
GameObject& getGameObject(World& world, std::string name){
  auto obj = maybeGetGameObjectByName(world, name);
  if (obj.has_value()){
    return *obj.value();
  }
  std::cout << "gameobject : " << name << " does not exist" << std::endl;
  assert(false);
}

objid getGroupId(World& world, objid id){
  return getGroupId(sceneForId(world, id), id); 
}
std::vector<objid> getIdsInGroup(World& world, objid index){
  return getIdsInGroup(sceneForId(world, index), getGroupId(world, index));
}

bool idInGroup(World& world, objid id, std::vector<objid> groupIds){
  auto groupId = getGroupId(world, id);
  for (auto gId : groupIds){
    if (groupId == gId){
      return true;
    }
  }
  return false;
}
bool idExists(World& world, objid id){
  return world.idToScene.find(id) != world.idToScene.end();
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

PhysicsInfo getPhysicsInfoForGameObject(World& world, Scene& scene, objid index){   // should be "for group id"
  GameObject obj = getGameObject(scene, index);
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
    boundInfo = voxelObj -> voxel.boundInfo;
  }

  auto navmeshObj = std::get_if<GameObjectNavmesh>(&gameObjV);
  if (navmeshObj != NULL){
    boundInfo = navmeshObj -> mesh.boundInfo;
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
  auto groupId = getGroupId(scene, id);
  bool isRoot = groupId == id;
  if (!isRoot){
    return GroupPhysicsInfo {
      .isRoot = isRoot,
    };
  }

  GroupPhysicsInfo groupInfo {
    .isRoot = isRoot,
    .physicsInfo = getPhysicsInfoForGameObject(world, scene, groupId),
    .physicsOptions = getGameObject(scene, groupId).physicsOptions,
  };  
  return groupInfo;
}


// TODO - physics bug - physicsOptions location/rotation/scale is not relative to parent 
void addPhysicsBody(World& world, Scene& scene, objid id, glm::vec3 initialScale){
  auto groupPhysicsInfo = getPhysicsInfoForGroup(world, scene, id);
  if (!groupPhysicsInfo.physicsOptions.enabled){
    return;
  }

  btRigidBody* rigidBody = NULL;

  GameObjectObj& toRender = world.objectMapping.at(id);
  bool isVoxelObj = std::get_if<GameObjectVoxel>(&toRender) != NULL;
  GameObjectHeightmap* heightmapObj = std::get_if<GameObjectHeightmap>(&toRender);
  bool isHeightmapObj = heightmapObj != NULL;

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

    if (isHeightmapObj){
      rigidBody = addRigidBodyHeightmap(
        world.physicsEnvironment,
        physicsInfo.transformation.position,
        physicsInfo.transformation.rotation,
        physicsOptions.isStatic,
        opts,
        heightmapObj -> heightmap.data,
        heightmapObj -> heightmap.width, 
        heightmapObj -> heightmap.height,
        physicsInfo.collisionInfo,
        heightmapObj -> heightmap.minHeight,
        heightmapObj -> heightmap.maxHeight
      );
    }else if (physicsOptions.shape == BOX || (!isVoxelObj && physicsOptions.shape == AUTOSHAPE)){
      std::cout << "INFO: PHYSICS: ADDING BOX RIGID BODY (" << id << ") -- " << (physicsInfo.boundInfo.isNotCentered ? "notcentered" : "centered") << std::endl;
      rigidBody = addRigidBodyRect(
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
      rigidBody = addRigidBodySphere(
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
      rigidBody = addRigidBodyVoxel(
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
  addPhysicsBody(world, sceneForId(world, id), id, oldScale);
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
Texture loadTextureDataWorld(World& world, std::string texturepath, unsigned char* data, int textureWidth, int textureHeight, int numChannels){
  if (world.textures.find(texturepath) != world.textures.end()){
    return world.textures.at(texturepath);
  }
  Texture texture = loadTextureData(data, textureWidth, textureHeight, numChannels);
  world.textures[texturepath] = texture;
  return texture;  
}
void updateTextureDataWorld(World& world, std::string texturepath, unsigned char* data, int textureWidth, int textureHeight){
  updateTextureData(world.textures.at(texturepath), data, textureWidth, textureHeight);
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
  std::function<void(objid, bool)> onObjectDelete, 
  btIDebugDraw* debugDrawer
){
  auto objectMapping = getObjectMapping();
  EmitterSystem emitters;
  std::set<objid> entitiesToUpdate;

  World world = {
    .physicsEnvironment = initPhysics(onObjectEnter, onObjectLeave, debugDrawer),
    .objectMapping = objectMapping,
    .emitters = emitters,
    .onObjectUpdate = onObjectUpdate,
    .onObjectCreate = onObjectCreate,
    .onObjectDelete = onObjectDelete,
    .entitiesToUpdate = entitiesToUpdate,
  };

  // Default meshes that are silently loaded in the background
  addMesh(world, "./res/models/ui/node.obj");
  addMesh(world, "./res/models/boundingbox/boundingbox.obj");
  addMesh(world, "./res/models/unit_rect/unit_rect.obj");
  addMesh(world, "./res/models/cone/cone.obj");
  addMesh(world, "./res/models/camera/camera.dae");
  addMesh(world, "./res/models/box/plane.dae");
  addMesh(world, "./res/models/controls/input.obj");
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

  for (auto &[_, fields] : additionalFieldsMap){    // @TODO - this looks wrong, shouldn't we copy all fields? 
    for (auto field : fieldsToCopy){
      assert(fields.find(field) == fields.end());
      if (additionalFields.find(field) != additionalFields.end()){
        fields[field] = additionalFields.at(field);
      }
    }
  }

  return additionalFieldsMap;
}

std::string getType(std::string name, std::vector<Field> additionalFields){
  std::string type = "default";
  for (Field field : additionalFields){
    if (name[0] == field.prefix){
      type = field.type;
    }
  }
  return type;
}

void addObjectToWorld(
  World& world, 
  Scene& scene, 
  objid sceneId, 
  std::string name,
  bool shouldLoadModel, 
  std::function<objid()> getId,
  SysInterface interface,
  glm::vec3 tint,
  std::map<std::string, std::string> additionalFields
){
    auto id =  scene.nameToId.at(name);

    if (world.idToScene.find(id) != world.idToScene.end()){
      std::cout << "id already in the scene: " << id << std::endl;
      assert(false);
    }

    world.idToScene[id] = sceneId;
    auto localSceneId = sceneId;

    addObject(id, getType(name, fields), additionalFields, world.objectMapping, world.meshes, "./res/models/ui/node.obj",
      [&world, &scene, sceneId, id, shouldLoadModel, getId, &additionalFields, &interface, tint](std::string meshName, std::vector<std::string> fieldsToCopy) -> bool {  // This is a weird function, it might be better considered "ensure model l"
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
            getId,
            tint
          );

          for (auto &[name, newSerialObj] : newSerialObjs){
            addObjectToWorld(world, scene, sceneId, name, false, getId, interface, newSerialObj.tint, newSerialObj.additionalFields);
          }
          return hasMesh;
        }
        return true;   // This is basically ensure model loaded so by definition this was already loaded. 
      }, 
      [&world](std::string texturepath) -> Texture {
        std::cout << "Custom texture loading: " << texturepath << std::endl;
        return loadTextureWorld(world, texturepath);
      },
      [&world](std::string texturepath, unsigned char* data, int textureWidth, int textureHeight, int numChannels) -> Texture {
        return loadTextureDataWorld(world, texturepath, data, textureWidth, textureHeight, numChannels);
      },
      [&world, localSceneId, id]() -> void {
        updatePhysicsBody(world, world.scenes.at(localSceneId), id);
      },
      [&world, sceneId, id, &interface](std::string sceneToLoad) -> void {
        std::cout << "INFO: -- SCENE LOADING : " << sceneToLoad << std::endl;
        auto childSceneId = addSceneToWorld(world, sceneToLoad, interface);
        auto rootId = world.scenes.at(childSceneId).rootId;
        addChildLink(world.scenes.at(sceneId), rootId, id);
        world.scenes.at(childSceneId).isNested = true;
      },
      [&world, &interface, name, id](float spawnrate, float lifetime, int limit, std::map<std::string, std::string> particleFields) -> void {
        std::vector<EmitterDelta> deltas;
        deltas.push_back(
          EmitterDelta {
            .attributeName = "position",
            .value = glm::vec3(0.f, 0.1f, 0.f),
        });
        deltas.push_back(
          EmitterDelta {
            .attributeName = "scale",
            .value = glm::vec3(0.01f, 0.01f, 0.01f),
        });
        deltas.push_back(
          EmitterDelta {
            .attributeName = "tint",
            .value = glm::vec3(0.05f, 0.0f, 0.0f),
        });
        addEmitter(world.emitters, name, id, interface.getCurrentTime(), limit, spawnrate, lifetime, fieldsToAttributes(particleFields), deltas);
      },
      [&world](MeshData& meshdata) -> Mesh {
        return loadMesh("./res/textures/default.jpg", meshdata, [&world](std::string texture) -> Texture {
          return loadTextureWorld(world, texture);
        });    
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

std::string serializeObject(World& world, objid id, std::string overridename){
  Scene& scene = sceneForId(world, id);
  return serializeObject(
    scene, 
    id, 
    [&world](objid objectId)-> std::vector<std::pair<std::string, std::string>> {
      return getAdditionalFields(objectId, world.objectMapping);
    }, 
    false,
    overridename
  );
}

void addSerialObjectsToWorld(
  World& world, 
  objid sceneId, 
  std::vector<objid> idsAdded,
  std::function<objid()> getNewObjectId,
  SysInterface interface,
  std::map<std::string, std::map<std::string, std::string>> additionalFields,
  std::map<std::string, glm::vec3> tint
){
  for (auto &[name, additionalField] : additionalFields){
    // Warning: getNewObjectId will mutate the idsAdded.  
    addObjectToWorld(world, world.scenes.at(sceneId), sceneId, name, true, getNewObjectId, interface, tint.at(name), additionalField);
  }
  for (auto id : idsAdded){
    addPhysicsBody(world,  world.scenes.at(sceneId), id, glm::vec3(1.f, 1.f, 1.f));   
  }

  for (auto id : idsAdded){
    auto obj = getGameObject(world, id);
    if (obj.script != ""){
      interface.loadScript(obj.script, id);
    }
  }

  for (auto id : idsAdded){
    auto obj = getGameObject(world, id); 
    world.onObjectCreate(obj);
  }
}

objid addSceneToWorldFromData(World& world, objid sceneId, std::string sceneData, SysInterface interface){
  assert(world.scenes.find(sceneId) == world.scenes.end());

  SceneDeserialization deserializedScene = deserializeScene(sceneData, getUniqueObjId);
  world.scenes[sceneId] = deserializedScene.scene;
  std::vector<objid> idsAdded;
  for (auto &[id, _] :  world.scenes.at(sceneId).idToGameObjects){
    idsAdded.push_back(id);
  }

  addSerialObjectsToWorld(world, sceneId, idsAdded, getUniqueObjId, interface, deserializedScene.additionalFields, deserializedScene.tints);
  return sceneId;
}

objid addSerialObject(
  World& world, 
  objid sceneId, 
  std::string name, 
  SysInterface interface, 
  std::map<std::string, std::string> additionalFields, 
  std::vector<std::string> children, 
  GameObject gameobjectObj, 
  std::vector<objid>& idsAdded
){
  auto getId = [&idsAdded]() -> objid {      // kind of hackey, this could just be returned from add objects, but flow control is tricky.
    auto newId = getUniqueObjId();
    idsAdded.push_back(newId);
    return newId;
  };

  addGameObjectToScene(world.scenes.at(sceneId), name, gameobjectObj, children);

  std::map<std::string, std::map<std::string, std::string>> additionalFieldsMap;
  std::map<std::string, glm::vec3> tints;
  additionalFieldsMap[name] = additionalFields;
  tints[name] = gameobjectObj.tint;

  addSerialObjectsToWorld(world, sceneId, idsAdded, getId, interface, additionalFieldsMap, tints);

  auto gameobjId = world.scenes.at(sceneId).nameToId.at(name);
  return gameobjId;
}

objid addSceneToWorld(World& world, std::string sceneFile, SysInterface interface){
  return addSceneToWorldFromData(world, getUniqueObjId(), loadFile(sceneFile), interface);
}

void removeObjectById(World& world, objid objectId, std::string name, SysInterface interface, std::string scriptName, bool netsynchronized){
  if (world.rigidbodys.find(objectId) != world.rigidbodys.end()){
    auto rigidBody = world.rigidbodys.at(objectId);
    assert(rigidBody != NULL);
    rmRigidBody(world.physicsEnvironment, rigidBody);
    world.rigidbodys.erase(objectId);
  }

  if (scriptName != ""){
    interface.unloadScript(scriptName, objectId);
  }
  removeObject(
    world.objectMapping, 
    objectId, 
    [](std::string cameraName) -> void {
      std::cout << "remove camera not yet implemented" << std::endl;
      assert(false);
    },
    [&world, name]() -> void {
      removeEmitter(world.emitters, name);
    }
  );
  
  world.idToScene.erase(objectId);
  world.onObjectDelete(objectId, netsynchronized);

  // @TODO IMPORTANT : remove free meshes (no way to tell currently if free -> need counting probably) from meshes
  std::cout << "TODO: MESH MANAGEMENT HORRIBLE NEED TO REMOVE AND NOT BE DUMB ABOUT LOADING THEM" << std::endl;
}
// this needs to also delete all children objects. 
void removeObjectFromScene(World& world, objid objectId, SysInterface interface){  
  Scene& scene = sceneForId(world, objectId);
  auto groupId = getGroupId(scene, objectId);
  for (auto gameobjId : getIdsInGroup(scene, groupId)){
    if (!idExists(scene, gameobjId)){
      continue;
    }
    auto idsToRemove = idsToRemoveFromScenegraph(scene, gameobjId);
    for (auto id : idsToRemove){
      auto gameobj = getGameObject(world, id);
      auto name = gameobj.name;
      auto scriptName = gameobj.script;
      auto netsynchronized = gameobj.netsynchronize;
      removeObjectById(world, id, name, interface, scriptName, netsynchronized);
    }
    removeObjectsFromScenegraph(scene, idsToRemove);  
  }
}

void copyObjectToScene(World& world, objid id, SysInterface interface){
  std::cout << "INFO: SCENE: COPY OBJECT: " << id << std::endl;
  auto serializedObject = serializeObject(world, id, getGameObject(world, id).name + "-copy");
  std::cout << "copy object: serialized object is: " << std::endl;
  std::cout << serializedObject << std::endl << std::endl;
  addObjectToScene(world, world.idToScene.at(id), serializedObject, -1, false, interface);
}


void removeSceneFromWorld(World& world, objid sceneId, SysInterface interface){
  if (world.scenes.find(sceneId) == world.scenes.end()) {
    std::cout << "INFO: SCENE MANAGEMENT: tried to remove (" << sceneId << ") but it does not exist" << std::endl;
    return;   // @todo maybe better to throw error instead
  }

  Scene& scene = world.scenes.at(sceneId);
  for (auto objectId : listObjInScene(scene)){
    auto gameobj = getGameObject(world, objectId);
    auto name = gameobj.name;
    auto scriptName = gameobj.script;
    auto netsynchronized = gameobj.netsynchronize;
    removeObjectById(world, objectId, name, interface, scriptName, netsynchronized);
  }
  world.scenes.erase(sceneId);
}
void removeAllScenesFromWorld(World& world, SysInterface interface){
  for (auto sceneId : allSceneIds(world)){
    removeSceneFromWorld(world, sceneId, interface);
  }
}


objid addObjectToScene(
  World& world, 
  objid sceneId, 
  std::string name, 
  GameobjAttributes attributes,
  SysInterface interface
){
  int id = attributes.numAttributes.find("id") != attributes.numAttributes.end() ? attributes.numAttributes.at("id") : -1;
  bool useObjId = attributes.numAttributes.find("id") != attributes.numAttributes.end();

  std::vector<objid> idsAdded;
  auto idToAdd = useObjId ? id : getUniqueObjId();
  idsAdded.push_back(idToAdd);
  auto gameobjectObj = gameObjectFromFields(name, "default", idToAdd, attributes);
  return addSerialObject(world, sceneId, name, interface, attributes.additionalFields, attributes.children, gameobjectObj, idsAdded);
}

objid addObjectToScene(World& world, objid sceneId, std::string serializedObj, objid id, bool useObjId, SysInterface interface){
  ParsedContent content = parseFormat(serializedObj);
  assert(content.layers.at(0).name == "default");   // TODO probably should allow the layer to actually be specified but ok for now

  auto serialAttrs = deserializeSceneTokens(content.tokens);

  std::cout << "SERIAL ATTR SIZE: " << serialAttrs.size() << std::endl;
  if (serialAttrs.size() > 1){
    std::cout << "SERIALIZATION GOT MORE THAN 1 OBJECT.  Either bad data or has child element" << std::endl;
  }
  assert(serialAttrs.size() == 1);
  
  auto name = serialAttrs.begin() -> first;
  GameobjAttributes& attrObj = serialAttrs.begin() -> second;

  std::vector<objid> idsAdded;
  auto idToAdd = useObjId ? id : getUniqueObjId();
  idsAdded.push_back(idToAdd);

  auto gameobj = gameObjectFromFields(name, content.layers.at(0).name, idToAdd, attrObj);
  return addSerialObject(world, sceneId, name, interface, attrObj.additionalFields, attrObj.children, gameobj, idsAdded);
}

std::map<std::string, std::string> getAttributes(World& world, objid id){
  // @TODO handle types better
  std::map<std::string, std::string> attr;
  auto objAttrs = objectAttributes(world.objectMapping, id);

  std::map<std::string, std::string> sceneAttrs;
  auto gameobj = getGameObject(world, id); 

  // todo missing physics   TODO I should expose the real types here not strings
  sceneAttrs["position"] = print(gameobj.transformation.position);
  sceneAttrs["scale"] = print(gameobj.transformation.scale);
  sceneAttrs["rotation"] = print(gameobj.transformation.rotation);

  if (gameobj.lookat != ""){
    sceneAttrs["lookat"] = gameobj.lookat;
  }
  if (gameobj.layer != ""){
    sceneAttrs["layer"] = gameobj.layer;
  }
  if (gameobj.script != ""){
    sceneAttrs["script"] = gameobj.script;
  }
  if (gameobj.fragshader != ""){
    sceneAttrs["fragshader"] = gameobj.fragshader;
  }

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
  
  auto attributes = extractAttributes(attr, { "position", "scale", "rotation", "lookat", "layer", "script" });
  GameObject& obj = getGameObject(world, id);

  if (attributes.find("position") != attributes.end()){
    obj.transformation.position = parseVec(attributes.at("position"));
  }
  if (attributes.find("scale") != attributes.end()){
    obj.transformation.scale = parseVec(attributes.at("scale"));
  }
}

void physicsTranslateSet(World& world, objid index, glm::vec3 pos){
  getGameObject(world, index).transformation.position = pos;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setPosition(body, pos);
  }
  world.entitiesToUpdate.insert(index);
}
void physicsTranslate(World& world, objid index, float x, float y, float z, bool moveRelativeEnabled){
  const int SPEED = 5;
  auto offset = glm::vec3(x * SPEED, y * SPEED, z * SPEED);

  glm::vec3 newPosition;
  if (moveRelativeEnabled){
    auto oldGameObject = getGameObject(world, index);
    newPosition = moveRelative(oldGameObject.transformation.position, oldGameObject.transformation.rotation, offset, false);
  }else{
    newPosition = move(getGameObject(world, index).transformation.position, offset);   
  }
  physicsTranslateSet(world, index, newPosition);
}
void applyPhysicsTranslation(World& world, objid index, glm::vec3 position, float offsetX, float offsetY, Axis manipulatorAxis){
  physicsTranslateSet(world, index, applyTranslation(position, offsetX, offsetY, manipulatorAxis));
}

void physicsRotateSet(World& world, objid index, glm::quat rotation){
  getGameObject(world, index).transformation.rotation = rotation;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setRotation(body, rotation);
  }
  world.entitiesToUpdate.insert(index);
}
void physicsRotate(World& world, objid index, float x, float y, float z){
  physicsRotateSet(world, index, setFrontDelta(getGameObject(world, index).transformation.rotation, x, y, z, 5));
}
void applyPhysicsRotation(World& world, objid index, glm::quat currentOrientation, float offsetX, float offsetY, Axis manipulatorAxis){
  physicsRotateSet(world, index, applyRotation(currentOrientation, offsetX, offsetY, manipulatorAxis));
}

void physicsScaleSet(World& world, objid index, glm::vec3 scale){
  Scene& scene = sceneForId(world, index);
  getGameObject(scene, index).transformation.scale = scale;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto collisionInfo = getPhysicsInfoForGameObject(world, scene, index).collisionInfo;
    auto body =  world.rigidbodys.at(index);
    setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
  }
  world.entitiesToUpdate.insert(index);
}
void physicsScale(World& world, objid index, float x, float y, float z){
  auto oldScale = getGameObject(world, index).transformation.scale;
  physicsScaleSet(world, index, glm::vec3(oldScale.x + x, oldScale.y + y, oldScale.z + z));
}
void applyPhysicsScaling(World& world, objid index, glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, Axis manipulatorAxis){
  physicsScaleSet(world, index, applyScaling(position, initialScale, lastX, lastY, offsetX, offsetY, manipulatorAxis));
}

void updatePhysicsPositionsAndClampVelocity(World& world, std::map<objid, btRigidBody*>& rigidbodys){
  for (auto [i, rigidBody]: rigidbodys){
    GameObject& gameobj = getGameObject(world, i);
    // @TODO - physics bug -  getPosition/Rotatin is in world space, need to translate this back relative to parent
    gameobj.transformation.rotation = getRotation(rigidBody);   
    gameobj.transformation.position = getPosition(rigidBody);
    clampMaxVelocity(rigidBody, gameobj.physicsOptions.maxspeed);
  }
}

void updateSoundPositions(World& world){
  forEveryGameobj(world, [&world](objid id, Scene& scene, GameObject& gameobj) -> void {
    updatePosition(world.objectMapping, id, gameobj.transformation.position);
  });
}

void enforceLookAt(World& world){
  forEveryGameobj(world, [&world](objid id, Scene& scene, GameObject& gameobj) -> void {
    std::string lookAt = gameobj.lookat;                      
    if (lookAt == "" || lookAt == gameobj.name){
      return;
    }
    if(scene.nameToId.find(lookAt) != scene.nameToId.end()){
      glm::vec3 fromPos = gameobj.transformation.position;
      glm::vec3 targetPosition = getGameObject(scene, lookAt).transformation.position;
      physicsRotateSet(world, id, orientationFromPos(fromPos, targetPosition));
    }
  });  
}

void callbackEntities(World& world){
  forEveryGameobj(world, [&world](objid id, Scene& scene, GameObject& gameobj) -> void {
    if (id == getGroupId(scene, id) && world.entitiesToUpdate.find(id) != world.entitiesToUpdate.end()){
      world.onObjectUpdate(gameobj);
    }
  });  
  world.entitiesToUpdate.clear();
}

// TODO generalize this function
void updateAttributeDelta(World& world, objid id, std::string attribute, AttributeValue delta){
  //std::cout << "Update particle diff: (" << attribute << ") - " << v << std::endl;
  GameObject& gameobj = getGameObject(world, id);
  applyAttribute(gameobj, attribute, delta);
  physicsTranslateSet(world, id, gameobj.transformation.position);
  physicsScaleSet(world, id, gameobj.transformation.scale);
}

void onWorldFrame(World& world, float timestep, float timeElapsed,  bool enablePhysics, bool dumpPhysics, SysInterface interface){
  updateEmitters(
    world.emitters, 
    timeElapsed, 
    [&world, &interface](std::string name, GameobjAttributes attributes, objid emitterNodeId) -> objid {      
      std::cout << "INFO: emitter: creating particle from emitter: " << name << std::endl;
      attributes.vecAttributes["position"] = fullTransformation(world, emitterNodeId).position;
      objid objectAdded = addObjectToScene(
        world, world.idToScene.at(getGameObject(world, name).id), getUniqueObjectName(), attributes, interface
      );
      return objectAdded;
    }, 
    [&world, &interface](objid id) -> void { 
      std::cout << "INFO: emitter: removing particle from emitter: " << id << std::endl;
      removeObjectFromScene(world, id, interface);
    },
    [&world](objid id, std::string attribute, AttributeValue delta)  -> void {
      updateAttributeDelta(world, id, attribute, delta);
    }
  );  

  if (enablePhysics){
    if (dumpPhysics){
      dumpPhysicsInfo(world.rigidbodys);
    }
    stepPhysicsSimulation(world.physicsEnvironment, timestep);
    updatePhysicsPositionsAndClampVelocity(world, world.rigidbodys);     
  }
  updateSoundPositions(world);
  enforceLookAt(world);   // probably should have physicsTranslateSet, so might be broken
  callbackEntities(world);

  onObjectFrame(world.objectMapping, [&world](std::string texturepath, unsigned char* data, int textureWidth, int textureHeight) -> void {
    updateTextureDataWorld(world, texturepath, data, textureWidth, textureHeight);
  });
}

std::vector<HitObject> raycast(World& world, glm::vec3 posFrom, glm::quat direction, float maxDistance){
  return raycast(world.physicsEnvironment, world.rigidbodys, posFrom, direction, maxDistance);
}

void traverseScene(World& world, Scene& scene, glm::mat4 initialModel, glm::vec3 scale, std::function<void(objid, glm::mat4, glm::mat4, bool, std::string, glm::vec3)> onObject){
  traverseScene(scene, initialModel, scale, onObject, [&world, &scene, &onObject](objid id, glm::mat4 modelMatrix, glm::vec3 scale) -> void {
      Scene& linkScene = world.scenes.at(world.idToScene.at(id));
      traverseScene(world, linkScene, modelMatrix, scale, onObject);
  });
}

void traverseScene(World& world, Scene& scene, std::function<void(objid, glm::mat4, glm::mat4, bool, std::string, glm::vec3)> onObject){
  traverseScene(world, scene, glm::mat4(1.f), glm::vec3(1.f, 1.f, 1.f), onObject);
}

Transformation fullTransformation(World& world, objid id){
  Scene& scene = sceneForId(world, id);
  while(scene.isNested){
    scene = sceneForId(world, parentId(scene, scene.rootId));
  }
  
  Transformation transformation = {};
  bool foundId = false;
  
  traverseScene(world, scene, [id, &foundId, &transformation](objid traversedId, glm::mat4 model, glm::mat4 parent, bool isOrtho, std::string fragshader, glm::vec3 tint) -> void {
    if (traversedId == id){
      foundId = true;
      transformation = getTransformationFromMatrix(model);
    }
  });
  assert(foundId);
  return transformation;
}

std::vector<LightInfo> getLightInfo(World& world){
  auto lightsIndexs = getGameObjectsIndex<GameObjectLight>(world.objectMapping);
  std::vector<LightInfo> lights;
  for (int i = 0; i < lightsIndexs.size(); i++){
    auto objectId =  lightsIndexs.at(i);
    auto objectLight = world.objectMapping.at(objectId);
    auto lightObject = std::get_if<GameObjectLight>(&objectLight);

    auto lightTransform = fullTransformation(world, objectId);
    LightInfo light {
      .pos = lightTransform.position,
      .rotation = lightTransform.rotation,
      .light = *lightObject,
    };
    lights.push_back(light);
  }
  return lights;
}

PortalInfo getPortalInfo(World& world, objid id){
  auto objectPortal = world.objectMapping.at(id);
  auto portalObject = std::get_if<GameObjectPortal>(&objectPortal);
  auto transform = getGameObject(world, portalObject -> camera).transformation;
  auto portalGameObject = getGameObject(world, id);

  PortalInfo info {
    .cameraPos = transform.position,
    .cameraRotation = transform.rotation,
    .portalPos = portalGameObject.transformation.position,
    .portalRotation = portalGameObject.transformation.rotation,
    .perspective = portalObject -> perspective,
    .id = id
  };
  return info;
}

std::vector<PortalInfo> getPortalInfo(World& world){ 
  auto portalIndexes = getGameObjectsIndex<GameObjectPortal>(world.objectMapping);
  std::vector<PortalInfo> portals;
  for (int i = 0; i < portalIndexes.size(); i++){
    portals.push_back(getPortalInfo(world, portalIndexes.at(i)));
  }
  return portals;
}

bool isPortal(World& world, objid id){
  auto objectPortal = world.objectMapping.at(id);
  auto portalObject = std::get_if<GameObjectPortal>(&objectPortal);
  return portalObject != NULL;
}

std::optional<Texture> textureForId(World& world, objid id){
  return textureForId(world.objectMapping, id);
}

float maskValues[] { 10.f };
HeightmapMask mask {
  .values = maskValues,
  .width = 1,
  .height = 1,
};

void applyHeightmapMasking(World& world, objid id, float amount){
  auto heightmaps = getHeightmaps(world.objectMapping);
  if (heightmaps.find(id) == heightmaps.end()){
    return;
  }
  GameObjectHeightmap& hm = *heightmaps.at(id);
  applyMasking(hm.heightmap, hm.heightmap.width / 2, hm.heightmap.height / 2, mask, amount, [&world, id]() -> void { 
      // We change *data fed to bullet.
      // This can be dynamic, however according to docs min + maxHeight must fall in range. 
      // Recreating simply ensures that the min/max height is always valid. 
    updatePhysicsBody(world, sceneForId(world, id), id); 
  }, hm.mesh);
}

glm::vec3 aiNavigate(World& world, objid id, glm::vec3 target){
  NavGraph  navgraph { };

  bool found = false;
  for (auto &[_, obj] : world.objectMapping){
    auto navConn = std::get_if<GameObjectNavConns>(&obj);
    if (navConn != NULL){
      navgraph = navConn -> navgraph;
      found = true;
    }
  }
  assert(found);

  auto getName = [&world](objid id) -> std::string {
    return getGameObject(world, id).name;
  };
  auto raycastWorld = [&world] (glm::vec3 posFrom, glm::quat direction, float maxDistance) -> std::vector<HitObject> {
    return raycast(world, posFrom, direction, maxDistance);
  };
  auto isNavmeshWorld = [&world](objid id) -> bool{ 
    return isNavmesh(world.objectMapping, id);
  };
  auto position = [&world](objid id) -> glm::vec3 {
    return fullTransformation(world, id).position;
  };

  auto currentMesh = targetNavmesh(position(id), raycastWorld, isNavmeshWorld, getName);
  auto destinationMesh = targetNavmesh(target, raycastWorld, isNavmeshWorld, getName);
  if (currentMesh != destinationMesh){
    auto searchResult = aiNavSearchPath(navgraph, currentMesh, destinationMesh);
    if (!searchResult.found || searchResult.path.size() < 2){
      return position(id);
    }
    auto targetNav = searchResult.path.at(1);
    auto targetLink = aiTargetLink(navgraph, currentMesh, targetNav);
    return aiNavPosition(id, targetLink, position, raycastWorld, isNavmeshWorld);
  }
  return aiNavPosition(id, target, position, raycastWorld, isNavmeshWorld);
}