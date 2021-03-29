#include "./scene.h"

GameObject& getGameObject(World& world, objid id){
  return getGameObject(sceneForId(world.sandbox, id), id);
}

std::optional<objid> getGameObjectByName(World& world, std::string name){
  auto obj = maybeGetGameObjectByName(world.sandbox, name);
  if (obj.has_value()){
    return obj.value() -> id;
  }
  return std::nullopt;
}
GameObject& getGameObject(World& world, std::string name){
  auto obj = maybeGetGameObjectByName(world.sandbox, name);
  if (obj.has_value()){
    return *obj.value();
  }
  std::cout << "gameobject : " << name << " does not exist" << std::endl;
  assert(false);
}

NameAndMesh getMeshesForGroupId(World& world, objid groupId){
  std::vector<std::reference_wrapper<std::string>> meshNames;
  std::vector<std::reference_wrapper<Mesh>> meshes;
  NameAndMesh nameAndMeshes = {
    .meshNames = meshNames,
    .meshes = meshes
  };
  for (auto id : getIdsInGroup(world.sandbox, groupId)){
    auto meshesForId = getMeshesForId(world.objectMapping, id);
    for (int i = 0; i < meshesForId.meshes.size(); i++){
      nameAndMeshes.meshNames.push_back(meshesForId.meshNames.at(i));
      nameAndMeshes.meshes.push_back(meshesForId.meshes.at(i));
    }    
  }
  return nameAndMeshes;
}

PhysicsInfo getPhysicsInfoForGameObject(World& world, objid index){  
  GameObject obj = getGameObject(world.sandbox, index);
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
    for (Mesh& mesh : meshes){
      boundInfos.push_back(mesh.boundInfo);
    }
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

GroupPhysicsInfo getPhysicsInfoForGroup(World& world, objid id){
  auto groupId = getGroupId(world.sandbox, id);
  bool isRoot = groupId == id;
  if (!isRoot){
    return GroupPhysicsInfo {
      .isRoot = isRoot,
    };
  }

  GroupPhysicsInfo groupInfo {
    .isRoot = isRoot,
    .physicsInfo = getPhysicsInfoForGameObject(world, groupId),
    .physicsOptions = getGameObject(world.sandbox, groupId).physicsOptions,
  };  
  return groupInfo;
}


// TODO - physics bug - physicsOptions location/rotation/scale is not relative to parent 
void addPhysicsBody(World& world, objid id, glm::vec3 initialScale){
  auto groupPhysicsInfo = getPhysicsInfoForGroup(world, id);
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
      .layer = physicsOptions.layer,
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

void updatePhysicsBody(World& world, objid id){
  auto rigidBody = world.rigidbodys.at(id);
  assert(rigidBody != NULL);
  glm::vec3 oldScale = getScale(rigidBody);
  rmRigidBody(world, id);
  addPhysicsBody(world, id, oldScale);
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
  ModelData data = loadModel("", meshpath);
  assert(data.meshIdToMeshData.size() ==  1);
  auto meshData = data.meshIdToMeshData.begin() -> second;
  world.meshes[meshpath] =  loadMesh("./res/textures/default.jpg", meshData, [&world](std::string texture) -> Texture {
    return loadTextureWorld(world, texture);
  });     
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
    .sandbox = createSceneSandbox(),
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
  std::map<std::string, std::string> additionalFields
){
    auto id =  scene.nameToId.at(name);

    if (idExists(world.sandbox, id)){
      std::cout << "id already in the scene: " << id << std::endl;
      assert(false);
    }

    world.sandbox.idToScene[id] = sceneId;
    auto localSceneId = sceneId;

    addObject(id, getType(name, fields), additionalFields, world.objectMapping, world.meshes, "./res/models/ui/node.obj",
      [&world, &scene, sceneId, id, name, shouldLoadModel, getId, &additionalFields, &interface](std::string meshName, std::vector<std::string> fieldsToCopy) -> bool {  // This is a weird function, it might be better considered "ensure model l"
        if (shouldLoadModel){
          ModelData data = loadModel(name, meshName); 
          world.animations[id] = data.animations;
          bool hasMesh = data.nodeToMeshId.at(0).size() > 0;     // this is 0 node because we just loaded a mesh, so by definition is root node

          for (auto [meshId, meshData] : data.meshIdToMeshData){
            auto meshPath = meshName + "::" + std::to_string(meshId);
            bool meshAlreadyLoaded = !(world.meshes.find(meshPath) == world.meshes.end());
            if (meshAlreadyLoaded){
              continue;
            }
            world.meshes[meshPath] = loadMesh("./res/textures/wood.jpg", meshData, [&world](std::string texture) -> Texture {
              return loadTextureWorld(world, texture);
            });    
          } 

          auto newSerialObjs = multiObjAdd(
            world.sandbox,
            id,
            0,
            data.childToParent, 
            data.nodeTransform, 
            data.names, 
            generateAdditionalFields(meshName, data, additionalFields, fieldsToCopy),
            getId
          );

          for (auto &[name, additionalFields] : newSerialObjs){
            addObjectToWorld(world, scene, sceneId, name, false, getId, interface, additionalFields);
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
        updatePhysicsBody(world, id);
      },
      [&world, id, &interface](std::string sceneToLoad) -> void {
        std::cout << "INFO: -- SCENE LOADING : " << sceneToLoad << std::endl;
        addLink(world.sandbox, addSceneToWorld(world, sceneToLoad, interface), id);
      },
      [&world, &interface, name, id](float spawnrate, float lifetime, int limit, std::map<std::string, std::string> particleFields, std::vector<EmitterDelta> deltas, bool enabled) -> void {
        addEmitter(world.emitters, name, id, interface.getCurrentTime(), limit, spawnrate, lifetime, fieldsToAttributes(particleFields), deltas, enabled);
      },
      [&world](MeshData& meshdata) -> Mesh {
        return loadMesh("./res/textures/default.jpg", meshdata, [&world](std::string texture) -> Texture {
          return loadTextureWorld(world, texture);
        });    
      }
    );
}

std::string getTextureById(World& world, int id){
  for (auto &[textureName, texture] : world.textures){
    if (texture.textureId == id){
      return textureName;
    }
  }
  std::cout << "TEXTURE : lookup: could not find texture with id: " << id << std::endl;
  assert(false);
  return "";
}

std::string serializeScene(World& world, objid sceneId, bool includeIds){
  Scene& scene = world.sandbox.scenes.at(sceneId);
  return serializeScene(scene, [&world](objid objectId)-> std::vector<std::pair<std::string, std::string>> {
    return getAdditionalFields(objectId, world.objectMapping, [&world](int textureId) -> std::string {
      return getTextureById(world, textureId);
    });
  }, includeIds);
} 

std::string serializeObject(World& world, objid id, std::string overridename){
  Scene& scene = sceneForId(world.sandbox, id);
  return serializeObject(
    scene, 
    id, 
    [&world](objid objectId)-> std::vector<std::pair<std::string, std::string>> {
      return getAdditionalFields(objectId, world.objectMapping, [&world](int textureId) -> std::string {
        return getTextureById(world, textureId);
      });
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
  std::map<std::string, std::map<std::string, std::string>> additionalFields
){
  for (auto &[name, additionalField] : additionalFields){
    // Warning: getNewObjectId will mutate the idsAdded.  
    addObjectToWorld(world, world.sandbox.scenes.at(sceneId), sceneId, name, true, getNewObjectId, interface, additionalField);
  }
  for (auto id : idsAdded){
    addPhysicsBody(world, id, glm::vec3(1.f, 1.f, 1.f));   
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
  auto data = addSceneDataToScenebox(world.sandbox, sceneId, sceneData, interface.layers);
  addSerialObjectsToWorld(world, sceneId, data.idsAdded, getUniqueObjId, interface, data.deserializedScene.additionalFields);
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

  addGameObjectToScene(world.sandbox.scenes.at(sceneId), name, gameobjectObj, children);

  std::map<std::string, std::map<std::string, std::string>> additionalFieldsMap;
  additionalFieldsMap[name] = additionalFields;

  addSerialObjectsToWorld(world, sceneId, idsAdded, getId, interface, additionalFieldsMap);

  auto gameobjId = world.sandbox.scenes.at(sceneId).nameToId.at(name);
  return gameobjId;
}

objid addSceneToWorld(World& world, std::string sceneFile, SysInterface interface){
  return addSceneToWorldFromData(world, getUniqueObjId(), loadFile(sceneFile), interface);
}

// todo finish removing data like eg clearing meshes, animations,etc
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
  interface.stopAnimation(objectId);
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
  
  world.sandbox.idToScene.erase(objectId);
  world.onObjectDelete(objectId, netsynchronized);

  // @TODO IMPORTANT : remove free meshes (no way to tell currently if free -> need counting probably) from meshes
  std::cout << "TODO: MESH MANAGEMENT HORRIBLE NEED TO REMOVE AND NOT BE DUMB ABOUT LOADING THEM" << std::endl;
}
// this needs to also delete all children objects. 
void removeObjectFromScene(World& world, objid objectId, SysInterface interface){  
  Scene& scene = sceneForId(world.sandbox, objectId);
  for (auto gameobjId : getIdsInGroup(world.sandbox, objectId)){
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
  addObjectToScene(world, world.sandbox.idToScene.at(id), serializedObject, -1, false, interface);
}


void removeSceneFromWorld(World& world, objid sceneId, SysInterface interface){
  if (world.sandbox.scenes.find(sceneId) == world.sandbox.scenes.end()) {
    std::cout << "INFO: SCENE MANAGEMENT: tried to remove (" << sceneId << ") but it does not exist" << std::endl;
    return;   // @todo maybe better to throw error instead
  }

  Scene& scene = world.sandbox.scenes.at(sceneId);
  for (auto objectId : listObjInScene(scene)){
    auto gameobj = getGameObject(world, objectId);
    auto name = gameobj.name;
    auto scriptName = gameobj.script;
    auto netsynchronized = gameobj.netsynchronize;
    removeObjectById(world, objectId, name, interface, scriptName, netsynchronized);
  }
  world.sandbox.scenes.erase(sceneId);
}
void removeAllScenesFromWorld(World& world, SysInterface interface){
  for (auto sceneId : allSceneIds(world.sandbox)){
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
  auto gameobjectObj = gameObjectFromFields(name, idToAdd, attributes);
  return addSerialObject(world, sceneId, name, interface, attributes.additionalFields, attributes.children, gameobjectObj, idsAdded);
}

objid addObjectToScene(World& world, objid sceneId, std::string serializedObj, objid id, bool useObjId, SysInterface interface){
  auto tokens = parseFormat(serializedObj);
  auto serialAttrs = deserializeSceneTokens(tokens);

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

  auto gameobj = gameObjectFromFields(name, idToAdd, attrObj);
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
    if (attr.find(attribute) != attr.end()){ // but why don't i pass all of them in?  this seems dumb
      attrToSet[attribute] = attr.at(attribute);
    }    
  }
  return attrToSet;
}

void setAttributes(World& world, objid id, std::map<std::string, std::string> attr){
  // @TODO create complete lists for attributes. 
  // look toward applyattribute delta 
  setObjectAttributes(
    world.objectMapping, 
    id, 
    extractAttributes(attr, { "mesh", "isDisabled", "clip", "from", "to", "color", "state" }),
    [&world, id](bool enabled) -> void {
      std::cout << "id: " << id << " should be enabled: " << enabled << std::endl;
      setEmitterEnabled(world.emitters, id, enabled);
    }
  );
  
  auto attributes = extractAttributes(attr, { "position", "scale", "rotation", "lookat", "layer", "script" });
  GameObject& obj = getGameObject(world, id);

  if (attributes.find("position") != attributes.end()){
    obj.transformation.position = parseVec(attributes.at("position"));
  }
  if (attributes.find("scale") != attributes.end()){
    obj.transformation.scale = parseVec(attributes.at("scale"));
  }
}
void setProperty(World& world, objid id, std::vector<Property>& properties){
  for (auto property : properties){
    if (property.propertyName == "position"){
      auto posV = std::get_if<glm::vec3>(&property.value);
      if(posV != NULL){
        getGameObject(world, id).transformation.position = *posV;
      }
    }
  }
}

// property suffix looks like the parts of the tokens on the right hand side
// eg position 10
// eg tint 0.9 0.2 0.4
AttributeValue parsePropertySuffix(std::string key, std::string value){
  if (key == "position" || key == "scale"){
    return parseVec(value);
  }
  return value;
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
  Scene& scene = sceneForId(world.sandbox, index);
  getGameObject(scene, index).transformation.scale = scale;

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto collisionInfo = getPhysicsInfoForGameObject(world, index).collisionInfo;
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
  forEveryGameobj(world.sandbox, [&world](objid id, Scene& scene, GameObject& gameobj) -> void {
    updatePosition(world.objectMapping, id, gameobj.transformation.position);
  });
}

void enforceLookAt(World& world){
  forEveryGameobj(world.sandbox, [&world](objid id, Scene& scene, GameObject& gameobj) -> void {
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
  forEveryGameobj(world.sandbox, [&world](objid id, Scene& scene, GameObject& gameobj) -> void {
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
      attributes.vecAttributes["position"] = fullTransformation(world.sandbox, emitterNodeId).position;
      objid objectAdded = addObjectToScene(
        world, world.sandbox.idToScene.at(getGameObject(world, name).id), getUniqueObjectName(), attributes, interface
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
  }, timeElapsed);
}
