#include "./scene.h"

GameObject& getGameObject(World& world, objid id){
  return getGameObject(world.sandbox, id);
}

std::optional<objid> getGameObjectByName(World& world, std::string name, objid sceneId){
  auto obj = maybeGetGameObjectByName(world.sandbox, name, sceneId);
  if (obj.has_value()){
    return obj.value() -> id;
  }
  return std::nullopt;
}
GameObject& getGameObject(World& world, std::string name, objid sceneId){
  auto obj = maybeGetGameObjectByName(world.sandbox, name, sceneId);
  if (obj.has_value()){
    return *obj.value();
  }
  std::cout << "gameobject : " << name << " does not exist" << std::endl;
  assert(false);
}

NameAndMeshObjName getMeshesForGroupId(World& world, objid groupId){
  std::vector<std::string> objnames;
  std::vector<std::reference_wrapper<std::string>> meshNames;
  std::vector<std::reference_wrapper<Mesh>> meshes;
  NameAndMeshObjName nameAndMeshes = {
    .objnames = objnames,
    .meshNames = meshNames,
    .meshes = meshes
  };
  for (auto id : getIdsInGroup(world.sandbox, groupId)){
    auto meshesForId = getMeshesForId(world.objectMapping, id);
    auto gameobjname = getGameObject(world, id).name;
    for (int i = 0; i < meshesForId.meshes.size(); i++){
      nameAndMeshes.objnames.push_back(gameobjname);
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
  };

  glm::vec3 offset(0.f, 0.f, 0.f);
  auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); 
  if (meshObj != NULL){
    std::vector<BoundInfo> boundInfos;
    auto meshes = getMeshesForGroupId(world, index).meshes;
    for (Mesh& mesh : meshes){
      boundInfos.push_back(mesh.boundInfo);
    }
    if (boundInfos.size() > 0){
      boundInfo = getMaxUnionBoundingInfo(boundInfos);
    }else{
      boundInfo = BoundInfo {  // maybe this should be 0 size, or not have a bound info? hmm
        .xMin = -0.5, .xMax = 0.5,
        .yMin = -0.5, .yMax = 0.5,
        .zMin = -0.5, .zMax = 0.5,
      };
    }
  }

  auto voxelObj = std::get_if<GameObjectVoxel>(&gameObjV);
  if (voxelObj != NULL){
    boundInfo = voxelObj -> voxel.boundInfo;
  }

  auto navmeshObj = std::get_if<GameObjectNavmesh>(&gameObjV);
  if (navmeshObj != NULL){
    boundInfo = navmeshObj -> mesh.boundInfo;
  }

  auto uiButtonObj = std::get_if<GameObjectUIButton>(&gameObjV);
  if (uiButtonObj != NULL){
    boundInfo = uiButtonObj -> common.mesh.boundInfo;
  }

  auto uiSliderObj = std::get_if<GameObjectUISlider>(&gameObjV);
  if (uiSliderObj != NULL){
    boundInfo = uiSliderObj -> common.mesh.boundInfo;
  }

  auto layoutObj = std::get_if<GameObjectUILayout>(&gameObjV);
  if (layoutObj != NULL){
    boundInfo = layoutObj -> boundInfo;
  }

  auto textObj = std::get_if<GameObjectUIText>(&gameObjV);
  if (textObj != NULL){
    boundInfo = boundInfoForCenteredText(textObj -> value, 1, textObj -> deltaOffset, textObj -> align, textObj -> wrap, textObj -> virtualization, &offset); 
  }

  PhysicsInfo info = {
    .boundInfo = boundInfo,
    .transformation = obj.transformation,
    .offset = offset,
  };

  return info;
}

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
PhysicsValue addPhysicsBody(World& world, objid id, glm::vec3 initialScale, bool initialLoad, std::vector<glm::vec3> verts){
  auto groupPhysicsInfo = getPhysicsInfoForGroup(world, id);
  if (!groupPhysicsInfo.physicsOptions.enabled){
    return PhysicsValue { .body = NULL, .offset = glm::vec3(0.f, 0.f, 0.f) };
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
      .velocity = physicsOptions.velocity,
      .angularVelocity = physicsOptions.angularVelocity,
    };

    if (isHeightmapObj){
      rigidBody = addRigidBodyHeightmap(
        world.physicsEnvironment,
        physicsInfo.transformation.position + groupPhysicsInfo.physicsInfo.offset,
        physicsInfo.transformation.rotation,
        physicsOptions.isStatic,
        opts,
        heightmapObj -> heightmap.data,
        heightmapObj -> heightmap.width, 
        heightmapObj -> heightmap.height,
        physicsInfo.transformation.scale,
        heightmapObj -> heightmap.minHeight,
        heightmapObj -> heightmap.maxHeight
      );
    }else if (physicsOptions.shape == BOX || (!isVoxelObj && physicsOptions.shape == AUTOSHAPE)){
      std::cout << "INFO: PHYSICS: ADDING BOX RIGID BODY (" << id << ")" << std::endl;
      rigidBody = addRigidBodyRect(
        world.physicsEnvironment, 
        physicsInfo.transformation.position + groupPhysicsInfo.physicsInfo.offset, 
        physicsInfo.boundInfo.xMax - physicsInfo.boundInfo.xMin, 
        physicsInfo.boundInfo.yMax - physicsInfo.boundInfo.yMin, 
        physicsInfo.boundInfo.zMax - physicsInfo.boundInfo.zMin,
        physicsInfo.transformation.rotation,
        physicsOptions.isStatic,
        physicsOptions.hasCollisions,
        physicsInfo.transformation.scale, 
        opts
      );
    }else if (physicsOptions.shape == SPHERE){
      std::cout << "INFO: PHYSICS: ADDING SPHERE RIGID BODY" << std::endl;
      rigidBody = addRigidBodySphere(
        world.physicsEnvironment, 
        physicsInfo.transformation.position + groupPhysicsInfo.physicsInfo.offset,
        maxvalue(
          (physicsInfo.boundInfo.xMax - physicsInfo.boundInfo.xMin), 
          (physicsInfo.boundInfo.yMax - physicsInfo.boundInfo.yMin) , 
          (physicsInfo.boundInfo.zMax - physicsInfo.boundInfo.zMin)
        ) / 2.f,                             
        physicsInfo.transformation.rotation,
        physicsOptions.isStatic,
        physicsOptions.hasCollisions,
        physicsInfo.transformation.scale,
        opts
      );
    }else if (physicsOptions.shape == CAPSULE){
      std::cout << "INFO: PHYSICS: ADDING CAPSULE RIGID BODY" << std::endl;
      rigidBody = addRigidBodyCapsule(
        world.physicsEnvironment,
        maxvalue(
          (physicsInfo.boundInfo.xMax - physicsInfo.boundInfo.xMin), 
          0.f,
          (physicsInfo.boundInfo.zMax - physicsInfo.boundInfo.zMin)
        ) / 2.f,
        (physicsInfo.boundInfo.yMax - physicsInfo.boundInfo.yMin) ,
        physicsInfo.transformation.position + groupPhysicsInfo.physicsInfo.offset,
        physicsInfo.transformation.rotation,
        physicsOptions.isStatic,
        physicsOptions.hasCollisions,
        physicsInfo.transformation.scale,
        opts
      );
    }else if (physicsOptions.shape == CYLINDER){
      std::cout << "INFO: PHYSICS: ADDING CYLINDER RIGID BODY" << std::endl;
      rigidBody = addRigidBodyCylinder(
        world.physicsEnvironment,
        maxvalue(
          (physicsInfo.boundInfo.xMax - physicsInfo.boundInfo.xMin), 
          0.f,
          (physicsInfo.boundInfo.zMax - physicsInfo.boundInfo.zMin)
        ) / 2.f,
        (physicsInfo.boundInfo.yMax - physicsInfo.boundInfo.yMin),
        physicsInfo.transformation.position + groupPhysicsInfo.physicsInfo.offset,
        physicsInfo.transformation.rotation,
        physicsOptions.isStatic,
        physicsOptions.hasCollisions,
        physicsInfo.transformation.scale,
        opts
      );
    }else if (physicsOptions.shape == CONVEXHULL){
      // This is a hack, but it should be ok.  UpdatePhysicsBody really only need to apply for voxels and heightmaps as of writing this
      // I don't have easy scope to the list of verts here, so I'd rather not reload the model (or really keep them in mem for no reason) just 
      // for this, which is unused.  Probably should just change the usage of the voxel/heightmap refresh code eventually.
      assert(initialLoad);  
      std::cout << "INFO: PHYSICS: ADDING CONVEXHULL RIGID BODY" << std::endl;
      rigidBody = addRigidBodyHull(
        world.physicsEnvironment,
        verts,
        physicsInfo.transformation.position + groupPhysicsInfo.physicsInfo.offset,
        physicsInfo.transformation.rotation,
        physicsOptions.isStatic,
        physicsOptions.hasCollisions,
        physicsInfo.transformation.scale,
        opts
      );
    }else if (physicsOptions.shape == SHAPE_EXACT){
      assert(initialLoad);
      std::cout << "INFO: PHYSICS: ADDING SHAPE_EXACT RIGID BODY" << std::endl;
      rigidBody = addRigidBodyExact(
        world.physicsEnvironment,
        verts,
        physicsInfo.transformation.position + groupPhysicsInfo.physicsInfo.offset,
        physicsInfo.transformation.rotation,
        physicsOptions.isStatic,
        physicsOptions.hasCollisions,
        physicsInfo.transformation.scale,
        opts
      );
    }else if (physicsOptions.shape == AUTOSHAPE && isVoxelObj){
      std::cout << "INFO: PHYSICS: ADDING AUTOSHAPE VOXEL RIGID BODY" << std::endl;
      rigidBody = addRigidBodyVoxel(
        world.physicsEnvironment, 
        physicsInfo.transformation.position + groupPhysicsInfo.physicsInfo.offset,
        physicsInfo.transformation.rotation,
        getVoxelBodies(std::get_if<GameObjectVoxel>(&toRender) -> voxel),
        physicsOptions.isStatic,
        physicsOptions.hasCollisions,
        initialScale,
        opts
      );
    }
  }

  PhysicsValue phys {
    .body = NULL,
    .offset = groupPhysicsInfo.physicsInfo.offset,
  };
  if (rigidBody != NULL){
    phys.body = rigidBody;
    world.rigidbodys[id] = phys;
  }
  return phys;
}
void rmRigidBody(World& world, objid id){
  auto rigidBodyPtr = world.rigidbodys.at(id).body;
  assert(rigidBodyPtr != NULL);
  rmRigidBody(world.physicsEnvironment, rigidBodyPtr);
  world.rigidbodys.erase(id);
}


bool hasPhysicsBody(World& world, objid id){
  return world.rigidbodys.find(id) != world.rigidbodys.end();
}
void updatePhysicsBody(World& world, objid id){
  auto rigidBody = world.rigidbodys.at(id).body;
  assert(rigidBody != NULL);
  glm::vec3 oldScale = getScale(rigidBody);
  rmRigidBody(world, id);
  addPhysicsBody(world, id, oldScale, false, {});
}

Texture loadTextureWorld(World& world, std::string texturepath, objid ownerId){
  if (world.textures.find(texturepath) != world.textures.end()){
    world.textures.at(texturepath).owners.insert(ownerId);
    return world.textures.at(texturepath).texture;
  }
  Texture texture = loadTexture(texturepath);
  world.textures[texturepath] = TextureRef {
    .owners = { ownerId },
    .texture = texture,
  };
  return texture;
}
Texture loadSkyboxWorld(World& world, std::string texturepath, objid ownerId){
  if (world.textures.find(texturepath) != world.textures.end()){
    world.textures.at(texturepath).owners.insert(ownerId);
    return world.textures.at(texturepath).texture;
  }
  Texture texture = loadCubemapTexture(texturepath);
  world.textures[texturepath] = TextureRef {
    .owners = { ownerId },
    .texture = texture,
  };
  return texture;
}

Texture loadTextureDataWorld(World& world, std::string texturepath, unsigned char* data, int textureWidth, int textureHeight, int numChannels, objid ownerId){
  if (world.textures.find(texturepath) != world.textures.end()){
    world.textures.at(texturepath).owners.insert(ownerId);
    return world.textures.at(texturepath).texture;
  }
  Texture texture = loadTextureData(data, textureWidth, textureHeight, numChannels);
  world.textures[texturepath] = TextureRef {
    .owners = { ownerId },
    .texture = texture,
  };
  return texture;  
}
void updateTextureDataWorld(World& world, std::string texturepath, unsigned char* data, int textureWidth, int textureHeight){
  updateTextureData(world.textures.at(texturepath).texture, data, textureWidth, textureHeight);
}

void freeTextureRef(World& world, std::string textureName){
  std::cout << "INFO: freeing texture: " << textureName  << std::endl;
  freeTexture(world.textures.at(textureName).texture);
  world.textures.erase(textureName);
}
void freeTextureRefsByOwner(World& world, int ownerId){
  for (auto &[name, textureRef] : world.textures){
    textureRef.owners.erase(ownerId);
  }
  std::vector<std::string> texturesToFree;
  for (auto &[name, textureRef] : world.textures){
    if (textureRef.owners.size() == 0){
      texturesToFree.push_back(name);
    }
  }
  for (auto textureName : texturesToFree){
    freeTextureRef(world, textureName);
  }  
}

void freeAnimationsForOwner(World& world, objid id){
  std::cout << "INFO: removing animations for: " << id << std::endl;
  world.animations.erase(id);
}

void loadMeshData(World& world, std::string meshPath, MeshData& meshData, int ownerId){
  if (world.meshes.find(meshPath) != world.meshes.end()){
    world.meshes.at(meshPath).owners.insert(ownerId);
  }else{
    world.meshes[meshPath] = MeshRef {
      .owners = { ownerId }, 
      .mesh = loadMesh("./res/textures/default.jpg", meshData, [&world, ownerId](std::string texture) -> Texture {
        return loadTextureWorld(world, texture, ownerId);
      })  
    };
  }
}

void addMesh(World& world, std::string meshpath){
  ModelData data = loadModel("", meshpath);
  if (data.meshIdToMeshData.size() !=  1){
    std::cout << "ERROR: " << meshpath << " actual size: " << data.meshIdToMeshData.size() << std::endl;
    assert(false);
  }
  auto meshData = data.meshIdToMeshData.begin() -> second;
  loadMeshData(world, meshpath, meshData, -1);
  std::cout << "WARNING: add mesh does not load animations, bones for default meshes" << std::endl;
}

void freeMeshRef(World& world, std::string meshname){
  std::cout << "INFO: freeing mesh: " << meshname  << std::endl;
  freeMesh(world.meshes.at(meshname).mesh);
  world.meshes.erase(meshname);
}
void freeMeshRefsByOwner(World& world, int ownerId){
  for (auto &[name, mesh] : world.meshes){
    mesh.owners.erase(ownerId);
  }
  std::vector<std::string> meshesToFree;
  for (auto &[name, mesh] : world.meshes){
    if (mesh.owners.size() == 0){
      meshesToFree.push_back(name);
    }
  }
  for (auto meshname : meshesToFree){
    freeMeshRef(world, meshname);
  }
}

std::function<Mesh(std::string)> getCreateMeshCopy(World& world, std::string rootname){
  return [&world, rootname](std::string meshname) -> Mesh {
    Mesh meshCopy = world.meshes.at(meshname).mesh;
    for (auto &bone : meshCopy.bones){
      auto suffix = bone.name.substr(5, bone.name.size() - 5); // bones all names root:whatever 
      auto newBoneName = rootname + suffix;
      bone.name = newBoneName;
    }
    return meshCopy;
  };
}

void loadSkybox(World& world, std::string skyboxpath){
  if (world.meshes.find("skybox") != world.meshes.end()){
    freeMeshRef(world, "skybox");
  }
  world.meshes["skybox"] = MeshRef {
    .owners = { -1 },
    .mesh = loadSkybox(
      "./res/textures/default.jpg", 
      "./res/models/skybox.obj",
      skyboxpath, 
      [&world](std::string texture) -> Texture {
        return loadTextureWorld(world, texture, -1);
      },
      [&world](std::string texture) -> Texture {
        return loadSkyboxWorld(world, texture, -1);
      }
    )
  };
}

World createWorld(
  collisionPairPosFn onObjectEnter, 
  collisionPairFn onObjectLeave, 
  std::function<void(GameObject&)> onObjectUpdate,  
  std::function<void(GameObject&)> onObjectCreate, 
  std::function<void(objid, bool)> onObjectDelete, 
  btIDebugDraw* debugDrawer,
  std::vector<LayerInfo> layers,
  SysInterface interface,
  std::vector<std::string> defaultMeshes
){
  auto objectMapping = getObjectMapping();
  World world = {
    .physicsEnvironment = initPhysics(onObjectEnter, onObjectLeave, debugDrawer),
    .objectMapping = objectMapping,
    .emitters = EmitterSystem { .emitters = {}, .additionalParticlesToRemove = {} },
    .onObjectUpdate = onObjectUpdate,
    .onObjectCreate = onObjectCreate,
    .onObjectDelete = onObjectDelete,
    .entitiesToUpdate = {},
    .sandbox = createSceneSandbox(layers),
  };

  // hackey, but createSceneSandbox adds root object with id 0 so this is needed
  std::vector<objid> idsAdded = { 0 };
  std::vector<GameObjectObj> addedGameobjObjs = {};
  addSerialObjectsToWorld(world, world.sandbox.mainScene.rootId, idsAdded, getUniqueObjId, interface, {{ "root", GameobjAttributesWithId { .id = idsAdded.at(0), .attr = GameobjAttributes{}}}}, false, addedGameobjObjs);

  // Default meshes that are silently loaded in the background
  for (auto &meshname : defaultMeshes){
    addMesh(world, meshname);
  }

  loadSkybox(world, ""); 

  auto generatedMesh = generateTestMesh();
  loadMeshData(world, "testmesh", generatedMesh, -1);
  return world;
}

std::map<objid, GameobjAttributes> generateAdditionalFields(std::string meshName, ModelData& data, GameobjAttributes& attr, std::vector<std::string> fieldsToCopy){
  std::map<objid, GameobjAttributes> additionalFieldsMap;
  for (auto [nodeId, _] : data.nodeTransform){
    additionalFieldsMap[nodeId] = GameobjAttributes{};
  }

  for (auto [nodeId, meshListIds] : data.nodeToMeshId){
    if (meshListIds.size() == 1){
      auto meshRef = meshName + "::" + std::to_string(meshListIds.at(0));
      additionalFieldsMap.at(nodeId).stringAttributes["mesh"] = meshRef;
    }else if (meshListIds.size() > 1){
      std::vector<std::string> meshRefNames;
      for (auto id : meshListIds){
        auto meshRef = meshName + "::" + std::to_string(id);
        meshRefNames.push_back(meshRef);
      }
      additionalFieldsMap.at(nodeId).stringAttributes["meshes"] = join(meshRefNames, ',');
    }
  }

  for (auto &[_, obj] : additionalFieldsMap){    // @TODO - this looks wrong, shouldn't we copy all fields? 
    for (auto field : fieldsToCopy){
      assert(obj.stringAttributes.find(field) == obj.stringAttributes.end());
      assert(obj.numAttributes.find(field) == obj.numAttributes.end());
      assert(obj.vecAttributes.find(field) == obj.vecAttributes.end());
      if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
        obj.stringAttributes[field] = attr.stringAttributes.at(field);
      }
      if (attr.numAttributes.find(field) != attr.numAttributes.end()){
        obj.numAttributes[field] = attr.numAttributes.at(field);
      }
      if (attr.vecAttributes.find(field) != attr.vecAttributes.end()){
        obj.vecAttributes[field] = attr.vecAttributes.at(field);
      }
    }
  }

  return additionalFieldsMap;
}

std::string getTextureById(World& world, int id){
  for (auto &[textureName, texture] : world.textures){
    if (texture.texture.textureId == id){
      return textureName;
    }
  }
  std::cout << "TEXTURE : lookup: could not find texture with id: " << id << std::endl;
  assert(false);
  return "";
}

std::string serializeScene(World& world, objid sceneId, bool includeIds){
  return serializeScene(world.sandbox, sceneId, [&world](objid objectId)-> std::vector<std::pair<std::string, std::string>> {
    return getAdditionalFields(objectId, world.objectMapping, [&world](int textureId) -> std::string {
      return getTextureById(world, textureId);
    });
  }, includeIds);
} 

std::string serializeObject(World& world, objid id, std::string overridename){
  auto gameobj = getGameObject(world.sandbox, id);
  auto gameobjecth = getGameObjectH(world.sandbox, id);
  auto children = childnames(world.sandbox, gameobjecth);
  auto additionalFields = getAdditionalFields(id, world.objectMapping, [&world](int textureId) -> std::string {
    return getTextureById(world, textureId);
  });
  return serializeObjectSandbox(gameobj, id, gameobjecth.groupId, additionalFields, children, false, overridename);
}

void addObjectToWorld(
  World& world, 
  objid sceneId, 
  objid id,
  std::string name,
  std::string topName,
  std::function<objid()> getId,
  SysInterface interface,
  GameobjAttributes attr,
  std::map<objid, std::vector<glm::vec3>>& idToModelVertexs,
  ModelData* data,
  std::string rootMeshName,
  bool returnObjectOnly,
  std::vector<GameObjectObj>& returnobjs // only added to if returnObjOnly = true
){

    auto loadMeshObject = [&world, id](MeshData& meshdata) -> Mesh {
      return loadMesh("./res/textures/default.jpg", meshdata, [&world, id](std::string texture) -> Texture {
        return loadTextureWorld(world, texture, id);
      });    
    };
    auto addEmitterObject = [&world, &interface, name, id](float spawnrate, float lifetime, int limit, GameobjAttributes& particleFields, std::vector<EmitterDelta> deltas, bool enabled, EmitterDeleteBehavior behavior) -> void {
      addEmitter(world.emitters, name, id, interface.getCurrentTime(), limit, spawnrate, lifetime, particleFields, deltas, enabled, behavior);
    };
    auto onCollisionChange = [&world, id]() -> void {
      //assert(false); // think about what this should do better!
      updatePhysicsBody(world, id);
    };
    auto ensureTextureLoaded = [&world, id](std::string texturepath) -> Texture {
      std::cout << "Custom texture loading: " << texturepath << std::endl;
      return loadTextureWorld(world, texturepath, id);
    };
    auto ensureMeshLoaded = [&world, sceneId, id, name, topName, getId, &attr, &interface, &idToModelVertexs, data, &rootMeshName, returnObjectOnly, &returnobjs](std::string meshName, std::vector<std::string> fieldsToCopy) -> std::vector<std::string> {
      if (meshName == ""){
        return {};
      }
      if (data == NULL){
        ModelData data = loadModel(name, meshName); 
        idToModelVertexs[id] = getVertexsFromModelData(data);
        world.animations[id] = data.animations;

        for (auto [meshId, meshData] : data.meshIdToMeshData){
          auto meshPath = nameForMeshId(meshName, meshId);
          loadMeshData(world, meshPath, meshData, id);
        } 

        auto newSerialObjs = multiObjAdd(
          world.sandbox,
          sceneId,
          id,
          0,
          data.childToParent, 
          data.nodeTransform, 
          data.names, 
          generateAdditionalFields(meshName, data, attr, fieldsToCopy),
          getId
        );

        for (auto &[name, objAttr] : newSerialObjs){
          addObjectToWorld(world, sceneId, objAttr.id, name, topName, getId, interface, objAttr.attr, idToModelVertexs, &data, meshName, returnObjectOnly, returnobjs);
        }
        return meshNamesForNode(data, meshName, name);
      }
      return meshNamesForNode(*data, rootMeshName, name);  
    }; 

    std::cout << "rootname create mesh: " << name << std::endl;
    if (returnObjectOnly){
      ObjectTypeUtil util {
        .id = id,
        .createMeshCopy = getCreateMeshCopy(world, topName),
        .meshes = world.meshes,
        .ensureTextureLoaded = [](std::string) -> Texture { return Texture{}; },
        .loadMesh = [](MeshData&) -> Mesh { return Mesh{}; },
        .addEmitter =  [](float spawnrate, float lifetime, int limit, GameobjAttributes& particleFields, std::vector<EmitterDelta> deltas, bool enabled, EmitterDeleteBehavior behavior) -> void {},
        .ensureMeshLoaded = [](std::string meshName, std::vector<std::string> fieldsToCopy) -> std::vector<std::string> { return {}; },
        .onCollisionChange = []() -> void {},
      }; 
      auto gameobjObj = createObjectType(getType(name), attr, util);
      returnobjs.push_back(gameobjObj);
      return;
    }
    ObjectTypeUtil util {
      .id = id,
      .createMeshCopy = getCreateMeshCopy(world, topName),
      .meshes = world.meshes,
      .ensureTextureLoaded = ensureTextureLoaded,
      .loadMesh = loadMeshObject,
      .addEmitter = addEmitterObject,
      .ensureMeshLoaded = ensureMeshLoaded,
      .onCollisionChange = onCollisionChange,
    };
    auto gameobjObj = createObjectType(getType(name), attr, util);
    addObjectType(world.objectMapping, gameobjObj, id);
}

void addSerialObjectsToWorld(
  World& world, 
  objid sceneId, 
  std::vector<objid>& idsAdded,
  std::function<objid()> getNewObjectId,
  SysInterface interface,
  std::map<std::string, GameobjAttributesWithId> nameToAttr,
  bool returnObjectOnly,
  std::vector<GameObjectObj>& gameobjObjs
){
  std::map<objid, std::vector<glm::vec3>> idToModelVertexs;
  for (auto &[name, objAttr] : nameToAttr){
    // Warning: getNewObjectId will mutate the idsAdded.  
    addObjectToWorld(world, sceneId, objAttr.id, name, name, getNewObjectId, interface, objAttr.attr, idToModelVertexs, NULL, "", returnObjectOnly, gameobjObjs);
  }
  if (returnObjectOnly){
    return;
  }
  for (auto id : idsAdded){
    std::vector<glm::vec3> modelVerts = {};
    if (idToModelVertexs.find(id) != idToModelVertexs.end()){
      modelVerts = idToModelVertexs.at(id);
    }
    std::cout << "id is: " << id << std::endl;
    auto phys = addPhysicsBody(world, id, glm::vec3(1.f, 1.f, 1.f), true, modelVerts); 
    if (phys.body != NULL){   // why do I need this?
      auto transform = fullTransformation(world.sandbox, id);
      setTransform(phys.body, transform.position + phys.offset, transform.scale, transform.rotation);
    }  
  }

  for (auto id : idsAdded){
    auto obj = getGameObject(world, id);
    if (obj.script != ""){
      interface.loadCScript(obj.script, id, sceneId);
    }
  }

  for (auto id : idsAdded){
    auto obj = getGameObject(world, id); 
    world.onObjectCreate(obj);
  }
}

objid addSceneToWorldFromData(World& world, std::string sceneFileName, objid sceneId, std::string sceneData, SysInterface interface){
  auto styles = loadStyles("./res/default.style");
  auto data = addSceneDataToScenebox(world.sandbox, sceneFileName, sceneId, sceneData, styles);
  std::vector<GameObjectObj> addedGameobjObjs = {};
  addSerialObjectsToWorld(world, sceneId, data.idsAdded, getUniqueObjId, interface, data.additionalFields, false, addedGameobjObjs);
  return sceneId;
}

objid addSceneToWorld(World& world, std::string sceneFile, SysInterface interface, std::vector<Token>& addedTokens){
  auto sceneData = loadFile(sceneFile) + "\n" + serializeSceneTokens(addedTokens);  // maybe should clean this up to prevent string hackeyness
  return addSceneToWorldFromData(world, sceneFile, getUniqueObjId(), sceneData, interface);
}

// todo finish removing data like eg clearing meshes, animations,etc
void removeObjectById(World& world, objid objectId, std::string name, SysInterface interface, std::string scriptName, bool netsynchronized){
  if (world.rigidbodys.find(objectId) != world.rigidbodys.end()){
    auto rigidBody = world.rigidbodys.at(objectId).body;
    assert(rigidBody != NULL);
    rmRigidBody(world.physicsEnvironment, rigidBody);
    world.rigidbodys.erase(objectId);
  }

  interface.stopAnimation(objectId);
  removeObject(
    world.objectMapping, 
    objectId, 
    [](std::string cameraName) -> void {
      std::cout << "remove camera not yet implemented" << std::endl;
      assert(false);
    },
    [&world, name]() -> void { removeEmitter(world.emitters, name); }
  );
  
  world.onObjectDelete(objectId, netsynchronized);
  freeMeshRefsByOwner(world, objectId);
  freeTextureRefsByOwner(world, objectId);
  freeAnimationsForOwner(world, objectId);
  if (scriptName != ""){
    interface.unloadCScript(scriptName, objectId);
  }
}

void removeObjectFromScene(World& world, objid objectId, SysInterface interface){  
  if (!idExists(world.sandbox, objectId)){
    return;
  }
  std::cout << "removing object: " << objectId << objectId << " " << getGameObject(world, objectId).name << std::endl;
  for (auto gameobjId : getIdsInGroup(world.sandbox, objectId)){
    if (!idExists(world.sandbox, gameobjId)){
      std::cout << "id does not exist: " << gameobjId << std::endl;
      //assert(false);
      continue;
    }
    auto idsToRemove = idsToRemoveFromScenegraph(world.sandbox, gameobjId);
    for (auto id : idsToRemove){
      auto gameobj = getGameObject(world, id);
      auto name = gameobj.name;
      auto scriptName = gameobj.script;
      auto netsynchronized = gameobj.netsynchronize;
      removeObjectById(world, id, name, interface, scriptName, netsynchronized);
    }
    removeObjectsFromScenegraph(world.sandbox, idsToRemove);  
  }
  maybePruneScenes(world.sandbox);
}

void copyObjectToScene(World& world, objid id, SysInterface interface){
  std::cout << "INFO: SCENE: COPY OBJECT: " << id << std::endl;
  auto serializedObject = serializeObject(world, id, getGameObject(world, id).name + "-copy-" + std::to_string(getUniqueObjId()));
  std::cout << "copy object: serialized object is: " << std::endl;
  std::cout << serializedObject << std::endl << std::endl;
  addObjectToScene(world, getGameObjectH(world.sandbox, id).sceneId, serializedObject, -1, false, interface);
}


void removeSceneFromWorld(World& world, objid sceneId, SysInterface interface){
  if (!sceneExists(world.sandbox, sceneId)) {
    std::cout << "INFO: SCENE MANAGEMENT: tried to remove (" << sceneId << ") but it does not exist" << std::endl;
    return;   // @todo maybe better to throw error instead
  }

  for (auto objectId : listObjInScene(world.sandbox, sceneId)){
    auto gameobj = getGameObject(world, objectId);
    auto name = gameobj.name;
    auto scriptName = gameobj.script;
    auto netsynchronized = gameobj.netsynchronize;
    removeObjectById(world, objectId, name, interface, scriptName, netsynchronized);
  }
  removeScene(world.sandbox, sceneId);
}
void removeAllScenesFromWorld(World& world, SysInterface interface){
  for (auto sceneId : allSceneIds(world.sandbox)){
    removeSceneFromWorld(world, sceneId, interface);
  }
}

GameObjPair createObjectForScene(World& world, objid sceneId, std::string& name, GameobjAttributes& attributes, SysInterface interface, bool returnOnly){
  int id = attributes.numAttributes.find("id") != attributes.numAttributes.end() ? attributes.numAttributes.at("id") : -1;
  bool useObjId = attributes.numAttributes.find("id") != attributes.numAttributes.end();
  auto idToAdd = useObjId ? id : getUniqueObjId();
  GameObjPair gameobjPair{
    .gameobj = gameObjectFromFields(name, idToAdd, attributes),
  };
  std::vector<objid> idsAdded = { gameobjPair.gameobj.id }; 
  auto getId = [&idsAdded]() -> objid {      // kind of hackey, this could just be returned from add objects, but flow control is tricky.
    auto newId = getUniqueObjId();
    idsAdded.push_back(newId);
    return newId;
  };
  if (!returnOnly){
    addGameObjectToScene(world.sandbox, sceneId, name, gameobjPair.gameobj, attributes.children);
  }
  std::vector<GameObjectObj> addedGameobjObjs = {};
  addSerialObjectsToWorld(world, sceneId, idsAdded, getId, interface, {{ name, GameobjAttributesWithId{ .id = idToAdd, .attr = attributes }}}, returnOnly, addedGameobjObjs);
  if (returnOnly){
    assert(addedGameobjObjs.size() == 1);
    gameobjPair.gameobjObj = addedGameobjObjs.at(0);
  }
  return gameobjPair;
}

struct SingleObjDeserialization {
  std::string name;
  GameobjAttributes attr;
};
SingleObjDeserialization deserializeSingleObj(std::string& serializedObj, objid id, bool useObjId){
  auto serialAttrs = deserializeSceneTokens(parseFormat(serializedObj));
  if (serialAttrs.size() > 1){
    std::cout << "SERIALIZATION GOT MORE THAN 1 OBJECT.  Either bad data or has child element, got " << serialAttrs.size() << std::endl;
  }
  assert(serialAttrs.size() == 1);
  GameobjAttributes& attrObj = serialAttrs.begin() -> second;
  if (useObjId){
    attrObj.numAttributes["id"] = id;
  } 
  return SingleObjDeserialization{
    .name = serialAttrs.begin() -> first,
    .attr = attrObj,
  };
}

GameObjPair createObjectForScene(World& world, objid sceneId, std::string& name, std::string& serializedObj, SysInterface interface){
  auto singleObj = deserializeSingleObj(serializedObj, -1, false);
  assert(singleObj.name == name);
  return createObjectForScene(world, sceneId, singleObj.name, singleObj.attr, interface, true);
}

objid addObjectToScene(World& world, objid sceneId, std::string name, GameobjAttributes attributes, SysInterface interface){
  createObjectForScene(world, sceneId, name, attributes, interface, false).gameobj;
  return getIdForName(world.sandbox, name, sceneId);
}
objid addObjectToScene(World& world, objid sceneId, std::string serializedObj, objid id, bool useObjId, SysInterface interface){
  auto singleObj = deserializeSingleObj(serializedObj, id, useObjId);
  return addObjectToScene(world, sceneId, singleObj.name, singleObj.attr, interface);
}

GameobjAttributes objectAttributes(GameObjectObj& gameobjObj, GameObject& gameobj){
  auto attr = gameobj.attr;
  objectAttributes(gameobjObj, attr);
  getAllAttributes(gameobj, attr);
  return attr;
}

GameobjAttributes objectAttributes(World& world, objid id){
  return objectAttributes(world.objectMapping.at(id), getGameObject(world, id));
}

void afterAttributesSet(World& world, objid id, GameObject& gameobj, bool velocitySet){
  physicsLocalTransformSet(world, id, gameobj.transformation);
  btRigidBody* body = world.rigidbodys.find(id) != world.rigidbodys.end() ? world.rigidbodys.at(id).body : NULL;

  if (body != NULL){
    rigidBodyOpts opts {
      .linear = gameobj.physicsOptions.linearFactor,
      .angular = gameobj.physicsOptions.angularFactor,
      .gravity = gameobj.physicsOptions.gravity,
      .friction = gameobj.physicsOptions.friction,
      .restitution = gameobj.physicsOptions.restitution,
      .mass = gameobj.physicsOptions.mass,
      .layer = gameobj.physicsOptions.layer,
      .velocity = velocitySet ? std::optional(gameobj.physicsOptions.velocity) : std::nullopt,    // velocity is not updated so this will reset the vel
    };
    updateRigidBodyOpts(world.physicsEnvironment, body, opts);
  }
}

void setAttributes(World& world, objid id, GameobjAttributes& attr){
  setObjectAttributes(
    world.objectMapping, 
    id, 
    attr,
    [&world, id](bool enabled) -> void {
      setEmitterEnabled(world.emitters, id, enabled);
    }
  );
  GameObject& obj = getGameObject(world, id);
  setAllAttributes(obj, attr);
  afterAttributesSet(world, id, obj, attr.vecAttributes.find("physics_velocity") != attr.vecAttributes.end());
}
void setProperty(World& world, objid id, std::vector<Property>& properties){
  GameObject& gameobj = getGameObject(world, id);
  for (auto property : properties){
    setAttribute(gameobj, property.propertyName, property.value);
  }
}

void physicsTranslateSet(World& world, objid index, glm::vec3 pos, bool relative){
  //std::cout << "physics translate set: " << index << " relative: " << relative << std::endl;
  if (relative){
    updateRelativePosition(world.sandbox, index, pos);
    if (world.rigidbodys.find(index) != world.rigidbodys.end()){
      PhysicsValue& phys = world.rigidbodys.at(index);
      auto body =  phys.body;
      auto pos = fullTransformation(world.sandbox, index).position;
      setPosition(body, pos + phys.offset);
    }
  }else{
    updateAbsolutePosition(world.sandbox, index, pos);
    if (world.rigidbodys.find(index) != world.rigidbodys.end()){
      PhysicsValue& phys = world.rigidbodys.at(index);
      auto body = phys.body;
      setPosition(body, pos + phys.offset);
    }
  }
  world.entitiesToUpdate.insert(index);
}

void applyPhysicsTranslation(World& world, objid index, float offsetX, float offsetY, Axis manipulatorAxis){
  auto position = fullTransformation(world.sandbox, index).position;
  physicsTranslateSet(world, index, applyTranslation(position, offsetX, offsetY, manipulatorAxis), false);
}

void physicsRotateSet(World& world, objid index, glm::quat rotation, bool relative){
  //std::cout << "physics rotate set: " << index << " relative: " << relative << std::endl;
  if (relative){
    updateRelativeRotation(world.sandbox, index, rotation);
    if (world.rigidbodys.find(index) != world.rigidbodys.end()){
      auto body =  world.rigidbodys.at(index).body;
      auto rot = fullTransformation(world.sandbox, index).rotation;
      setRotation(body, rot);
    }
  }else{
    updateAbsoluteRotation(world.sandbox, index, rotation);
    if (world.rigidbodys.find(index) != world.rigidbodys.end()){
      auto body =  world.rigidbodys.at(index).body;
      auto rot = fullTransformation(world.sandbox, index).rotation;
      setRotation(body, rot);
    }
  }
  world.entitiesToUpdate.insert(index);
}

void applyPhysicsRotation(World& world, objid index, float offsetX, float offsetY, Axis manipulatorAxis){
  auto currentOrientation = fullTransformation(world.sandbox, index).rotation;
  physicsRotateSet(world, index, applyRotation(currentOrientation, offsetX, offsetY, manipulatorAxis), false);
}

void physicsScaleSet(World& world, objid index, glm::vec3 scale){
  updateAbsoluteScale(world.sandbox, index, scale);

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto collisionInfo = getPhysicsInfoForGameObject(world, index).transformation.scale;
    auto body =  world.rigidbodys.at(index).body;
    setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
  }
  world.entitiesToUpdate.insert(index);
}

void applyPhysicsScaling(World& world, objid index, float lastX, float lastY, float offsetX, float offsetY, Axis manipulatorAxis){
  auto transform = fullTransformation(world.sandbox, index);
  physicsScaleSet(world, index, applyScaling(transform.position, transform.scale, lastX, lastY, offsetX, offsetY, manipulatorAxis));
}

void physicsLocalTransformSet(World& world, objid index, Transformation transform){
  updateRelativeTransform(world.sandbox, index, transform);
  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    PhysicsValue& phys = world.rigidbodys.at(index);
    auto body = phys.body;
    auto fullTransform = fullTransformation(world.sandbox, index);
    setTransform(body, fullTransform.position + phys.offset, fullTransform.scale, fullTransform.rotation);
  }
}

void updatePhysicsPositionsAndClampVelocity(World& world, std::map<objid, PhysicsValue>& rigidbodys){
  for (auto [i, rigidBody]: rigidbodys){
    GameObject& gameobj = getGameObject(world, i);
    updateAbsoluteTransform(world.sandbox, i, Transformation {
      .position = getPosition(rigidBody.body) - rigidBody.offset,
      .scale = getScale(rigidBody.body),
      .rotation = getRotation(rigidBody.body),
    });
    clampMaxVelocity(rigidBody.body, gameobj.physicsOptions.maxspeed);
  }
}

void updateSoundPositions(World& world){
  forEveryGameobj(world.sandbox, [&world](objid id, GameObject& gameobj) -> void {
    auto absolutePosition = fullTransformation(world.sandbox, id).position;
    updatePosition(world.objectMapping, id, absolutePosition);
  });
}

void enforceLookAt(World& world){
  forEveryGameobj(world.sandbox, [&world](objid id, GameObject& gameobj) -> void {
    std::string lookAt = gameobj.lookat;                      
    if (lookAt == "" || lookAt == gameobj.name){
      return;
    }
    auto sceneId = getGameObjectH(world.sandbox, id).sceneId;
    if(idExists(world.sandbox, lookAt, sceneId)){
      glm::vec3 fromPos = fullTransformation(world.sandbox, id).position;
      glm::vec3 targetPosition = fullTransformation(world.sandbox, getGameObject(world.sandbox, lookAt, sceneId).id).position;
      physicsRotateSet(world, id, orientationFromPos(fromPos, targetPosition), false);
    }
  });  
}

void callbackEntities(World& world){
  forEveryGameobj(world.sandbox, [&world](objid id, GameObject& gameobj) -> void {
    if (id == getGroupId(world.sandbox, id) && world.entitiesToUpdate.find(id) != world.entitiesToUpdate.end()){
      world.onObjectUpdate(gameobj);
    }
  });  
  world.entitiesToUpdate.clear();
}

void onWorldFrame(World& world, float timestep, float timeElapsed,  bool enablePhysics, bool dumpPhysics, SysInterface interface){
  updateEmitters(
    world.emitters, 
    timeElapsed,
    [&world, &interface](std::string name, GameobjAttributes attributes, objid emitterNodeId, NewParticleOptions particleOpts) -> objid {      
      std::cout << "INFO: emitter: creating particle from emitter: " << name << std::endl;
      attributes.vecAttributes["position"] = particleOpts.position.has_value() ?  particleOpts.position.value() : fullTransformation(world.sandbox, emitterNodeId).position;
      if (particleOpts.velocity.has_value()){
        attributes.vecAttributes["physics_velocity"] = particleOpts.velocity.value();
      }
      if (particleOpts.angularVelocity.has_value()){
        attributes.vecAttributes["physics_avelocity"] = particleOpts.angularVelocity.value();
      }
      objid objectAdded = addObjectToScene(
        world, getGameObjectH(world.sandbox, emitterNodeId).sceneId, getUniqueObjectName(), attributes, interface
      );
      if (particleOpts.orientation.has_value()){
        physicsRotateSet(world, objectAdded, particleOpts.orientation.value(), true);
      }
      return objectAdded;
    }, 
    [&world, &interface](objid id) -> void { 
      std::cout << "INFO: emitter: removing particle from emitter: " << id << std::endl;
      if (idExists(world.sandbox, id)){
        removeObjectFromScene(world, id, interface);
      }
    },
    [&world](objid id, std::string attribute, AttributeValue delta)  -> void {
      std::cout << "update particle attr -- WARNING ADD FPS INDEPNDENC HERE: " << attribute << std::endl;

      GameObject& gameobj = getGameObject(world, id);
      applyAttributeDelta(gameobj, attribute, delta);
      afterAttributesSet(world, id, gameobj, attribute == "physics_velocity");

      //setAttributes(world, id, GameobjAttributes& attr){
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
  updateSandbox(world.sandbox);
  callbackEntities(world);

  onObjectFrame(world.objectMapping, [&world](std::string texturepath, unsigned char* data, int textureWidth, int textureHeight) -> void {
    updateTextureDataWorld(world, texturepath, data, textureWidth, textureHeight);
  }, timeElapsed);
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

std::string sceneNameForSceneId(World& world, objid sceneId){
  return world.sandbox.sceneIdToSceneName.at(sceneId);
}
