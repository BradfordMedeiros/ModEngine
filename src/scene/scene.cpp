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

std::vector<NameAndMeshObjName> getMeshesForGameobj(World& world, objid gameobjId){
  std::vector<NameAndMeshObjName> nameAndMeshObjNames;
  auto groupId = getGroupId(world.sandbox, gameobjId);
  auto allIds = groupId == gameobjId ? getIdsInGroupByObjId(world.sandbox, groupId) : std::vector<objid>({ gameobjId });
  std::cout << "1 physics : , groupId: " << groupId << ", ids " << print(allIds) << std::endl;
  for (auto id : allIds){
    auto meshesForId = getMeshesForId(world.objectMapping, id);
    auto gameobjname = getGameObject(world, id).name;
    for (int i = 0; i < meshesForId.meshes.size(); i++){
      nameAndMeshObjNames.push_back(NameAndMeshObjName {
        .objname = gameobjname,
        .meshname = meshesForId.meshNames.at(i),
        .mesh = meshesForId.meshes.at(i),
      });
    }    
  }
  return nameAndMeshObjNames;
}

glm::vec3 getOffsetForBoundInfo(BoundInfo& boundInfo, glm::vec3 scale){
  float xoffset = 0.5f * (boundInfo.xMax + boundInfo.xMin) * scale.x;
  float yoffset = 0.5f * (boundInfo.yMax + boundInfo.yMin) * scale.y;
  float zoffset = 0.5f * (boundInfo.zMax + boundInfo.zMin) * scale.z;
  return glm::vec3(xoffset, yoffset, zoffset);
}

std::optional<PhysicsInfo> getPhysicsInfoForGameObject(World& world, objid index){  
  GameObject obj = getGameObject(world.sandbox, index);
  auto gameObjV = world.objectMapping.at(index); 

  BoundInfo boundInfo = { .xMin = -1,  .xMax = 1, .yMin = -1, .yMax = 1, .zMin = -1, .zMax = 1 };
  std::optional<glm::vec3> finalOffset = std::nullopt;

  auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); 
  if (meshObj != NULL){
    std::vector<BoundInfo> boundInfos;
    auto meshObjDatas = getMeshesForGameobj(world, index);

    std::vector<Mesh*> meshes;
    for (auto &meshObjData : meshObjDatas){
      meshes.push_back(meshObjData.mesh);
    }

    for (Mesh* mesh : meshes){
      boundInfos.push_back(mesh -> boundInfo);
    }
    if (boundInfos.size() == 0){
      return std::nullopt;
    }

    auto fullTransform = fullTransformation(world.sandbox, index);
    boundInfo = getMaxUnionBoundingInfo(boundInfos);
    finalOffset = getOffsetForBoundInfo(boundInfo, fullTransform.scale);
    std::cout << "2 physics : " << obj.name << ", index = " << index << ", group = " <<  getGameObjectH(world.sandbox, index).groupId << ", size = " << meshes.size() << ", offset: " << print(finalOffset.value()) << std::endl;

  }

  auto navmeshObj = std::get_if<GameObjectNavmesh>(&gameObjV);
  if (navmeshObj != NULL){
    boundInfo = navmeshObj -> mesh.boundInfo;
  }

  auto textObj = std::get_if<GameObjectUIText>(&gameObjV);
  if (textObj != NULL){
    // textObj -> value, 1, textObj -> deltaOffset, textObj -> align, textObj -> wrap, textObj -> virtualization, &offset
    glm::vec3 offset(0.f, 0.f, 0.f);
    boundInfo = boundInfoForCenteredText(
      world.interface.fontFamilyByName(textObj -> fontFamily),
      textObj -> value,
      0,
      0,
      1000,
      textObj -> align, 
      textObj -> wrap, 
      textObj -> virtualization, 
      textObj -> cursor.cursorIndex, 
      textObj -> cursor.cursorIndexLeft, 
      textObj -> cursor.highlightLength,
      &offset
    ); 
    finalOffset = offset; 
  }

  PhysicsInfo info = {
    .boundInfo = boundInfo,
    .transformation = obj.transformation,
    .offset = finalOffset,
  };

  return info;
}

std::vector<glm::vec3> vertsForId(World& world, objid id){
  auto meshes = getMeshesForId(world.objectMapping, id).meshes;
  if (meshes.size() == 0){
    std::cout << "no meshes for: " << getGameObject(world, id).name << std::endl;
    return {};
  }
  std::vector<glm::vec3> vertPositions;
  auto vertices = readVertsFromMeshVao(*meshes.at(0));
  for (auto &vertex : vertices){
    vertPositions.push_back(vertex.position);
  }
  return vertPositions;
}

PhysicsValue addPhysicsBody(World& world, objid id, bool initialLoad){
  auto physicsOptions = getGameObject(world.sandbox, id).physicsOptions;
  if (!physicsOptions.enabled){
    return PhysicsValue { .body = NULL, .offset = std::nullopt };
  }
  auto physicsInfoOpt = getPhysicsInfoForGameObject(world, id);
  if (!physicsInfoOpt.has_value()){
    return PhysicsValue { .body = NULL, .offset = std::nullopt };
  }

  PhysicsInfo& physicsInfo = physicsInfoOpt.value();
  //std::cout << "physics info for : " << getGameObject(world, id).name << print(physicsInfo.transformation) << std::endl;
  //std::cout << "physics info bound: " << print(physicsInfo.boundInfo) << std::endl;
  //std::cout << std::endl << std::endl;

  btRigidBody* rigidBody = NULL;

  GameObjectObj& toRender = world.objectMapping.at(id);

  GameObjectOctree* octreeObj = std::get_if<GameObjectOctree>(&toRender);
  bool isOctree = octreeObj != NULL;

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

  if (isOctree){
    auto physicsShapes = getPhysicsShapes(octreeObj -> octree);
    std::cout << debugInfo(physicsShapes) << std::endl;
    rigidBody = addRigidBodyOctree(world.physicsEnvironment, physicsInfo.transformation.position, physicsInfo.transformation.rotation, physicsInfo.transformation.scale, physicsOptions.isStatic, physicsOptions.hasCollisions, opts, physicsShapes.blocks, physicsShapes.shapes);
  }else if (physicsOptions.shape == BOX || physicsOptions.shape == AUTOSHAPE){
    std::cout << "INFO: PHYSICS: ADDING BOX RIGID BODY (" << id << ")" << std::endl;
    rigidBody = addRigidBodyRect(
      world.physicsEnvironment, 
      calcOffsetFromRotation(physicsInfo.transformation.position, physicsInfo.offset, physicsInfo.transformation.rotation), 
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
      calcOffsetFromRotation(physicsInfo.transformation.position, physicsInfo.offset, physicsInfo.transformation.rotation),
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
      calcOffsetFromRotation(physicsInfo.transformation.position, physicsInfo.offset, physicsInfo.transformation.rotation),
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
      calcOffsetFromRotation(physicsInfo.transformation.position, physicsInfo.offset, physicsInfo.transformation.rotation),
      physicsInfo.transformation.rotation,
      physicsOptions.isStatic,
      physicsOptions.hasCollisions,
      physicsInfo.transformation.scale,
      opts
    );
  }else if (physicsOptions.shape == CONVEXHULL){
    auto verts = vertsForId(world, id);
    if (verts.size() == 0){
       return PhysicsValue { .body = NULL, .offset = std::nullopt };
    }
    // This is a hack, but it should be ok.  UpdatePhysicsBody really only need to apply for [voxels - octrees?] and heightmaps as of writing this
    // I don't have easy scope to the list of verts here, so I'd rather not reload the model (or really keep them in mem for no reason) just 
    // for this, which is unused.  Probably should just change the usage of the voxel/heightmap refresh code eventually.
    assert(initialLoad);
    physicsInfo.offset = glm::vec3(0.f, 0.f, 0.f);  
    std::cout << "INFO: PHYSICS: ADDING CONVEXHULL RIGID BODY" << std::endl;
    rigidBody = addRigidBodyHull(
      world.physicsEnvironment,
      verts,
      calcOffsetFromRotation(physicsInfo.transformation.position, physicsInfo.offset, physicsInfo.transformation.rotation),
      physicsInfo.transformation.rotation,
      physicsOptions.isStatic,
      physicsOptions.hasCollisions,
      physicsInfo.transformation.scale,
      opts
    );
  }else if (physicsOptions.shape == SHAPE_EXACT){
    auto verts = vertsForId(world, id);
    if (verts.size() == 0){
       return PhysicsValue { .body = NULL, .offset = std::nullopt };
    }
    assert(initialLoad);
    std::cout << "INFO: PHYSICS: ADDING SHAPE_EXACT RIGID BODY" << std::endl;
    rigidBody = addRigidBodyExact(
      world.physicsEnvironment,
      verts,
      calcOffsetFromRotation(physicsInfo.transformation.position, physicsInfo.offset, physicsInfo.transformation.rotation),
      physicsInfo.transformation.rotation,
      physicsOptions.isStatic,
      physicsOptions.hasCollisions,
      physicsInfo.transformation.scale,
      opts
    );
  }

  PhysicsValue phys {
    .body = NULL,
    .offset = physicsInfo.offset,
  };
  if (rigidBody != NULL){
    phys.body = rigidBody;
    world.rigidbodys[id] = phys;
    modlog("rigidbody", std::string("added rigid body: ") + std::to_string(id) + ", " + print((void*)rigidBody));
  }
  return phys;
}
void rmRigidBodyWorld(World& world, objid id){
  auto rigidBodyPtr = world.rigidbodys.at(id).body;
  assert(rigidBodyPtr != NULL);
  rmRigidBody(world.physicsEnvironment, rigidBodyPtr);
  modlog("rigidbody", std::string("rm rigid body: ") + std::to_string(id) + ", " + print((void*)rigidBodyPtr));
  world.rigidbodys.erase(id);
}

void updatePhysicsBody(World& world, objid id){
  bool hasRigidBody = world.rigidbodys.find(id) != world.rigidbodys.end();
  if (hasRigidBody){
    auto rigidBody = world.rigidbodys.at(id).body;
    assert(rigidBody != NULL);
    rmRigidBodyWorld(world, id);
  }
  addPhysicsBody(world, id, false);
}

void shaderSetTextureName(const char* name, unsigned int textureId);
Texture loadTextureWorld(World& world, std::string texturepath, objid ownerId){
  if (world.textures.find(texturepath) != world.textures.end()){
    world.textures.at(texturepath).owners.insert(ownerId);
    return world.textures.at(texturepath).texture;
  }
  auto texturePath = world.interface.modlayerPath(texturepath);
  if (!fileExists(texturePath) && ownerId != -1){ // root owner for default models etc, should never use default texs
    return loadTextureWorld(world, "./res/models/box/grid.png", ownerId);
  }
  Texture texture = loadTexture(texturePath);
  shaderSetTextureName(texturepath.c_str(), texture.textureId);
  world.textures[texturepath] = TextureRef {
    .owners = { ownerId },
    .texture = texture,
    .mappingTexture = std::nullopt,
  };
  return texture;
}

Texture loadTextureAtlasWorld(World& world, std::string texturepath, std::vector<std::string> atlasTextures, objid ownerId){
  if (world.textures.find(texturepath) != world.textures.end()){
    world.textures.at(texturepath).owners.insert(ownerId);
    return world.textures.at(texturepath).texture;
  }

  std::vector<std::string> textureValues;
  for (auto &atlasTexture : atlasTextures){
    auto newAtlasTexturePath = world.interface.modlayerPath(atlasTexture);
    textureValues.push_back(newAtlasTexturePath);
  }
  Texture texture = loadTextureAtlas(textureValues);
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

Texture loadTextureWorldSelection(World& world, std::string texturepath, objid ownerId, int textureWidth, int textureHeight, std::optional<objid> mappingTexture){
  std::cout << "load texture world empty: " << texturepath << std::endl;
  modassert(world.textures.find(texturepath) == world.textures.end(), "texture is already loaded: " + texturepath);
  Texture texture = loadTextureSelection(textureWidth, textureHeight);
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

void freeAnimationsForOwner(World& world, objid id){
  std::cout << "INFO: removing animations for: " << id << std::endl;
  world.animations.erase(id);
}

ModelData loadModelPath(World& world, std::string rootname, std::string modelPath){
  return loadModel(rootname, world.interface.modlayerPath(modelPath));
}

void loadMeshData(World& world, std::string meshPath, MeshData& meshData, int ownerId){
  if (world.meshes.find(meshPath) != world.meshes.end()){
    world.meshes.at(meshPath).owners.insert(ownerId);
    for (auto &textureRef : world.meshes.at(meshPath).textureRefs){
      world.textures.at(textureRef).owners.insert(ownerId);
    }
  }else{
    std::set<std::string> textureRefs = {};
    modlog("mesh", std::string("adding mesh to cache: ") + meshPath);
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

void loadModelData(World& world, std::string meshpath, int ownerId){
  modlog("load model data", meshpath);
  if (world.modelDatas.find(meshpath) != world.modelDatas.end()){
    world.modelDatas.at(meshpath).owners.insert(ownerId);
  }else{
    world.modelDatas[meshpath] = ModelDataRef {
      .owners = { ownerId },
      .modelData = loadModelCore(meshpath), 
    };
  }
}

ModelData modelDataFromCache(World& world,  std::string meshpath, std::string rootname, int ownerId){
  loadModelData(world, meshpath, ownerId);
  ModelDataCore& modelDataCore = world.modelDatas.at(meshpath).modelData;
  auto modelData = extractModel(modelDataCore, rootname);
  if (modelData.animations.size() > 0){
    world.animations[ownerId] = modelData.animations;
  }

  for (auto [meshId, meshData] : modelData.meshIdToMeshData){
    auto meshPath = nameForMeshId(meshpath, meshId);
    loadMeshData(world, meshPath, meshData, ownerId);
  } 
  return modelData;
}

ModelData modelDataFromCacheFromData(World& world, std::string meshpath, std::string rootname, int ownerId, ModelDataCore& modelDataCore){
  world.modelDatas[meshpath] = ModelDataRef {
    .owners = { ownerId },
    .modelData = modelDataCore, 
  };

  auto modelData = extractModel(modelDataCore, rootname);
  if (modelData.animations.size() > 0){
    world.animations[ownerId] = modelData.animations;
  }

  for (auto [meshId, meshData] : modelData.meshIdToMeshData){
    auto meshPath = nameForMeshId(meshpath, meshId);
    loadMeshData(world, meshPath, meshData, ownerId);
  } 
  return modelData;
}

void freeModelDataRefsByOwner(World& world, int ownerId){
  for (auto &[_, modelRef] : world.modelDatas){
    modelRef.owners.erase(ownerId);
  }
  std::vector<std::string> modelDataToFree;
  for (auto &[name, modelRef] : world.modelDatas){
    if (modelRef.owners.size() == 0){
      modelDataToFree.push_back(name);
    }
  }
  for (auto name : modelDataToFree){
    world.modelDatas.erase(name);
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

std::function<Mesh(std::string)> getCreateMeshCopy(World& world){
  return [&world](std::string meshname) -> Mesh {
    std::cout << "ensure mesh getting: " << meshname << std::endl;
    return world.meshes.at(meshname).mesh;
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
extern std::vector<AutoSerialize> octreeAutoserializer;
extern std::vector<AutoSerialize> emitterAutoserializer;
extern std::vector<AutoSerialize> navmeshAutoserializer;
extern std::vector<AutoSerialize> textAutoserializer;
extern std::vector<AutoSerialize> prefabAutoserializer;
extern std::vector<AutoSerialize> gameobjSerializer;

void assertFieldTypesUnique(){
  std::map<std::string, AttributeValueType> fieldToType = {};
  std::vector<std::vector<AutoSerialize>*> allSerializers = {
    &meshAutoserializer,
    &cameraAutoserializer,
    &portalAutoserializer,
    &soundAutoserializer,
    &lightAutoserializer,
    &octreeAutoserializer,
    &emitterAutoserializer,
    &navmeshAutoserializer,
    &textAutoserializer,
    &prefabAutoserializer,
    &gameobjSerializer,
  };
  for (auto serializer : allSerializers){
    auto allFields = serializerFieldNames(*serializer);
    for (auto &fieldname : allFields){
      auto autoserializer = getAutoserializeByField(*serializer, fieldname.c_str());
      auto serializerType = typeForSerializer(*(autoserializer.value()));
      if (fieldToType.find(fieldname) != fieldToType.end()){
        auto existingType = fieldToType.at(fieldname);
        modassert(existingType == serializerType, std::string("mixed types for: ") + fieldname);
      }
      fieldToType[fieldname] = serializerType;
    }
  }
}

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
  }else if (type == "octree"){
    return serializerFieldNames(octreeAutoserializer);
  }else if (type == "emitter"){
    return serializerFieldNames(emitterAutoserializer);
  }else if (type == "navmesh"){
    return serializerFieldNames(navmeshAutoserializer);
  }else if (type == "text"){
    return serializerFieldNames(textAutoserializer);
  }else if (type == "custom"){
    return {};
  }else if (type == "prefab"){
    return serializerFieldNames(prefabAutoserializer);
  }
  modassert(false, "autoserializer not found");
  return {};
}

std::set<std::string> allFieldNames(GameObject& gameobj){
  auto objFieldsNames = getObjautoserializerFields(gameobj.name);
  auto gameobjFields = serializerFieldNames(gameobjSerializer);
  auto allAttrs = allKeysAndAttributes(gameobj.additionalAttr);
  std::set<std::string> allFieldNames;
  for (auto &field : objFieldsNames){
    allFieldNames.insert(field);
  }
  for (auto &field : gameobjFields){
    allFieldNames.insert(field);
  }
  for (auto &attr : allAttrs){
    allFieldNames.insert(attr.field);
  }
  return allFieldNames;
};

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
  assertFieldTypesUnique();
  
  auto objectMapping = getObjectMapping();
  World world = {
    .physicsEnvironment = initPhysics(onObjectEnter, onObjectLeave, debugDrawer),
    .objectMapping = objectMapping,
    .onObjectUpdate = onObjectUpdate,
    .onObjectCreate = onObjectCreate,
    .onObjectDelete = onObjectDelete,
    .entitiesToUpdate = {},
    .sandbox = createSceneSandbox(layers, getObjautoserializerFields),
    .interface = interface,
  };

  // hackey, but createSceneSandbox adds root object with id 0 so this is needed
  std::vector<objid> idsAdded = { 0 };
  std::map<std::string, GameobjAttributes> submodelAttributes;

  auto getId = createGetUniqueObjId(idsAdded);
  addSerialObjectsToWorld(world, 0, idsAdded, getId, {{ "root", GameobjAttributesWithId { .id = idsAdded.at(0), .attr = GameobjAttributes{}}}}, submodelAttributes);

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

std::map<objid, GameobjAttributes> applyFieldsToSubelements(std::string meshName, ModelData& data, std::map<std::string, GameobjAttributes>& overrideAttributes){
  std::map<objid, GameobjAttributes> additionalFieldsMap;
  for (auto [nodeId, _] : data.nodeTransform){
    additionalFieldsMap[nodeId] = GameobjAttributes { };
  }

  for (auto [nodeId, meshListIds] : data.nodeToMeshId){
    std::vector<std::string> meshesForGameObj;
    for (auto id : meshListIds){
      auto meshRef = meshName + "=" + std::to_string(id);
      meshesForGameObj.push_back(meshRef);
    }
    if (meshesForGameObj.size() > 0){
      additionalFieldsMap.at(nodeId).attr["meshes"] = meshesForGameObj;
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
  auto idsToSerialize = includeSubmodelAttr ? getIdsInGroupByObjId(world.sandbox, getGroupId(world.sandbox, id)) : singleId;

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

void updatePhysicsFromSandbox(World& world){
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
}

std::function<Mesh(MeshData&)> createScopedLoadMesh(World& world, objid id){
  auto loadMeshObject = [&world, id](MeshData& meshdata) -> Mesh {
    return loadMesh("./res/textures/default.jpg", meshdata, [&world, id](std::string texture) -> Texture {
      return loadTextureWorld(world, texture, id);
    });    
  };
  return loadMeshObject;
}

void addObjectToWorld(
  World& world, 
  objid sceneId, 
  objid id,
  std::string name,
  std::function<objid()> getId,
  GameobjAttributes& attr,
  std::map<std::string, GameobjAttributes>& submodelAttributes
){
    auto loadMeshObject = createScopedLoadMesh(world, id);
    auto ensureTextureLoaded = [&world, id](std::string texturepath) -> Texture {
      std::cout << "Custom texture loading: " << texturepath << std::endl;
      return loadTextureWorld(world, texturepath, id);
    };
    auto ensureMeshLoaded = [&world, sceneId, id, name, getId, &attr, &submodelAttributes](std::string meshName) {
      // this assumes that the root mesh is loaded first, which i should probably cover, although it probably doesnt get hit
      modassert(meshName.size() > 0, std::string("invalid mesh name:  ") + meshName + ", name = " + name);
      modassert(isRootMeshName(meshName), "ensureMeshLoaded called on something that was not a root mesh");

      std::cout << "ensure mesh, loading: " << meshName << ", name = " << name << std::endl;
      ModelData modelData = modelDataFromCache(world, meshName, name, id);
      attr.attr["meshes"] =  meshNamesForNode(modelData, meshName, name);

      auto additionalFields = applyFieldsToSubelements(meshName, modelData, submodelAttributes); 
      auto newSerialObjs = multiObjAdd(
        world.sandbox,
        sceneId,
        id,
        0,
        modelData.childToParent,  // these need to come from the model cache
        modelData.nodeTransform, 
        modelData.names, 
        additionalFields,    
        getId,
        getObjautoserializerFields
      );
      for (auto &[name, objAttr] : newSerialObjs){
        addObjectToWorld(world, sceneId, objAttr.id, name, getId, objAttr.attr, submodelAttributes);
      }
    }; 

    auto loadScene = [&world, id, sceneId](std::string sceneFile, std::vector<Token>& addedTokens) -> objid {
      modlog("prefab load scene", std::to_string(id));
      auto newSceneId = addSceneToWorld(world, sceneFile, addedTokens, std::nullopt, std::nullopt, std::nullopt, id);
      updatePhysicsFromSandbox(world);
      return newSceneId;
    };

    ObjectTypeUtil util {
      .id = id,
      .createMeshCopy = getCreateMeshCopy(world),
      .meshes = world.meshes,
      .ensureTextureLoaded = ensureTextureLoaded,
      .releaseTexture = [&world, id](int textureId){
          freeTextureRefsIdByOwner(world, id, textureId);
      },
      .loadMesh = loadMeshObject,
      .ensureMeshLoaded = ensureMeshLoaded,
      .pathForModLayer = world.interface.modlayerPath,
      .loadScene = loadScene,
      .getCurrentTime = world.interface.getCurrentTime,
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
  std::map<std::string, GameobjAttributes>& submodelAttributes
){
  for (auto &[name, objAttr] : nameToAttr){
    // Warning: getNewObjectId will mutate the idsAdded.  
    addObjectToWorld(world, sceneId, objAttr.id, name, getNewObjectId, objAttr.attr, submodelAttributes);
  }


  for (auto &id : idsAdded){
    auto phys = addPhysicsBody(world, id, true); 
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

objid addSceneToWorldFromData(World& world, std::string sceneFileName, objid sceneId, std::string sceneData, std::optional<std::string> name, std::optional<std::vector<std::string>> tags, std::optional<objid> parentId){
  auto data = addSceneDataToScenebox(world.sandbox, sceneFileName, sceneId, sceneData, name, tags, getObjautoserializerFields, parentId);
  auto getId = createGetUniqueObjId(data.idsAdded);
  addSerialObjectsToWorld(world, sceneId, data.idsAdded, getId, data.additionalFields, data.subelementAttributes);
  return sceneId;
}

objid addSceneToWorld(World& world, std::string sceneFile, std::vector<Token>& addedTokens, std::optional<std::string> name, std::optional<std::vector<std::string>> tags, std::optional<objid> sceneId, std::optional<objid> parentId){
  auto sceneData = world.interface.readFile(sceneFile) + "\n" + serializeSceneTokens(addedTokens);  // maybe should clean this up to prevent string hackeyness
  return addSceneToWorldFromData(world, sceneFile, sceneId.has_value() ? sceneId.value() : getUniqueObjId(), sceneData, name, tags, parentId);
}

// todo verify removing data like eg clearing meshes, animations,etc
void removeObjectById(World& world, objid objectId, std::string name, std::string scriptName, bool netsynchronized, std::set<objid>& _scenesToUnload){
  modlog("removeObjectById", std::to_string(objectId));

  if (!idExists(world.sandbox, objectId)){
    return;
  }

  if (world.rigidbodys.find(objectId) != world.rigidbodys.end()){
    rmRigidBodyWorld(world, objectId);
  }

  world.interface.stopAnimation(objectId);
  removeObject(
    world.objectMapping, 
    objectId, 
    [](std::string cameraName) -> void {
      std::cout << "remove camera not yet implemented" << std::endl;
      assert(false);
    },
    [&world, &_scenesToUnload](objid sceneId) -> void {
      modlog("prefab unload scene", std::to_string(sceneId));
      _scenesToUnload.insert(sceneId);
    }
  );
  
  freeModelDataRefsByOwner(world, objectId);
  freeMeshRefsByOwner(world, objectId);
  freeTextureRefsByOwner(world, objectId);
  freeAnimationsForOwner(world, objectId);
  if (scriptName != ""){
    world.interface.unloadCScript(scriptName, objectId);
  }
}

// TODO - the logic for remove object, remove scene should be the same code path
// The way this should work is that you should query all the objects that need to be unloaded on remove
// and then just remove all of it.  I would like this called once per frame and all the scenes and ids
// batched per frame ideally, and then this can prevent onObjectDelete having a partial state of removal on that frame

void removeObjectFromScene(World& world, objid gameobjId){
  modlog("removeObjectFromScene", std::to_string(gameobjId));
  if (!idExists(world.sandbox, gameobjId)){
    //std::cout << "id does not exist: " << gameobjId << std::endl;
    //assert(false);
    return;
  }
  auto idsToRemove = idsToRemoveFromScenegraph(world.sandbox, gameobjId);
  for (auto id : idsToRemove){
    if (!idExists(world.sandbox, id)){
      continue;
    }
    auto gameobj = getGameObject(world, id);
    auto netsynchronized = gameobj.netsynchronize;
    world.onObjectDelete(id, netsynchronized);
  }
  modlog("removeObjectFromScene idsToRemove", print(idsToRemove));

  std::set<objid> scenesToUnload;
  for (auto id : idsToRemove){
    if (!idExists(world.sandbox, id)){ // needed b/c removeobjectbyid could remove other entities in scene
      continue;
    }
    auto gameobj = getGameObject(world, id);
    auto name = gameobj.name;
    auto scriptName = gameobj.script;
    auto netsynchronized = gameobj.netsynchronize;
    removeObjectById(world, id, name, scriptName, netsynchronized, scenesToUnload);
  }
  removeObjectsFromScenegraph(world.sandbox, idsToRemove);

  for (auto sceneId : scenesToUnload){
    removeSceneFromWorld(world, sceneId);
  }
}

void removeGroupFromScene(World& world, objid idInGroup){  
  modlog("removeGroupFromScene", std::to_string(idInGroup));
  if (!idExists(world.sandbox, idInGroup)){
    return;
  }
  std::cout << "removing object: " << idInGroup << " " << getGameObject(world, idInGroup).name << std::endl;
  for (auto gameobjId : getIdsInGroupByObjId(world.sandbox, idInGroup)){
    removeObjectFromScene(world, gameobjId);
  }
}

// This is a shitty side effect of the fact that you can add an object in onObjectDelete
// It's probably a good idea to consider delaying the creation of the object to the next 
// frame and then this can be removed, but then that has issues for setAttr etc fuck
// Alternatively, maybe remove the callbacks, and then maintain a list you can query in the next frame?
// idk
// maybe this isn't too bad, might need to make the listObj more efficient ? 
// just kind of gross
std::vector<objid> stableObjInScene(World& world, objid sceneId){
  const int LOOP_LIMIT = 1000;
  std::set<objid> idAlreadyCalledDelete;
  for (int i = 0; i < LOOP_LIMIT; i++){
    bool newObject = false;
    auto objectIds = listObjAndDescInScene(world.sandbox, sceneId);
    bool oneExists = false;
    for (auto objectId : objectIds){
      if (!idExists(world.sandbox, objectId)){
        continue;
      }
      oneExists = true;
      auto gameobj = getGameObject(world, objectId);
      auto netsynchronized = gameobj.netsynchronize;
      if (idAlreadyCalledDelete.count(objectId) == 0){
        newObject = true;
        world.onObjectDelete(objectId, netsynchronized);
        idAlreadyCalledDelete.insert(objectId);
      }
      if (!newObject){
        return objectIds;
      }
    }
    if (!oneExists){
      return {};
    }
  }
  modassert(false, "stableObjInScene default return  reached loop limit should not have happened");
  return {};
}

void removeSceneFromWorld(World& world, objid sceneId){
  modlog("removeSceneFromWorld", std::to_string(sceneId));
  if (!sceneExists(world.sandbox, sceneId)) {
    std::cout << "INFO: SCENE MANAGEMENT: tried to remove (" << sceneId << ") but it does not exist" << std::endl;
    return;   // @todo maybe better to throw error instead
  }

  auto objectIds = stableObjInScene(world, sceneId);
  std::set<objid> scenesToUnload;
  for (auto objectId : objectIds){
    if (!idExists(world.sandbox, objectId)){  // this is needed b/c removeobject by id can in turn end up removing other entities
      continue;
    }
    auto gameobj = getGameObject(world, objectId);
    auto name = gameobj.name;
    auto scriptName = gameobj.script;
    auto netsynchronized = gameobj.netsynchronize;
    removeObjectById(world, objectId, name, scriptName, netsynchronized, scenesToUnload);
  }
  removeObjectsFromScenegraph(world.sandbox, objectIds);
  removeScene(world.sandbox, sceneId); // this just needs to prune the scene

  for (auto sceneId : scenesToUnload){
    removeSceneFromWorld(world, sceneId);
  }
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

void createObjectForScene(World& world, objid sceneId, std::string& name, AttrChildrenPair& attrWithChildren, std::map<std::string, GameobjAttributes>& submodelAttributes){
  GameobjAttributes& attributes = attrWithChildren.attr;
  auto idAttr = objIdFromAttribute(attributes);
  objid idToAdd = idAttr.has_value() ? idAttr.value() : getUniqueObjId();

  GameObjPair gameobjPair{
    .gameobj = gameObjectFromFields(name, idToAdd, attributes, getObjautoserializerFields(name)),
  };
  std::vector<objid> idsAdded = { gameobjPair.gameobj.id }; 
  auto getId = createGetUniqueObjId(idsAdded);
  addGameObjectToScene(world.sandbox, sceneId, name, gameobjPair.gameobj, attrWithChildren.children);
  addSerialObjectsToWorld(world, sceneId, idsAdded, getId, {{ name, GameobjAttributesWithId{ .id = idToAdd, .attr = attributes }}}, submodelAttributes);
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
    attrObj.attr.attr["id"] = static_cast<float>(id);
  } 
  return SingleObjDeserialization{
    .name = serialAttrs.begin() -> first,
    .attrWithChildren = attrObj,
    .submodelAttributes = subelementAttributes,
  };
}

objid addObjectToScene(World& world, objid sceneId, std::string name, AttrChildrenPair attrWithChildren, std::map<std::string, GameobjAttributes>& submodelAttributes){
  createObjectForScene(world, sceneId, name, attrWithChildren, submodelAttributes);
  return getIdForName(world.sandbox, name, sceneId);
}

objid addObjectToScene(World& world, objid sceneId, std::string serializedObj, objid id, bool useObjId){
  std::cout << "serialized obj: " << std::endl;
  std::cout << serializedObj << std::endl << std::endl;

  auto singleObj = deserializeSingleObj(serializedObj, id, useObjId).value();
  return addObjectToScene(world, sceneId, singleObj.name, singleObj.attrWithChildren, singleObj.submodelAttributes);
}

AttributeValuePtr ptrFromAttributeValue(AttributeValue& attributeValue){
  std::string* strValue = std::get_if<std::string>(&attributeValue);
  if (strValue){
    return strValue;
  }
  glm::vec2* vec2Value = std::get_if<glm::vec2>(&attributeValue);
  if (vec2Value){
    return vec2Value;
  }
  glm::vec3* vec3Value = std::get_if<glm::vec3>(&attributeValue);
  if (vec3Value){
    return vec3Value;
  }
  glm::vec4* vec4Value = std::get_if<glm::vec4>(&attributeValue);
  if (vec4Value){
    return vec4Value;
  }
  float* floatValue = std::get_if<float>(&attributeValue);
  if (floatValue){
    return floatValue;
  }
  modassert(false, "ptrFromAttributeValue invalid value type");
  return (float*)NULL;
}

std::optional<AttributeValuePtr> getObjectAttributePtr(World& world, objid id, const char* field){
  modassert(world.sandbox.mainScene.idToGameObjects.find(id) != world.sandbox.mainScene.idToGameObjects.end(), "getObjectAttributePtr gameobj does not exist");
  modassert(world.objectMapping.find(id) != world.objectMapping.end(), std::string("getObjectAttributePtr gameobjObj does not exist: ") + std::to_string(id));

  GameObject& gameobj = getGameObject(world, id);
  auto valuePtr = getAttributePtr(gameobj, field);
  if (valuePtr.has_value()){
    return valuePtr;
  }
  
  GameObjectObj& gameobjObj = world.objectMapping.at(id);
  auto objectValuePtr = getObjectAttributePtr(gameobjObj, field);
  if (objectValuePtr.has_value()){
    return objectValuePtr;
  }
  if (gameobj.additionalAttr.attr.find(field) != gameobj.additionalAttr.attr.end()){
      return ptrFromAttributeValue(gameobj.additionalAttr.attr.at(field));
  }
  return std::nullopt;
}

// Not complete, but just complete when needed
std::optional<AttributeValue> getObjectAttribute(World& world, objid id, const char* field){
  auto attrPtr = getObjectAttributePtr(world, id, field);
  if (!attrPtr.has_value()){
    return std::nullopt;
  }

  auto vec2Ptr = std::get_if<glm::vec2*>(&attrPtr.value());
  if (vec2Ptr){
    return **vec2Ptr;
  }

  glm::vec3** vec3Ptr = std::get_if<glm::vec3*>(&attrPtr.value());
  if (vec3Ptr){
    return **vec3Ptr;
  }

  auto vec4Ptr = std::get_if<glm::vec4*>(&attrPtr.value());
  if (vec4Ptr){
    return **vec4Ptr;
  }

  auto stringPtr = std::get_if<std::string*>(&attrPtr.value());
  if (stringPtr){
    return **stringPtr;
  }


  auto floatPtr = std::get_if<float*>(&attrPtr.value());
  if (floatPtr){
    return **floatPtr;
  }

  auto boolPtr = std::get_if<bool*>(&attrPtr.value());
  if (boolPtr){
    modassert(false, "boolPtr not yet implemented");
  }


  auto uintPtr = std::get_if<uint*>(&attrPtr.value());
  if (uintPtr){ 
    return static_cast<float>(**uintPtr);
    modassert(false, "uintPtr not yet implemented");
  }

  modassert(false, std::string("getObjectAttribute not implemented for this type, field: ") + std::string(field));
  return std::nullopt;
}

void afterAttributesSet(World& world, objid id, GameObject& gameobj, bool velocitySet, bool physicsEnableChanged){
  //std::cout << "rigid bodies old: " << world.rigidbodys.size() << std::endl;
  if (physicsEnableChanged){
    modlog("physics", std::string("rebuilding for: ") + gameobj.name);
    updatePhysicsBody(world, id);
  }
  //std::cout << "rigid bodies new: " << world.rigidbodys.size() << std::endl;

  //auto transformation = gameobjectTransformation(world, id, false);
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
      .linearDamping = gameobj.physicsOptions.linearDamping,
    };
    updateRigidBodyOpts(world.physicsEnvironment, body, opts);
  }
}

void setSingleGameObjectAttr(World& world, objid id, const char* field, AttributeValue value){
  GameObject& gameobj = getGameObject(world, id);
  bool physicsEnableInitial = gameobj.physicsOptions.enabled;
  bool physicsStaticInitial = gameobj.physicsOptions.isStatic;

  auto loadMeshObject = [&world, id](MeshData& meshdata) -> Mesh {
    return loadMesh("./res/textures/default.jpg", meshdata, [&world, id](std::string texture) -> Texture {
      return loadTextureWorld(world, texture, id);
    });    
  };

  ObjectSetAttribUtil util {
    .ensureTextureLoaded = [&world, id](std::string texturepath) -> Texture {
      return loadTextureWorld(world, texturepath, id);
    },
    .releaseTexture = [&world, id](int textureId){
      freeTextureRefsIdByOwner(world, id, textureId);
    },
    .loadMesh = loadMeshObject,
    .unloadMesh = freeMesh,
    .pathForModLayer = world.interface.modlayerPath,
    .id = id,
  };

  bool setCoreAttr = false;
  bool setObjectAttr = false;

  setCoreAttr = setAttribute(gameobj, field, value, util);
  bool physicsObjectNeedsRebuild = gameobj.physicsOptions.enabled != physicsEnableInitial;
  physicsObjectNeedsRebuild = gameobj.physicsOptions.isStatic != physicsStaticInitial;

  if (!setCoreAttr){
    SetAttrFlags setAttrFlags { .rebuildPhysics = false };
    setObjectAttr = setObjectAttribute(world.objectMapping, id, field, value, util, setAttrFlags);
    physicsObjectNeedsRebuild = physicsObjectNeedsRebuild || setAttrFlags.rebuildPhysics;  
  }

  if (!setCoreAttr && !setObjectAttr){
    GameobjAttributes attr = gameobjAttrFromValue(field, value);
    mergeAttributes(gameobj.additionalAttr, attr);
  }

  bool fieldIsVelocity = std::string(field) == "physics_velocity";
  afterAttributesSet(world, id, gameobj, fieldIsVelocity, physicsObjectNeedsRebuild);
}

void applyAttributeDelta(World& world, objid id, std::string field, AttributeValue delta, float timestep){
  auto attribute = getObjectAttribute(world, id, field.c_str());
  modassert(attribute.has_value(), "attribute does not have a value: " + field);
  auto attributeSum = addAttributes(attribute.value(), timeAdjustedAttribute(delta, timestep));
  //modlog("emitter apply attribute", std::string(field) + ", adding value: " + print(delta) + ", new value: " + print(attributeSum));
  setSingleGameObjectAttr(world, id, field.c_str(), attributeSum);
}

void setAttributes(World& world, objid id, std::vector<GameobjAttribute> allAttrs){
  for (auto &attr : allAttrs){
    setSingleGameObjectAttr(world, id, attr.field.c_str(), attr.attributeValue);
  }
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

extern bool useTransform2;

void onWorldFrame(World& world, float timestep, float timeElapsed,  bool enablePhysics, bool paused, Transformation& viewTransform, bool showVisualizations){
  if (!paused){
    updateEmitters(
      getEmitterSystem(), 
      timeElapsed,
      [&world](GameobjAttributes attributes, std::map<std::string, GameobjAttributes> submodelAttributes, objid emitterNodeId, NewParticleOptions particleOpts) -> std::optional<objid> {     
        if (particleOpts.parentId.has_value() && !idExists(world.sandbox, particleOpts.parentId.value())){
          return std::nullopt;
        } 
        std::cout << "INFO: emitter: creating particle from emitter: " << getGameObject(world.sandbox, emitterNodeId).name << std::endl;
        attributes.attr["position"] = particleOpts.position.has_value() ?  particleOpts.position.value() : fullTransformation(world.sandbox, emitterNodeId).position;
        if (particleOpts.velocity.has_value()){
          attributes.attr["physics_velocity"] = particleOpts.velocity.value();
        }
        if (particleOpts.angularVelocity.has_value()){
          attributes.attr["physics_avelocity"] = particleOpts.angularVelocity.value();
        }
        AttrChildrenPair attrChildren {
          .attr = attributes,
          .children = {},
        };

        // should get rid of this copying stuff, should make subelements just refer to suffix path
        auto newName =  getUniqueObjectName("emitter");
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
          //std::cout << "old transform: " << print(oldTransformation) << std::endl;
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
      [&world, timestep](objid id, std::string attribute, AttributeValue delta)  -> void {
        MODTODO("update particle attr -- WARNING ADD FPS INDEPNDENC HERE");
        std::cout << "emitter: " << attribute << ", " << print(delta) << std::endl;
        applyAttributeDelta(world, id, attribute, delta, timestep);
      }
    );  
  }
  
  //std::cout << "on world frame physics: " << enablePhysics << std::endl;

  stepPhysicsSimulation(world.physicsEnvironment, timestep, paused, enablePhysics);
  updatePhysicsPositionsAndClampVelocity(world, world.rigidbodys);  
  
  forEveryGameobj(world.sandbox, [&world, &viewTransform](objid id, GameObject& gameobj) -> void {
    auto absolutePosition = fullTransformation(world.sandbox, id).position;
    updateObjectPositions(world.objectMapping, id, absolutePosition, viewTransform);
  });

  enforceLookAt(world);   // probably should have physicsTranslateSet, so might be broken
  updatePhysicsFromSandbox(world);
  forEveryGameobj(world.sandbox, [&world](objid id, GameObject& gameobj) -> void {
    if (id == getGroupId(world.sandbox, id) && world.entitiesToUpdate.find(id) != world.entitiesToUpdate.end()){
      world.onObjectUpdate(gameobj);
    }
  });  
  world.entitiesToUpdate.clear();

  if (showVisualizations){
    // move this into on object frame
    auto selectedOctree = getSelectedOctreeId();
    if (selectedOctree.has_value()){
      modassert(idExists(world.sandbox, selectedOctree.value()), "octree id selected but does not exist");

      //auto transformation = fullTransformation(world.sandbox, selectedOctree.value());
      //std::cout << "octree transformation: " << print(transformation) << std::endl;
      auto transform = fullModelTransform(world.sandbox, selectedOctree.value());
      GameObjectObj& toRender = world.objectMapping.at(selectedOctree.value());
      GameObjectOctree* octreeObj = std::get_if<GameObjectOctree>(&toRender);
      modassert(octreeObj, "draw selection grid onFrame not octree type");
      drawOctreeSelectionGrid(octreeObj -> octree, world.interface.drawLine, transform);
    }    
  }
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

int getNumberOfRigidBodies(World& world){
  return world.rigidbodys.size();
}