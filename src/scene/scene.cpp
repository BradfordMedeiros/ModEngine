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
  std::cout << "index is: " << index << std::endl;
  auto gameObjV = world.objectMapping.at(index); 

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

  auto uiButtonObj = std::get_if<GameObjectUIButton>(&gameObjV);
  if (uiButtonObj != NULL){
    boundInfo = uiButtonObj -> common.mesh.boundInfo;
  }

  auto layoutObj = std::get_if<GameObjectUILayout>(&gameObjV);
  if (layoutObj != NULL){
    boundInfo = layoutObj -> boundInfo;
  }

  auto textObj = std::get_if<GameObjectUIText>(&gameObjV);
  if (textObj != NULL){
    boundInfo = boundInfoForCenteredText(textObj -> value, 1, textObj -> deltaOffset); // overly coupled to drawing functions passed in, should decouple
  }

  PhysicsInfo info = {
    .boundInfo = boundInfo,
    .transformation = obj.transformation,
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
btRigidBody* addPhysicsBody(World& world, objid id, glm::vec3 initialScale, bool initialLoad, std::vector<glm::vec3> verts){
  auto groupPhysicsInfo = getPhysicsInfoForGroup(world, id);
  if (!groupPhysicsInfo.physicsOptions.enabled){
    return NULL;
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
        physicsInfo.transformation.scale,
        heightmapObj -> heightmap.minHeight,
        heightmapObj -> heightmap.maxHeight
      );
    }else if (physicsOptions.shape == BOX || (!isVoxelObj && physicsOptions.shape == AUTOSHAPE)){
      std::cout << "INFO: PHYSICS: ADDING BOX RIGID BODY (" << id << ")" << std::endl;
      rigidBody = addRigidBodyRect(
        world.physicsEnvironment, 
        physicsInfo.transformation.position, 
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
        physicsInfo.transformation.position,
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
        physicsInfo.transformation.position,
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
        physicsInfo.transformation.position,
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
        physicsInfo.transformation.position,
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
        physicsInfo.transformation.position,
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
  return rigidBody;
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
  assert(data.meshIdToMeshData.size() ==  1);
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
  SysInterface interface
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
    .sandbox = createSceneSandbox(layers),
  };

  // hackey, but createSceneSandbox adds root object with id 0 so this is needed
  addSerialObjectsToWorld(world, world.sandbox.mainScene.rootId, { 0 }, getUniqueObjId, interface, {{ "root", GameobjAttributes{}}});

  // Default meshes that are silently loaded in the background
  addMesh(world, "./res/models/ui/node.obj");
  addMesh(world, "./res/models/boundingbox/boundingbox.obj");
  addMesh(world, "./res/models/unit_rect/unit_rect.obj");
  addMesh(world, "./res/models/cone/cone.obj");
  addMesh(world, "./res/models/camera/camera.dae");
  addMesh(world, "./res/models/box/plane.dae");
  addMesh(world, "./res/models/controls/input.obj");
  addMesh(world, "./res/models/controls/unitxy.obj");

  loadSkybox(world, "./res/textures/skyboxs/desert/"); 

  std::vector<glm::vec3> face = {
    glm::vec4(0.f, 0.f, 0.f, 1.f),
    glm::vec4(1.f, 0.f, 0.f, 1.f),
    glm::vec4(0.5f, 1.f, 0.f, 1.f),
  };
  std::vector<glm::vec3> points = {
    glm::vec3(0.f, 0.f, 0.f),
    glm::vec3(0.f, 0.f, 5.f),
    glm::vec3(1.f, 0.f, 7.f),
  };
  auto generatedMesh = generateMesh(face, points);
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
  objid sceneId, 
  std::string name,
  bool shouldLoadModel, 
  std::function<objid()> getId,
  SysInterface interface,
  GameobjAttributes attr,
  std::map<objid, std::vector<glm::vec3>>& idToModelVertexs
){
    auto id = getIdForName(world.sandbox, name, sceneId);

    addObject(id, getType(name, fields), attr, world.objectMapping, world.meshes, "./res/models/ui/node.obj",
      [&world, sceneId, id, name, shouldLoadModel, getId, &attr, &interface, &idToModelVertexs](std::string meshName, std::vector<std::string> fieldsToCopy) -> bool {  // This is a weird function, it might be better considered "ensure model l"
        if (shouldLoadModel){
          ModelData data = loadModel(name, meshName); 
          idToModelVertexs[id] = getVertexsFromModelData(data);

          world.animations[id] = data.animations;
          bool hasMesh = data.nodeToMeshId.at(0).size() > 0;     // this is 0 node because we just loaded a mesh, so by definition is root node

          for (auto [meshId, meshData] : data.meshIdToMeshData){
            auto meshPath = meshName + "::" + std::to_string(meshId);
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
            addObjectToWorld(world, sceneId, name, false, getId, interface, objAttr, idToModelVertexs);
          }
          return hasMesh;
        }
        return true;   // This is basically ensure model loaded so by definition this was already loaded. 
      }, 
      [&world, id](std::string texturepath) -> Texture {
        std::cout << "Custom texture loading: " << texturepath << std::endl;
        return loadTextureWorld(world, texturepath, id);
      },
      [&world, id](std::string texturepath, unsigned char* data, int textureWidth, int textureHeight, int numChannels) -> Texture {
        return loadTextureDataWorld(world, texturepath, data, textureWidth, textureHeight, numChannels, id);
      },
      [&world, id]() -> void {
        assert(false); // think about what this should do better!
        updatePhysicsBody(world, id);
      },
      [&world, &interface, name, id](float spawnrate, float lifetime, int limit, std::map<std::string, std::string> particleFields, std::vector<EmitterDelta> deltas, bool enabled) -> void {
        addEmitter(world.emitters, name, id, interface.getCurrentTime(), limit, spawnrate, lifetime, fieldsToAttributes(particleFields), deltas, enabled);
      },
      [&world, id](MeshData& meshdata) -> Mesh {
        return loadMesh("./res/textures/default.jpg", meshdata, [&world, id](std::string texture) -> Texture {
          return loadTextureWorld(world, texture, id);
        });    
      }
    );
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
  return serializeObject(
    world.sandbox, 
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
  std::map<std::string, GameobjAttributes> nameToAttr
){
  std::map<objid, std::vector<glm::vec3>> idToModelVertexs;
  for (auto &[name, attr] : nameToAttr){
    // Warning: getNewObjectId will mutate the idsAdded.  
    addObjectToWorld(world, sceneId, name, true, getNewObjectId, interface, attr, idToModelVertexs);
  }
  for (auto id : idsAdded){
    std::vector<glm::vec3> modelVerts = {};
    if (idToModelVertexs.find(id) != idToModelVertexs.end()){
      modelVerts = idToModelVertexs.at(id);
    }
    auto physicsBody = addPhysicsBody(world, id, glm::vec3(1.f, 1.f, 1.f), true, modelVerts); 
    if (physicsBody != NULL){
      auto transform = fullTransformation(world.sandbox, id);
      setTransform(physicsBody, transform.position, transform.scale, transform.rotation);
    }  
  }

  for (auto id : idsAdded){
    auto obj = getGameObject(world, id);
    if (obj.script != ""){
      interface.loadScript(obj.script, id, sceneId);
    }
  }

  for (auto id : idsAdded){
    auto obj = getGameObject(world, id); 
    world.onObjectCreate(obj);
  }
}

objid addSceneToWorldFromData(World& world, std::string sceneFileName, objid sceneId, std::string sceneData, SysInterface interface){
  auto data = addSceneDataToScenebox(world.sandbox, sceneFileName, sceneId, sceneData);
  addSerialObjectsToWorld(world, sceneId, data.idsAdded, getUniqueObjId, interface, data.additionalFields);
  return sceneId;
}

objid addSerialObject(
  World& world, 
  objid sceneId, 
  std::string name, 
  SysInterface interface, 
  GameobjAttributes& attr, 
  std::vector<std::string> children, 
  GameObject gameobjectObj, 
  std::vector<objid>& idsAdded
){
  auto getId = [&idsAdded]() -> objid {      // kind of hackey, this could just be returned from add objects, but flow control is tricky.
    auto newId = getUniqueObjId();
    idsAdded.push_back(newId);
    return newId;
  };

  addGameObjectToScene(world.sandbox, sceneId, name, gameobjectObj, children);

  std::map<std::string, GameobjAttributes> additionalFieldsMap;
  additionalFieldsMap[name] = attr;

  addSerialObjectsToWorld(world, sceneId, idsAdded, getId, interface, additionalFieldsMap);

  return getIdForName(world.sandbox, name, sceneId);
}

objid addSceneToWorld(World& world, std::string sceneFile, SysInterface interface){
  return addSceneToWorldFromData(world, sceneFile, getUniqueObjId(), loadFile(sceneFile), interface);
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
  
  world.onObjectDelete(objectId, netsynchronized);
  freeMeshRefsByOwner(world, objectId);
  freeTextureRefsByOwner(world, objectId);
}

// this needs to also delete all children objects. 
void removeObjectFromScene(World& world, objid objectId, SysInterface interface){  
  for (auto gameobjId : getIdsInGroup(world.sandbox, objectId)){
    if (!idExists(world.sandbox, gameobjId)){
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
  return addSerialObject(world, sceneId, name, interface, attributes, attributes.children, gameobjectObj, idsAdded);
}

objid addObjectToScene(World& world, objid sceneId, std::string serializedObj, objid id, bool useObjId, SysInterface interface){
  auto tokens = parseFormat(serializedObj);
  auto serialAttrs = deserializeSceneTokens(tokens);

  std::cout << "SERIAL ATTR SIZE: " << serialAttrs.size() << std::endl;
  if (serialAttrs.size() > 1){
    std::cout << "SERIALIZATION GOT MORE THAN 1 OBJECT.  Either bad data or has child element, got " << serialAttrs.size() << std::endl;
  }
  assert(serialAttrs.size() == 1);
  
  auto name = serialAttrs.begin() -> first;
  GameobjAttributes& attrObj = serialAttrs.begin() -> second;

  std::vector<objid> idsAdded;
  auto idToAdd = useObjId ? id : getUniqueObjId();
  idsAdded.push_back(idToAdd);

  auto gameobj = gameObjectFromFields(name, idToAdd, attrObj);
  return addSerialObject(world, sceneId, name, interface, attrObj, attrObj.children, gameobj, idsAdded);
}

GameobjAttributes objectAttributes(World& world, objid id){
  GameobjAttributes attr {};
  objectAttributes(world.objectMapping, id, attr);
  getAllAttributes(getGameObject(world, id), attr);
  return attr;
}

void afterAttributesSet(World& world, objid id, GameObject& gameobj){
  assert(false); // needs to use proper absolute (or calc via relative)
  physicsTranslateSet(world, id, gameobj.transformation.position);
  std::cout << "setting new pos to : " << print(gameobj.transformation.position) << std::endl;
  physicsScaleSet(world, id, gameobj.transformation.scale);  
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
  afterAttributesSet(world, id, obj);
}
void setProperty(World& world, objid id, std::vector<Property>& properties){
  GameObject& gameobj = getGameObject(world, id);
  for (auto property : properties){
    setAttribute(gameobj, property.propertyName, property.value);
  }
}

void physicsTranslateSet(World& world, objid index, glm::vec3 pos){
  updateAbsolutePosition(world.sandbox, index, pos);

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setPosition(body, pos);
  }
  world.entitiesToUpdate.insert(index);
}

void applyPhysicsTranslation(World& world, objid index, float offsetX, float offsetY, Axis manipulatorAxis){
  auto position = fullTransformation(world.sandbox, index).position;
  physicsTranslateSet(world, index, applyTranslation(position, offsetX, offsetY, manipulatorAxis));
}

void physicsRotateSet(World& world, objid index, glm::quat rotation){
  updateAbsoluteRotation(world.sandbox, index, rotation);

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto body =  world.rigidbodys.at(index);
    setRotation(body, rotation);
  }
  world.entitiesToUpdate.insert(index);
}

void applyPhysicsRotation(World& world, objid index, float offsetX, float offsetY, Axis manipulatorAxis){
  auto currentOrientation = fullTransformation(world.sandbox, index).rotation;
  physicsRotateSet(world, index, applyRotation(currentOrientation, offsetX, offsetY, manipulatorAxis));
}

void physicsScaleSet(World& world, objid index, glm::vec3 scale){
  updateAbsoluteScale(world.sandbox, index, scale);

  if (world.rigidbodys.find(index) != world.rigidbodys.end()){
    auto collisionInfo = getPhysicsInfoForGameObject(world, index).transformation.scale;
    auto body =  world.rigidbodys.at(index);
    setScale(body, collisionInfo.x, collisionInfo.y, collisionInfo.z);
  }
  world.entitiesToUpdate.insert(index);
}

void applyPhysicsScaling(World& world, objid index, float lastX, float lastY, float offsetX, float offsetY, Axis manipulatorAxis){
  auto transform = fullTransformation(world.sandbox, index);
  physicsScaleSet(world, index, applyScaling(transform.position, transform.scale, lastX, lastY, offsetX, offsetY, manipulatorAxis));
}

void updatePhysicsPositionsAndClampVelocity(World& world, std::map<objid, btRigidBody*>& rigidbodys){
  for (auto [i, rigidBody]: rigidbodys){
    GameObject& gameobj = getGameObject(world, i);
    updateAbsoluteTransform(world.sandbox, i, Transformation {
      .position = getPosition(rigidBody),
      .scale = getScale(rigidBody),
      .rotation = getRotation(rigidBody),
    });
    clampMaxVelocity(rigidBody, gameobj.physicsOptions.maxspeed);
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
      physicsRotateSet(world, id, orientationFromPos(fromPos, targetPosition));
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

void updateAttributeDelta(World& world, objid id, std::string attribute, AttributeValue delta){
  std::cout << "Update particle diff: (" << attribute << ")" << std::endl;
  GameObject& gameobj = getGameObject(world, id);
  applyAttributeDelta(gameobj, attribute, delta);
  afterAttributesSet(world, id, gameobj);
}

void onWorldFrame(World& world, float timestep, float timeElapsed,  bool enablePhysics, bool dumpPhysics, SysInterface interface){
  updateSandbox(world.sandbox);
  updateEmitters(
    world.emitters, 
    timeElapsed, 
    [&world, &interface](std::string name, GameobjAttributes attributes, objid emitterNodeId) -> objid {      
      std::cout << "INFO: emitter: creating particle from emitter: " << name << std::endl;
      attributes.vecAttributes["position"] = fullTransformation(world.sandbox, emitterNodeId).position;
      objid objectAdded = addObjectToScene(
        world, getGameObjectH(world.sandbox, emitterNodeId).sceneId, getUniqueObjectName(), attributes, interface
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