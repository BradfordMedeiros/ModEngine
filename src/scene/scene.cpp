#include "./scene.h"

GameObject& getGameObject(World& world, objid id){
  return getGameObject(world.sandbox, id);
}

std::optional<objid> getGameObjectByNamePrefix(World& world, std::string name, objid sceneId, bool sceneIdExplicit){
  auto obj = maybeGetGameObjectByName(world.sandbox, name, sceneId, true);
  if (obj.has_value()){
    return obj.value() -> id;
  }
  return std::nullopt;
}
GameObject& getGameObject(World& world, std::string name, objid sceneId){
  auto obj = maybeGetGameObjectByName(world.sandbox, name, sceneId, false);
  modassert(obj.has_value(), std::string("gameobject : ") + name + " does not exist");
  return *obj.value();
}

NameAndMeshObjName getMeshesForGroupId(World& world, objid groupId){
  std::vector<std::string> objnames;
  std::vector<std::reference_wrapper<std::string>> meshNames;
  std::vector<std::reference_wrapper<Mesh>> meshes;
  NameAndMeshObjName nameAndMeshes = {
    .objnames = objnames,
    .meshNames = meshNames,
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

  bool hasOffset = false;
  auto textObj = std::get_if<GameObjectUIText>(&gameObjV);
  if (textObj != NULL){
    // textObj -> value, 1, textObj -> deltaOffset, textObj -> align, textObj -> wrap, textObj -> virtualization, &offset
    hasOffset = true;
    boundInfo = boundInfoForCenteredText(
      world.interface.fontFamilyByName(textObj -> fontFamily),
      textObj -> value,
      0,
      0,
      1000,
      0, 
      textObj -> align, 
      textObj -> wrap, 
      textObj -> virtualization, 
      textObj -> cursor.cursorIndex, 
      textObj -> cursor.cursorIndexLeft, 
      textObj -> cursor.highlightLength,
      &offset
    ); 
  }

  PhysicsInfo info = {
    .boundInfo = boundInfo,
    .transformation = obj.transformation,
    .offset = hasOffset ? std::optional<glm::vec3>(offset) : std::nullopt,
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
PhysicsValue addPhysicsBody(World& world, objid id, bool initialLoad, std::vector<glm::vec3> verts){
  auto groupPhysicsInfo = getPhysicsInfoForGroup(world, id);
  if (!groupPhysicsInfo.physicsOptions.enabled){
    return PhysicsValue { .body = NULL, .offset = std::nullopt };
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
      .linearDamping = physicsOptions.linearDamping,
    };

    if (isHeightmapObj){
      rigidBody = addRigidBodyHeightmap(
        world.physicsEnvironment,
        calcOffsetFromRotation(physicsInfo.transformation.position, groupPhysicsInfo.physicsInfo.offset, physicsInfo.transformation.rotation),
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
        calcOffsetFromRotation(physicsInfo.transformation.position, groupPhysicsInfo.physicsInfo.offset, physicsInfo.transformation.rotation), 
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
        calcOffsetFromRotation(physicsInfo.transformation.position, groupPhysicsInfo.physicsInfo.offset, physicsInfo.transformation.rotation),
        calculateRadiusForScale(
          glm::vec3(
            (physicsInfo.boundInfo.xMax - physicsInfo.boundInfo.xMin), 
            (physicsInfo.boundInfo.yMax - physicsInfo.boundInfo.yMin), 
            (physicsInfo.boundInfo.zMax - physicsInfo.boundInfo.zMin)
          )
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
        calcOffsetFromRotation(physicsInfo.transformation.position, groupPhysicsInfo.physicsInfo.offset, physicsInfo.transformation.rotation),
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
        calcOffsetFromRotation(physicsInfo.transformation.position, groupPhysicsInfo.physicsInfo.offset, physicsInfo.transformation.rotation),
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
        calcOffsetFromRotation(physicsInfo.transformation.position, groupPhysicsInfo.physicsInfo.offset, physicsInfo.transformation.rotation),
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
        calcOffsetFromRotation(physicsInfo.transformation.position, groupPhysicsInfo.physicsInfo.offset, physicsInfo.transformation.rotation),
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
        calcOffsetFromRotation(physicsInfo.transformation.position, groupPhysicsInfo.physicsInfo.offset, physicsInfo.transformation.rotation),
        physicsInfo.transformation.rotation,
        getVoxelBodies(std::get_if<GameObjectVoxel>(&toRender) -> voxel),
        physicsOptions.isStatic,
        physicsOptions.hasCollisions,
        physicsInfo.transformation.scale,
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
  bool hasRigidBody = world.rigidbodys.find(id) != world.rigidbodys.end();
  if (hasRigidBody){
    auto rigidBody = world.rigidbodys.at(id).body;
    assert(rigidBody != NULL);
    rmRigidBody(world, id);
  }
  addPhysicsBody(world, id, false, {});
}

Texture loadTextureWorld(World& world, std::string texturepath, objid ownerId){
  std::cout << "load texture world: " << texturepath << std::endl;
  if (world.textures.find(texturepath) != world.textures.end()){
    world.textures.at(texturepath).owners.insert(ownerId);
    return world.textures.at(texturepath).texture;
  }
  auto texturePath = world.interface.modlayerPath(texturepath);
  if (!fileExists(texturePath) && ownerId != -1){ // root owner for default models etc, should never use default texs
    return loadTextureWorld(world, "./res/models/box/grid.png", ownerId);
  }
  Texture texture = loadTexture(texturePath);
  world.textures[texturepath] = TextureRef {
    .owners = { ownerId },
    .texture = texture,
    .mappingTexture = std::nullopt,
  };
  return texture;
}

Texture loadTextureWorldEmpty(World& world, std::string texturepath, objid ownerId, int textureWidth, int textureHeight, std::optional<objid> mappingTexture){
  std::cout << "load texture world empty: " << texturepath << std::endl;
  modassert(world.textures.find(texturepath) == world.textures.end(), "texture is already loaded: " + texturepath);
  Texture texture = loadTextureEmpty(textureWidth, textureHeight, 4);
  world.textures[texturepath] = TextureRef {
    .owners = { ownerId },
    .texture = texture,
    .mappingTexture = mappingTexture,
  };
  return texture;  
}

void maybeReloadTextureWorld(World& world, std::string texturepath){
  if (world.textures.find(texturepath) == world.textures.end()){
    modlog("scene", std::string("texture not reloaded because does not exist: ") + texturepath);
    return;
  }
  modlog("scene", std::string("texture attempt reload: ") + texturepath);
  replaceTexture(world.textures.at(texturepath).texture, texturepath, true);
  modlog("scene", std::string("texture reloaded: ") + texturepath);
}

Texture loadSkyboxWorld(World& world, std::string texturepath, objid ownerId){
  if (world.textures.find(texturepath) != world.textures.end()){
    world.textures.at(texturepath).owners.insert(ownerId);
    return world.textures.at(texturepath).texture;
  }
  Texture texture = loadCubemapTexture(world.interface.modlayerPath(texturepath));
  world.textures[texturepath] = TextureRef {
    .owners = { ownerId },
    .texture = texture,
    .mappingTexture = std::nullopt,
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
    .mappingTexture = std::nullopt,
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

void freeTextureRefsIdByOwner(World& world, int ownerId, std::optional<int> id){
  for (auto &[name, textureRef] : world.textures){
    if (!id.has_value() || id.value() == textureRef.texture.textureId){
      textureRef.owners.erase(ownerId);
    }
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
void freeTextureRefsByOwner(World& world, int ownerId){
  freeTextureRefsIdByOwner(world, ownerId, std::nullopt);
}

std::optional<objid> getMappingTexture(World& world, std::string& texture){
  for (auto &[name, textureRef] : world.textures){
    if (name == texture){
      return textureRef.mappingTexture;
    }
  }
  return std::nullopt;
}

void freeAnimationsForOwner(World& world, objid id){
  std::cout << "INFO: removing animations for: " << id << std::endl;
  world.animations.erase(id);
}

void loadMeshData(World& world, std::string meshPath, MeshData& meshData, int ownerId){
  if (world.meshes.find(meshPath) != world.meshes.end()){
    world.meshes.at(meshPath).owners.insert(ownerId);
    for (auto &textureRef : world.meshes.at(meshPath).textureRefs){
      world.textures.at(textureRef).owners.insert(ownerId);
    }
  }else{
    std::set<std::string> textureRefs = {};
    world.meshes[meshPath] = MeshRef {
      .owners = { ownerId },
      .mesh = loadMesh("./res/textures/default.jpg", meshData, [&world, &textureRefs, &meshPath, ownerId](std::string texture) -> Texture {
        textureRefs.insert(texture);
        return loadTextureWorld(world, texture, ownerId);
      })  
    };
    world.meshes.at(meshPath).textureRefs = textureRefs;
  }
}

ModelData loadModelPath(World& world, std::string rootname, std::string modelPath){
  return loadModel(rootname, world.interface.modlayerPath(modelPath));
}

void addMesh(World& world, std::string meshpath){
  ModelData data = loadModelPath(world, "", meshpath);
  if (data.meshIdToMeshData.size() !=  1){
    std::cout << "ERROR: " << meshpath << " actual size: " << data.meshIdToMeshData.size() << std::endl;
    assert(false);
  }
  auto meshData = data.meshIdToMeshData.begin() -> second;
  loadMeshData(world, meshpath, meshData, -1);
  std::cout << "WARNING: add mesh does not load animations, bones for default meshes" << std::endl;
}

void addSpriteMesh(World& world, std::string meshPath){
  int ownerId = -1;
  if (world.meshes.find(meshPath) != world.meshes.end()){
    world.meshes.at(meshPath).owners.insert(ownerId);
  }else{
    std::set<std::string> textureRefs = {};
    world.meshes[meshPath] = MeshRef {
      .owners = { ownerId },
      .textureRefs = textureRefs,
      .mesh = loadSpriteMesh(meshPath, [&world, &textureRefs, ownerId](std::string texture) -> Texture {
        textureRefs.insert(texture);
        return loadTextureWorld(world, texture, ownerId);
      })
    };
    world.meshes.at(meshPath).textureRefs = textureRefs;
  }
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

  std::set<std::string> textureRefs = {};
  world.meshes["skybox"] = MeshRef {
    .owners = { -1 },
    .textureRefs = {},
    .mesh = loadSkybox(
      "./res/textures/default.jpg", 
      "./res/models/skybox.obj",
      skyboxpath, 
      [&world, &textureRefs](std::string texture) -> Texture {
        textureRefs.insert(texture);
        return loadTextureWorld(world, texture, -1);
      },
      [&world, &textureRefs](std::string texture) -> Texture {
        textureRefs.insert(texture);
        return loadSkyboxWorld(world, texture, -1);
      }
    )
  };
  world.meshes.at("skybox").textureRefs = textureRefs;

}

extern std::vector<AutoSerialize> meshAutoserializer;
extern std::vector<AutoSerialize> cameraAutoserializer;
extern std::vector<AutoSerialize> portalAutoserializer;
extern std::vector<AutoSerialize> soundAutoserializer;
extern std::vector<AutoSerialize> lightAutoserializer;
extern std::vector<AutoSerialize> voxelAutoserializer;
extern std::vector<AutoSerialize> emitterAutoserializer;
extern std::vector<AutoSerialize> heightmapAutoserializer;
extern std::vector<AutoSerialize> navmeshAutoserializer;
extern std::vector<AutoSerialize> navconnAutoserializer;
extern std::vector<AutoSerialize> uiLayoutAutoserializer;
extern std::vector<AutoSerialize> uiSliderAutoserializer;
extern std::vector<AutoSerialize> textAutoserializer;
extern std::vector<AutoSerialize> geoAutoserializer;
extern std::vector<AutoSerialize> uiButtonAutoserializer;
extern std::vector<AutoSerialize> prefabAutoserializer;
std::set<std::string> getObjautoserializerFields(std::string& name){
  auto type = getType(name);
  if (type == "default"){
    return serializerFieldNames(meshAutoserializer);
  }else if (type == "camera"){
    return serializerFieldNames(cameraAutoserializer);
  }else if (type == "portal"){
    return serializerFieldNames(portalAutoserializer);
  }else if (type == "sound"){
    return serializerFieldNames(soundAutoserializer);
  }else if (type == "light"){
    return serializerFieldNames(lightAutoserializer);
  }else if (type == "voxel"){
    return serializerFieldNames(voxelAutoserializer);
  }else if (type == "root"){
    return {};
  }else if (type == "emitter"){
    return serializerFieldNames(emitterAutoserializer);
  }else if (type == "heightmap"){
    return serializerFieldNames(heightmapAutoserializer);
  }else if (type == "navmesh"){
    return serializerFieldNames(navmeshAutoserializer);
  }else if (type == "navconnection"){
    return serializerFieldNames(navconnAutoserializer);
  }else if (type == "slider"){
    return serializerFieldNames(uiSliderAutoserializer);
  }else if (type == "text"){
    return serializerFieldNames(textAutoserializer);
  }else if (type == "layout"){
    return serializerFieldNames(uiLayoutAutoserializer);
  }else if (type == "ui"){
    return serializerFieldNames(uiButtonAutoserializer);
  }else if (type == "geo"){
    return serializerFieldNames(geoAutoserializer);
  }else if (type == "custom"){
    return {};
  }else if (type == "prefab"){
    return serializerFieldNames(prefabAutoserializer);
  }
  modassert(false, "autoserializer not found");
  return {};
}

// kind of hackey, this could just be returned from add objects, but flow control is tricky.
std::function<objid(void)> createGetUniqueObjId(std::vector<objid>& idsAdded){
  return [&idsAdded]() -> objid {      
    auto newId = getUniqueObjId();
    idsAdded.push_back(newId);
    return newId;
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
  std::vector<std::string> defaultMeshes,
  std::vector<std::string> spriteMeshes
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
    .sandbox = createSceneSandbox(layers, getObjautoserializerFields),
    .interface = interface,
  };

  // hackey, but createSceneSandbox adds root object with id 0 so this is needed
  std::vector<objid> idsAdded = { 0 };
  std::vector<GameObjectObj> addedGameobjObjs = {};
  std::map<std::string, GameobjAttributes> submodelAttributes;

  auto getId = createGetUniqueObjId(idsAdded);
  addSerialObjectsToWorld(world, world.sandbox.mainScene.rootId, idsAdded, getId, {{ "root", GameobjAttributesWithId { .id = idsAdded.at(0), .attr = GameobjAttributes{}}}}, false, addedGameobjObjs, submodelAttributes);

  // Default meshes that are silently loaded in the background
  for (auto &meshname : defaultMeshes){
    addMesh(world, meshname);
  }
  for (auto &spriteMesh : spriteMeshes){
    addSpriteMesh(world, spriteMesh);
  }
  loadSkybox(world, ""); 
  return world;
}

std::map<objid, GameobjAttributes> applyFieldsToSubelements(std::string meshName, ModelData& data, GameobjAttributes& attr, std::map<std::string, GameobjAttributes>& overrideAttributes){
  std::map<objid, GameobjAttributes> additionalFieldsMap;
  for (auto [nodeId, _] : data.nodeTransform){
    additionalFieldsMap[nodeId] = GameobjAttributes { };
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

  for (auto &[id, name] : data.names){
    if (overrideAttributes.find(name) != overrideAttributes.end()){
      GameobjAttributes& attrOverrides = overrideAttributes.at(name);
      mergeAttributes(additionalFieldsMap.at(id), attrOverrides);
    }
  }


  return additionalFieldsMap;
}

std::optional<std::string> getTextureById(World& world, int id){
  for (auto &[textureName, texture] : world.textures){
    if (texture.texture.textureId == id){
      return textureName;
    }
  }
  return std::nullopt;
}

std::string serializeScene(World& world, objid sceneId, bool includeIds){
  return serializeScene(world.sandbox, sceneId, [&world](objid objectId)-> std::vector<std::pair<std::string, std::string>> {
    return getAdditionalFields(objectId, world.objectMapping, [&world](int textureId) -> std::string {
      return getTextureById(world, textureId).value();
    });
  }, includeIds);
} 

std::string serializeObjectById(World& world, objid id, std::string overridename){
  auto gameobj = getGameObject(world.sandbox, id);
  auto gameobjecth = getGameObjectH(world.sandbox, id);
  auto children = childnames(world.sandbox, gameobjecth);
  auto additionalFields = getAdditionalFields(id, world.objectMapping, [&world](int textureId) -> std::string {
    return getTextureById(world, textureId).value();
  });
  return serializeObj(id, gameobjecth.groupId, gameobj, children, false, additionalFields, overridename);
}

std::string serializeObject(World& world, objid id, bool includeSubmodelAttr, std::string overridename){
  // std::vector<objid> getIdsInGroup(SceneSandbox& sandbox, objid index){
  std::vector<objid> singleId = { id };
  auto idsToSerialize = includeSubmodelAttr ? getIdsInGroup(world.sandbox, getGroupId(world.sandbox, id)) : singleId;

  std::string serializedData = "";
  for (auto &gameobjid : idsToSerialize){
    std::cout << "serialized = " << gameobjid <<  "(" << std::endl;
    auto newName = rewriteTargetName(getGameObject(world, gameobjid).name, overridename);
    auto serialized = serializeObjectById(world, gameobjid, newName);
    std::cout << serialized;
    std::cout << ")" << std::endl << std::endl;
    serializedData += serialized;
  }

  std::cout << "all serialized data: " << std::endl;
  std::cout << serializedData << std::endl << std::endl;
  
  return serializedData;
}

void addObjectToWorld(
  World& world, 
  objid sceneId, 
  objid id,
  std::string name,
  std::string topName,
  std::function<objid()> getId,
  GameobjAttributes attr,
  std::map<objid, std::vector<glm::vec3>>& idToModelVertexs,
  std::map<std::string, GameobjAttributes>& submodelAttributes,
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
    auto addEmitterObject = [&world, name, id](std::string templateName, float spawnrate, float lifetime, int limit, GameobjAttributes& particleFields, std::map<std::string, GameobjAttributes> submodelAttributes, std::vector<EmitterDelta> deltas, bool enabled, EmitterDeleteBehavior behavior) -> void {
      addEmitter(world.emitters, name, templateName, id, world.interface.getCurrentTime(), limit, spawnrate, lifetime, particleFields, submodelAttributes, deltas, enabled, behavior);
    };
    auto onCollisionChange = [&world, id]() -> void {
      //assert(false); // think about what this should do better!
      updatePhysicsBody(world, id);
    };
    auto ensureTextureLoaded = [&world, id](std::string texturepath) -> Texture {
      std::cout << "Custom texture loading: " << texturepath << std::endl;
      return loadTextureWorld(world, texturepath, id);
    };
    auto ensureMeshLoaded = [&world, sceneId, id, name, topName, getId, &attr, &idToModelVertexs, &submodelAttributes, data, &rootMeshName, returnObjectOnly, &returnobjs](std::string meshName, bool* _isRoot) -> std::vector<std::string> {
      *_isRoot = data == NULL;
      if (meshName == ""){
        return {};
      }
      if (data == NULL){
        ModelData data = loadModelPath(world, name, meshName); 
        idToModelVertexs[id] = getVertexsFromModelData(data);
        if (data.animations.size() > 0){
          world.animations[id] = data.animations;
        }

        for (auto [meshId, meshData] : data.meshIdToMeshData){
          auto meshPath = nameForMeshId(meshName, meshId);
          loadMeshData(world, meshPath, meshData, id);
        } 

        auto additionalFields = applyFieldsToSubelements(meshName, data, attr, submodelAttributes);
        auto newSerialObjs = multiObjAdd(
          world.sandbox,
          sceneId,
          id,
          0,
          data.childToParent, 
          data.nodeTransform, 
          data.names, 
          additionalFields,
          getId,
          getObjautoserializerFields
        );

        std::cout << "id is: " << id << std::endl;
        for (auto &[name, objAttr] : newSerialObjs){
          addObjectToWorld(world, sceneId, objAttr.id, name, topName, getId, objAttr.attr, idToModelVertexs, submodelAttributes, &data, meshName, returnObjectOnly, returnobjs);
        }
        std::cout << std::endl;
        return meshNamesForNode(data, meshName, name);
      }
      return meshNamesForNode(*data, rootMeshName, name);  
    }; 

    auto loadScene = [&world, id](std::string sceneFile, std::vector<Token>& addedTokens) -> objid {
      modlog("load scene", std::string("loading scene: " + sceneFile));
      auto sceneId = addSceneToWorld(world, sceneFile, addedTokens, std::nullopt, std::nullopt, std::nullopt);  // should make original obj the parent
      auto rootId = rootIdForScene(world.sandbox, sceneId);
      makeParent(world.sandbox, rootId, id);
      return sceneId;
    };

    if (returnObjectOnly){
      ObjectTypeUtil util {
        .id = id,
        .createMeshCopy = getCreateMeshCopy(world, topName),
        .meshes = world.meshes,
        .ensureTextureLoaded = [](std::string) -> Texture { return Texture{}; },
        .releaseTexture = [&world, id](int textureId){
          freeTextureRefsIdByOwner(world, id, textureId);
        },
        .loadMesh = [](MeshData&) -> Mesh { return Mesh{}; },
        .addEmitter =  [](std::string templateName, float spawnrate, float lifetime, int limit, GameobjAttributes& particleFields, std::map<std::string, GameobjAttributes>& submodelAttributes, std::vector<EmitterDelta> deltas, bool enabled, EmitterDeleteBehavior behavior) -> void {},
        .ensureMeshLoaded = [](std::string meshName, bool* isRoot) -> std::vector<std::string> { *isRoot = true; return {  }; },
        .onCollisionChange = []() -> void {},
        .pathForModLayer = world.interface.modlayerPath,
        .loadScene = loadScene,
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
      .releaseTexture = [&world, id](int textureId){
          freeTextureRefsIdByOwner(world, id, textureId);
      },
      .loadMesh = loadMeshObject,
      .addEmitter = addEmitterObject,
      .ensureMeshLoaded = ensureMeshLoaded,
      .onCollisionChange = onCollisionChange,
      .pathForModLayer = world.interface.modlayerPath,
      .loadScene = loadScene,
    };
    auto gameobjObj = createObjectType(getType(name), attr, util);
    addObjectType(world.objectMapping, gameobjObj, id);
}

void addSerialObjectsToWorld(
  World& world, 
  objid sceneId, 
  std::vector<objid>& idsAdded,
  std::function<objid()> getNewObjectId,
  std::map<std::string, GameobjAttributesWithId> nameToAttr,
  bool returnObjectOnly,
  std::vector<GameObjectObj>& gameobjObjs,
  std::map<std::string, GameobjAttributes>& submodelAttributes
){

  //for (auto &[name, _] : submodelAttributes){
  //  std::cout << "submodel attr: " << name << std::endl;
  //}

  std::map<objid, std::vector<glm::vec3>> idToModelVertexs;
  for (auto &[name, objAttr] : nameToAttr){
    // Warning: getNewObjectId will mutate the idsAdded.  
    //std::cout << "add serial: " << name << std::endl;
    //std::cout << print(objAttr.attr) << std::endl;
    //std::cout << std::endl;
    addObjectToWorld(world, sceneId, objAttr.id, name, name, getNewObjectId, objAttr.attr, idToModelVertexs, submodelAttributes, NULL, "", returnObjectOnly, gameobjObjs);
  }
  if (returnObjectOnly){
    return;
  }

  for (auto &id : idsAdded){
    std::vector<glm::vec3> modelVerts = {};
    if (idToModelVertexs.find(id) != idToModelVertexs.end()){
      modelVerts = idToModelVertexs.at(id);
    }
    auto phys = addPhysicsBody(world, id, true, modelVerts); 
    if (phys.body != NULL){   // why do I need this?
      auto transform = fullTransformation(world.sandbox, id);
      setTransform(world.physicsEnvironment, phys.body, calcOffsetFromRotation(transform.position, phys.offset, transform.rotation), transform.scale, transform.rotation);
    }  
  }

  for (auto id : idsAdded){
    auto obj = getGameObject(world, id);
    if (obj.script != ""){
      world.interface.loadCScript(obj.script, id, sceneId);
    }
  }

  for (auto id : idsAdded){
    auto obj = getGameObject(world, id); 
    world.onObjectCreate(obj);
  }
}

objid addSceneToWorldFromData(World& world, std::string sceneFileName, objid sceneId, std::string sceneData, std::optional<std::string> name, std::optional<std::vector<std::string>> tags){
  auto styles = loadStyles("./res/default.style", world.interface.readFile);
  auto data = addSceneDataToScenebox(world.sandbox, sceneFileName, sceneId, sceneData, styles, name, tags, getObjautoserializerFields);
  std::vector<GameObjectObj> addedGameobjObjs = {};

  auto getId = createGetUniqueObjId(data.idsAdded);
  addSerialObjectsToWorld(world, sceneId, data.idsAdded, getId, data.additionalFields, false, addedGameobjObjs, data.subelementAttributes);
  return sceneId;
}

objid addSceneToWorld(World& world, std::string sceneFile, std::vector<Token>& addedTokens, std::optional<std::string> name, std::optional<std::vector<std::string>> tags, std::optional<objid> sceneId){
  auto sceneData = world.interface.readFile(sceneFile) + "\n" + serializeSceneTokens(addedTokens);  // maybe should clean this up to prevent string hackeyness
  return addSceneToWorldFromData(world, sceneFile, sceneId.has_value() ? sceneId.value() : getUniqueObjId(), sceneData, name, tags);
}

// todo finish removing data like eg clearing meshes, animations,etc
void removeObjectById(World& world, objid objectId, std::string name, std::string scriptName, bool netsynchronized){
  if (world.rigidbodys.find(objectId) != world.rigidbodys.end()){
    auto rigidBody = world.rigidbodys.at(objectId).body;
    assert(rigidBody != NULL);
    rmRigidBody(world.physicsEnvironment, rigidBody);
    world.rigidbodys.erase(objectId);
  }

  world.interface.stopAnimation(objectId);
  world.onObjectDelete(objectId, netsynchronized);
  removeObject(
    world.objectMapping, 
    objectId, 
    [](std::string cameraName) -> void {
      std::cout << "remove camera not yet implemented" << std::endl;
      assert(false);
    },
    [&world, name]() -> void { removeEmitter(world.emitters, name); },
    [&world](objid sceneId) -> void {
      modlog("load scene", std::string("unload scene: " + std::to_string(sceneId)));
      removeSceneFromWorld(world, sceneId);
    }
  );
  
  freeMeshRefsByOwner(world, objectId);
  freeTextureRefsByOwner(world, objectId);
  freeAnimationsForOwner(world, objectId);
  if (scriptName != ""){
    world.interface.unloadCScript(scriptName, objectId);
  }
}

void removeObjectFromScene(World& world, objid objectId){  
  if (!idExists(world.sandbox, objectId)){
    return;
  }
  std::cout << "removing object: " << objectId << objectId << " " << getGameObject(world, objectId).name << std::endl;
  for (auto gameobjId : getIdsInGroup(world.sandbox, objectId)){
    if (!idExists(world.sandbox, gameobjId)){
      //std::cout << "id does not exist: " << gameobjId << std::endl;
      //assert(false);
      continue;
    }
    auto idsToRemove = idsToRemoveFromScenegraph(world.sandbox, gameobjId);
    for (auto id : idsToRemove){
      if (!idExists(world.sandbox, id)){ // needed b/c removeobjectbyid could remove other entities in scene
        continue;
      }
      auto gameobj = getGameObject(world, id);
      auto name = gameobj.name;
      auto scriptName = gameobj.script;
      auto netsynchronized = gameobj.netsynchronize;
      removeObjectById(world, id, name, scriptName, netsynchronized);
    }
    removeObjectsFromScenegraph(world.sandbox, idsToRemove);  
  }
  maybePruneScenes(world.sandbox);
}

bool copyObjectToScene(World& world, objid id){
  std::cout << "INFO: SCENE: COPY OBJECT: " << id << std::endl;
  auto serializedObject = serializeObject(world, id, true, getGameObject(world, id).name + "-copy-" + std::to_string(getUniqueObjId()));
  if (!deserializeSingleObj(serializedObject, id, false).has_value()){  // really bad hack 
    modlog("copy object", "copy object failure, more than one object tried to be copied");
    return false;
  }
  addObjectToScene(world, getGameObjectH(world.sandbox, id).sceneId, serializedObject, -1, false);
  return true;
}


void removeSceneFromWorld(World& world, objid sceneId){
  if (!sceneExists(world.sandbox, sceneId)) {
    std::cout << "INFO: SCENE MANAGEMENT: tried to remove (" << sceneId << ") but it does not exist" << std::endl;
    return;   // @todo maybe better to throw error instead
  }

  for (auto objectId : listObjInScene(world.sandbox, sceneId)){
    if (!idExists(world.sandbox, objectId)){  // this is needed b/c removeobject by id can in turn end up removing other entities
      continue;
    }
    auto gameobj = getGameObject(world, objectId);
    auto name = gameobj.name;
    auto scriptName = gameobj.script;
    auto netsynchronized = gameobj.netsynchronize;
    removeObjectById(world, objectId, name, scriptName, netsynchronized);
  }
  removeScene(world.sandbox, sceneId);
}
void removeAllScenesFromWorld(World& world){
  for (auto sceneId : allSceneIds(world.sandbox, std::nullopt)){
    removeSceneFromWorld(world, sceneId);
  }
}

GameObjPair createObjectForScene(World& world, objid sceneId, std::string& name, AttrChildrenPair& attrWithChildren, std::map<std::string, GameobjAttributes>& submodelAttributes, bool returnOnly){
  GameobjAttributes& attributes = attrWithChildren.attr;
  int id = attributes.numAttributes.find("id") != attributes.numAttributes.end() ? attributes.numAttributes.at("id") : -1;
  bool useObjId = attributes.numAttributes.find("id") != attributes.numAttributes.end();
  auto idToAdd = useObjId ? id : getUniqueObjId();

  GameObjPair gameobjPair{
    .gameobj = gameObjectFromFields(name, idToAdd, attributes, getObjautoserializerFields(name)),
  };
  std::vector<objid> idsAdded = { gameobjPair.gameobj.id }; 
  auto getId = createGetUniqueObjId(idsAdded);
  if (!returnOnly){
    addGameObjectToScene(world.sandbox, sceneId, name, gameobjPair.gameobj, attrWithChildren.children);
  }
  std::vector<GameObjectObj> addedGameobjObjs = {};

  
  addSerialObjectsToWorld(world, sceneId, idsAdded, getId, {{ name, GameobjAttributesWithId{ .id = idToAdd, .attr = attributes }}}, returnOnly, addedGameobjObjs, submodelAttributes);
  if (returnOnly){
    assert(addedGameobjObjs.size() == 1);
    gameobjPair.gameobjObj = addedGameobjObjs.at(0);
  }
  return gameobjPair;
}

std::optional<SingleObjDeserialization> deserializeSingleObj(std::string& serializedObj, objid id, bool useObjId){
  auto tokens = parseFormat(serializedObj);
  auto dividedTokens = divideMainAndSubelementTokens(tokens);
  auto serialAttrs = deserializeSceneTokens(dividedTokens.mainTokens);
  auto subelementAttrs = deserializeSceneTokens(dividedTokens.subelementTokens);
  std::map<std::string, GameobjAttributes> subelementAttributes;
  for (auto &[name, attrWithChildren] : subelementAttrs){
    subelementAttributes[name] = attrWithChildren.attr;
  }

  if (serialAttrs.size() > 1){
    std::cout << "SERIALIZATION GOT MORE THAN 1 OBJECT.  Either bad data or has child element, got " << serialAttrs.size() << std::endl;
    return std::nullopt;
  }
  assert(serialAttrs.size() == 1);
  AttrChildrenPair& attrObj = serialAttrs.begin() -> second;
  if (useObjId){
    attrObj.attr.numAttributes["id"] = id;
  } 
  return SingleObjDeserialization{
    .name = serialAttrs.begin() -> first,
    .attrWithChildren = attrObj,
    .submodelAttributes = subelementAttributes,
  };
}

GameObjPair createObjectForScene(World& world, objid sceneId, std::string& name, std::string& serializedObj){
  auto singleObj = deserializeSingleObj(serializedObj, -1, false).value();
  assert(singleObj.name == name);
  return createObjectForScene(world, sceneId, singleObj.name, singleObj.attrWithChildren, singleObj.submodelAttributes, true);
}

objid addObjectToScene(World& world, objid sceneId, std::string name, AttrChildrenPair attrWithChildren, std::map<std::string, GameobjAttributes>& submodelAttributes){
  createObjectForScene(world, sceneId, name, attrWithChildren, submodelAttributes, false).gameobj;
  return getIdForName(world.sandbox, name, sceneId);
}

objid addObjectToScene(World& world, objid sceneId, std::string serializedObj, objid id, bool useObjId){
  std::cout << "serialized obj: " << std::endl;
  std::cout << serializedObj << std::endl << std::endl;

  auto singleObj = deserializeSingleObj(serializedObj, id, useObjId).value();
  return addObjectToScene(world, sceneId, singleObj.name, singleObj.attrWithChildren, singleObj.submodelAttributes);
}

GameobjAttributes objectAttributes(GameObjectObj& gameobjObj, GameObject& gameobj){
  GameobjAttributes attr = gameobj.additionalAttr;
  objectAttributes(gameobjObj, attr);
  getAllAttributes(gameobj, attr);
  return attr;
}

GameobjAttributes objectAttributes(World& world, objid id){
  return objectAttributes(world.objectMapping.at(id), getGameObject(world, id));
}

bool objectHasAttribute(World& world, objid id, std::string type, std::optional<AttributeValue> value){
  auto attrs = objectAttributes(world, id);
  return hasAttribute(attrs, type, value);
}

void afterAttributesSet(World& world, objid id, GameObject& gameobj, bool velocitySet, bool physicsEnableChanged){
  //std::cout << "rigid bodies old: " << world.rigidbodys.size() << std::endl;
  if (physicsEnableChanged){
    updatePhysicsBody(world, id);
  }
  //std::cout << "rigid bodies new: " << world.rigidbodys.size() << std::endl;

  auto transformation = gameobjectTransformation(world, id, false);
  physicsLocalTransformSet(world, id, transformation);
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
      .linearDamping = gameobj.physicsOptions.linearDamping,
    };
    updateRigidBodyOpts(world.physicsEnvironment, body, opts);
  }
}

void setAttributes(World& world, objid id, GameobjAttributes& attr){
  auto loadMeshObject = [&world, id](MeshData& meshdata) -> Mesh {
    return loadMesh("./res/textures/default.jpg", meshdata, [&world, id](std::string texture) -> Texture {
      return loadTextureWorld(world, texture, id);
    });    
  };

  ObjectSetAttribUtil util {
    .setEmitterEnabled = [&world, id](bool enabled) -> void {
      setEmitterEnabled(world.emitters, id, enabled);
    },
    .ensureTextureLoaded = [&world, id](std::string texturepath) -> Texture {
      return loadTextureWorld(world, texturepath, id);
    },
    .releaseTexture = [&world, id](int textureId){
      freeTextureRefsIdByOwner(world, id, textureId);
    },
    .loadMesh = loadMeshObject,
    .unloadMesh = freeMesh,
    .pathForModLayer = world.interface.modlayerPath,
  };
  auto shouldRebuildPhysics = setObjectAttributes(world.objectMapping, id, attr, util);

  GameObject& obj = getGameObject(world, id);
  bool oldPhysicsEnabled = obj.physicsOptions.enabled;
  setAllAttributes(obj, attr, util);
  bool newPhysicsEnabled = obj.physicsOptions.enabled;

  auto autoserializerFields = getObjautoserializerFields(obj.name); 
  auto additionalAttr = getAdditionalAttr(attr, autoserializerFields); // this should also filter out object attributes 
  mergeAttributes(obj.additionalAttr, additionalAttr);
  afterAttributesSet(world, id, obj, attr.vecAttr.vec3.find("physics_velocity") != attr.vecAttr.vec3.end(), oldPhysicsEnabled != newPhysicsEnabled || shouldRebuildPhysics);
}
void setProperty(World& world, objid id, std::vector<Property>& properties){
  GameObject& gameobj = getGameObject(world, id);
  GameobjAttributes attr { 
    .stringAttributes = {},
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {}},
  };
  for (auto property : properties){
    addAttributeFieldDynamic(attr, property.propertyName, property.value); 
  }
  setAttributes(world, id, attr);
}

void physicsTranslateSet(World& world, objid index, glm::vec3 pos, bool relative){
  //std::cout << "physics translate set: " << index << " relative: " << relative << std::endl;
  if (relative){
    updateRelativePosition(world.sandbox, index, pos);
    if (world.rigidbodys.find(index) != world.rigidbodys.end()){
      PhysicsValue& phys = world.rigidbodys.at(index);
      auto body =  phys.body;
      auto transform = fullTransformation(world.sandbox, index);
      setPosition(body, calcOffsetFromRotation(transform.position, phys.offset, transform.rotation));
    }
  }else{
    updateAbsolutePosition(world.sandbox, index, pos);
    if (world.rigidbodys.find(index) != world.rigidbodys.end()){
      PhysicsValue& phys = world.rigidbodys.at(index);
      auto body = phys.body;
      auto transform = fullTransformation(world.sandbox, index);
      setPosition(body, calcOffsetFromRotation(pos, phys.offset, transform.rotation));
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
      auto rigidBody = world.rigidbodys.at(index);
      auto body = rigidBody.body;
      auto transform = fullTransformation(world.sandbox, index);
      auto rot = transform.rotation;
      auto newPositionOffset = calcOffsetFromRotation(transform.position, rigidBody.offset, rot);
      setPosition(body, newPositionOffset);
      setRotation(body, rot);
    }
  }else{
    //modassert(false, "not supposed to be absolute");
    updateAbsoluteRotation(world.sandbox, index, rotation);
    if (world.rigidbodys.find(index) != world.rigidbodys.end()){
      auto rigidBody = world.rigidbodys.at(index);
      auto body =  rigidBody.body;
      auto transform = fullTransformation(world.sandbox, index);
      auto rot = transform.rotation;
      auto newPositionOffset = calcOffsetFromRotation(transform.position, rigidBody.offset, rot);
      setPosition(body, newPositionOffset);
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

  std::cout << "physics scale set: " << index << " - " <<  print(scale) << std::endl;
  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    //auto collisionInfo = getPhysicsInfoForGameObject(world, index).transformation.scale;
    //auto newScale = collisionInfo;
    auto newScale = scale;             // this should be reconsidered how this relates to transformation.scale
    auto body =  world.rigidbodys.at(index).body;
    setScale(world.physicsEnvironment, body, newScale.x, newScale.y, newScale.z);
    std::cout << "physics scale set physics: " << index << " - " << print(scale) << std::endl;
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
    setTransform(world.physicsEnvironment, body, calcOffsetFromRotation(fullTransform.position, phys.offset, fullTransform.rotation), fullTransform.scale, fullTransform.rotation);
  }
}

void updatePhysicsPositionsAndClampVelocity(World& world, std::map<objid, PhysicsValue>& rigidbodys){
  for (auto [i, rigidBody]: rigidbodys){
    GameObject& gameobj = getGameObject(world, i);
    if (!gameobj.physicsOptions.isStatic){
      auto rotation = getRotation(rigidBody.body);
      updateAbsoluteTransform(world.sandbox, i, Transformation {
        .position = calcOffsetFromRotationReverse(getPosition(rigidBody.body), rigidBody.offset, rotation),
        .scale = getScale(rigidBody.body),
        .rotation = rotation,
      });

      gameobj.physicsOptions.velocity = getVelocity(rigidBody.body);
      gameobj.physicsOptions.angularVelocity = getAngularVelocity(rigidBody.body);
    }
    clampMaxVelocity(rigidBody.body, gameobj.physicsOptions.maxspeed);
  }
}

void updateSoundPositions(World& world, Transformation& viewTransform){
  forEveryGameobj(world.sandbox, [&world, &viewTransform](objid id, GameObject& gameobj) -> void {
    auto absolutePosition = fullTransformation(world.sandbox, id).position;
    updatePosition(world.objectMapping, id, absolutePosition, viewTransform);
  });
}

void enforceLookAt(World& world){
  bool extractSceneIdFromName(std::string& name, objid* _id, std::string* _searchName);

  forEveryGameobj(world.sandbox, [&world](objid id, GameObject& gameobj) -> void {
    std::string lookAt = gameobj.lookat;                      
    if (lookAt == "" || lookAt == gameobj.name){
      return;
    }
    auto sceneId = getGameObjectH(world.sandbox, id).sceneId;

    auto parsedSceneId = 0;
    std::string parsedSearchName = "";
    auto hasParsedName = extractSceneIdFromName(lookAt, &parsedSceneId, &parsedSearchName);
    if (hasParsedName){
      sceneId = parsedSceneId;
      lookAt = parsedSearchName;
    }

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

// Should speed up attribute getting/setting, esp single attribute here  
std::optional<AttributeValue> getSingleAttribute(World& world, objid id, std::string attribute){
  auto allAttrs = objectAttributes(world, id);
  return getAttribute(allAttrs, attribute);
}

GameobjAttributes gameobjAttrFromValue(std::string& field, AttributeValue value){
  GameobjAttributes attr {
    .stringAttributes = {},
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  auto stringValue = std::get_if<std::string>(&value);
  auto floatValue = std::get_if<float>(&value);
  auto vec3Value = std::get_if<glm::vec3>(&value);
  auto vec4Value = std::get_if<glm::vec4>(&value);
  if (stringValue){
    attr.stringAttributes[field] = *stringValue;
  }else if (floatValue){
    attr.numAttributes[field] = *floatValue;
  }else if (vec3Value){
    attr.vecAttr.vec3[field] = *vec3Value;
  }else if (vec4Value){
    attr.vecAttr.vec4[field] = *vec4Value;
  }else{
    modassert(false, "invalid attribute value type");
  }
  return attr;
}

// TODO -> eliminate all the strings in the fields and use some sort of symbol system
void applyAttributeDelta(World& world, objid id, std::string field, AttributeValue delta){
  GameObject& gameobj = getGameObject(world, id);
  auto attribute = getSingleAttribute(world, id, field);
  modassert(attribute.has_value(), "attribute does not have a value: " + field);
  auto attributeSum = addAttributes(attribute.value(), delta);
  auto attrValue = gameobjAttrFromValue(field, attributeSum);

  std::cout << "apply attribute: " << field << " " << print(attributeSum) << std::endl;
  setAttributes(world, id, attrValue);
}

extern bool useTransform2;

void onWorldFrame(World& world, float timestep, float timeElapsed,  bool enablePhysics, bool dumpPhysics, bool paused, Transformation& viewTransform){
  if (!paused){
    updateEmitters(
      world.emitters, 
      timeElapsed,
      [&world](std::string name, std::string templateName, GameobjAttributes attributes, std::map<std::string, GameobjAttributes> submodelAttributes, objid emitterNodeId, NewParticleOptions particleOpts) -> std::optional<objid> {     
        if (particleOpts.parentId.has_value() && !idExists(world.sandbox, particleOpts.parentId.value())){
          return std::nullopt;
        } 
        std::cout << "INFO: emitter: creating particle from emitter: " << name << std::endl;
        attributes.vecAttr.vec3["position"] = particleOpts.position.has_value() ?  particleOpts.position.value() : fullTransformation(world.sandbox, emitterNodeId).position;
        if (particleOpts.velocity.has_value()){
          attributes.vecAttr.vec3["physics_velocity"] = particleOpts.velocity.value();
        }
        if (particleOpts.angularVelocity.has_value()){
          attributes.vecAttr.vec3["physics_avelocity"] = particleOpts.angularVelocity.value();
        }
        AttrChildrenPair attrChildren {
          .attr = attributes,
          .children = {},
        };

        // should get rid of this copying stuff, should make subelements just refer to suffix path
        auto newName =  getUniqueObjectName(templateName);
        std::map<std::string, GameobjAttributes> submodelAttributesFixedPath;
        for (auto &[name, attr] : submodelAttributes){
          submodelAttributesFixedPath[newName + "/" + name] = attr;
        }
        ////////////////////////////////////////////////////////////////////////
        
        objid objectAdded = addObjectToScene(world, getGameObjectH(world.sandbox, emitterNodeId).sceneId, newName, attrChildren, submodelAttributesFixedPath);
        if (particleOpts.orientation.has_value()){
          physicsRotateSet(world, objectAdded, particleOpts.orientation.value(), true);
        }

        auto oldTransformation = gameobjectTransformation(world, objectAdded, true);
   
        if (particleOpts.parentId.has_value()){
          modlog("emitters", "parent to id: " + std::to_string(particleOpts.parentId.value()));


          auto constraint = getGameObject(world.sandbox, objectAdded).transformation;
          std::cout << "old transform: " << print(oldTransformation) << std::endl;
          makeParent(world.sandbox, objectAdded, particleOpts.parentId.value());

          useTransform2 = true;
          //getGameObject(world.sandbox, objectAdded).transformation = oldTransformation;
          updateAbsoluteTransform(world.sandbox, objectAdded, Transformation {
            .position = oldTransformation.position,
            .scale = oldTransformation.scale,
            .rotation = oldTransformation.rotation,
          });
          useTransform2 = false;

          //auto newTransformation = gameobjectTransformation(world, objectAdded, true);
          //modassert(aboutEqual(oldTransformation.position, newTransformation.position) && aboutEqual(oldTransformation.scale, newTransformation.scale), std::string("transforms not equal: old = ") + print(oldTransformation) + ", new = " + print(newTransformation));
        }
        return objectAdded;
      }, 
      [&world](objid id) -> void { 
        std::cout << "INFO: emitter: removing particle from emitter: " << id << std::endl;
        if (idExists(world.sandbox, id)){
          removeObjectFromScene(world, id);
        }
      },
      [&world](objid id, std::string attribute, AttributeValue delta)  -> void {
        MODTODO("update particle attr -- WARNING ADD FPS INDEPNDENC HERE");
        applyAttributeDelta(world, id, attribute, delta);
      }
    );  
  }
  
  if (enablePhysics && dumpPhysics){
    dumpPhysicsInfo(world.rigidbodys);
  }
  //std::cout << "on world frame physics: " << enablePhysics << std::endl;

  stepPhysicsSimulation(world.physicsEnvironment, timestep, paused, enablePhysics);
  updatePhysicsPositionsAndClampVelocity(world, world.rigidbodys);  
  
  updateSoundPositions(world, viewTransform);
  enforceLookAt(world);   // probably should have physicsTranslateSet, so might be broken
  
  auto updatedIds = updateSandbox(world.sandbox);  
  for (auto index : updatedIds){
    auto transform = fullTransformation(world.sandbox, index);
    if (world.rigidbodys.find(index) != world.rigidbodys.end()){
      PhysicsValue& phys = world.rigidbodys.at(index);
      auto body =  phys.body;
      auto fullTransform = fullTransformation(world.sandbox, index);
      setTransform(world.physicsEnvironment, body, calcOffsetFromRotation(fullTransform.position, phys.offset, fullTransform.rotation), fullTransform.scale, fullTransform.rotation);
    }
  }
  
  callbackEntities(world);

  onObjectFrame(world.objectMapping, [&world](std::string texturepath, unsigned char* data, int textureWidth, int textureHeight) -> void {
    updateTextureDataWorld(world, texturepath, data, textureWidth, textureHeight);
  }, timeElapsed);
}


Properties getProperties(World& world, objid id){
  Properties properties {
    .transformation =  gameobjectTransformation(world, id, false),
  };
  return properties;
}
void setProperties(World& world, objid id, Properties& properties){
  physicsLocalTransformSet(world, id, properties.transformation);
}

std::string sceneFileForSceneId(World& world, objid sceneId){
  return world.sandbox.sceneIdToSceneMetadata.at(sceneId).scenefile;
}

std::optional<std::string> sceneNameForSceneId(World& world, objid sceneId){
  return world.sandbox.sceneIdToSceneMetadata.at(sceneId).name;
}

std::vector<std::string> sceneTagsForSceneId(World& world, objid sceneId){
  return world.sandbox.sceneIdToSceneMetadata.at(sceneId).tags;
}

////////////
glm::vec3 gameobjectPosition(World& world, objid id, bool isWorld){
  if (isWorld){
    return fullTransformation(world.sandbox, id).position;
  }
  return calcRelativeTransform(world.sandbox, id).position; // getGameObject(world, id).transformation.position;   // fix relative reference
}
glm::vec3 gameobjectScale(World& world, objid id, bool isWorld){
  if (isWorld){
    return fullTransformation(world.sandbox, id).scale;
  }
  return calcRelativeTransform(world.sandbox, id).scale;   // fix relative reference
}

glm::quat gameobjectRotation(World& world, objid id, bool isWorld){
  if (isWorld){
    return fullTransformation(world.sandbox, id).rotation;
  }
  return calcRelativeTransform(world.sandbox, id).rotation;  // fix relative reference
}

Transformation gameobjectTransformation(World& world, objid id, bool isWorld){
  if (isWorld){
    return fullTransformation(world.sandbox, id);
  }
  return calcRelativeTransform(world.sandbox, id);
}