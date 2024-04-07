#include "./scene_object.h"

std::vector<LightInfo> getLightInfo(World& world){
  auto lightsIndexs = getGameObjectsIndex<GameObjectLight>(world.objectMapping);
  std::vector<LightInfo> lights;
  for (int i = 0; i < lightsIndexs.size(); i++){
    auto objectId =  lightsIndexs.at(i);
    GameObjectObj& objectLight = world.objectMapping.at(objectId);
    auto lightObject = std::get_if<GameObjectLight>(&objectLight);

    auto lightTransform = fullTransformation(world.sandbox, objectId);
    LightInfo light {
      .transform = lightTransform,
      .light = *lightObject,
    };
    lights.push_back(light);
  }
  return lights;
}

std::optional<PortalInfo> getPortalInfo(World& world, objid id){
  GameObjectObj& objectPortal = world.objectMapping.at(id);
  auto portalObject = std::get_if<GameObjectPortal>(&objectPortal);
  if (portalObject -> camera == ""){
    return std::nullopt;
  }

  auto parsedSceneId = 0;
  std::string parsedSearchName = "";
  auto hasParsedName = extractSceneIdFromName(portalObject -> camera, &parsedSceneId, &parsedSearchName);
  auto sceneId = hasParsedName ? parsedSceneId : getGameObjectH(world.sandbox, id).sceneId;

  auto cameraId = getGameObject(world, portalObject -> camera, sceneId).id;
  auto cameraFullTransform = fullTransformation(world.sandbox, cameraId);
  auto portalFullTransform = fullTransformation(world.sandbox, id);

  PortalInfo info {
    .cameraTransform = cameraFullTransform,
    .portalPos = portalFullTransform.position,
    .portalRotation = portalFullTransform.rotation,
    .perspective = portalObject -> perspective,
    .id = id
  };
  return info;
}

std::vector<PortalInfo> getPortalInfo(World& world){ 
  auto portalIndexes = getGameObjectsIndex<GameObjectPortal>(world.objectMapping);
  std::vector<PortalInfo> portals;
  for (int i = 0; i < portalIndexes.size(); i++){
    auto portalInfo = getPortalInfo(world, portalIndexes.at(i));
    if (portalInfo.has_value()){
      portals.push_back(portalInfo.value());
    }
  }
  return portals;
}

bool isPortal(World& world, objid id){
  GameObjectObj& objectPortal = world.objectMapping.at(id);
  auto portalObject = std::get_if<GameObjectPortal>(&objectPortal);
  return portalObject != NULL;
}

glm::mat4 renderPortalView(PortalInfo info, Transformation transform){
  if (!info.perspective){
    return renderView(info.cameraTransform.position, info.cameraTransform.rotation);
  }
  auto cameraToPortalOffset = transform.position - info.portalPos;
  return glm::inverse(renderView(glm::vec3(0.f, 0.f, 0.f), info.portalRotation) *  glm::inverse(renderView(cameraToPortalOffset, transform.rotation))) * renderView(info.cameraTransform.position, info.cameraTransform.rotation);
}

// TODO - needs to be done relative to parent, not local space
void teleportObject(World& world, objid objectId, objid portalId){
  std::cout << "teleporting object: " << objectId << std::endl;
  assert(false);  // needs to use absolute positions
  GameObject& gameobject = getGameObject(world, objectId);
  auto portalInfo = getPortalInfo(world, portalId);
  if (!portalInfo.has_value()){
    return;
  }

  auto transformation = gameobjectTransformation(world, gameobject.id, false);
  auto portalView = glm::inverse(renderPortalView(portalInfo.value(), transformation));
  auto newTransform = getTransformationFromMatrix(portalView);
  auto newPosition = newTransform.position;
  physicsTranslateSet(world, objectId, newPosition, false);
}
void maybeTeleportObjects(World& world, objid obj1Id, objid obj2Id){
  auto obj1IsPortal = isPortal(world, obj1Id);
  auto obj2IsPortal = isPortal(world, obj2Id);
  if (obj1IsPortal && !obj2IsPortal){
    teleportObject(world, obj2Id, obj1Id);
  }else if (!obj1IsPortal && obj2IsPortal){
    teleportObject(world, obj1Id, obj2Id);
  } 
}

bool isOctree(World& world, objid id){
  GameObjectObj& obj = world.objectMapping.at(id);
  return std::get_if<GameObjectOctree>(&obj) != NULL;  
}

void applyHeightmapMasking(World& world, objid id, HeightmapMask& mask, float amount, float uvx, float uvy, bool shouldAverage, float radius){
  auto heightmaps = getHeightmaps(world.objectMapping);
  if (heightmaps.find(id) == heightmaps.end()){
    return;
  }
  GameObjectHeightmap& hm = *heightmaps.at(id);

  int cellX = uvx * hm.heightmap.width;
  int cellY = uvy * hm.heightmap.height;
  //std::cout << "cell (" << cellX << ", " << cellY << " )" << std::endl;
  //std::cout << "uv: ( " << uvx << ", " << uvy << " )" << std::endl;  
  applyMasking(hm.heightmap, cellX, cellY, mask, amount, [&world, id]() -> void { 
      // We change *data fed to bullet.
      // This can be dynamic, however according to docs min + maxHeight must fall in range. 
      // Recreating simply ensures that the min/max height is always valid. 
    updatePhysicsBody(world, id); 
  }, hm.mesh, shouldAverage);
}

GameObjectHeightmap& getHeightmap(World& world, objid id){
  auto heightmaps = getHeightmaps(world.objectMapping);
  if (heightmaps.find(id) == heightmaps.end()){
    assert(false);
  }
  GameObjectHeightmap& hm = *heightmaps.at(id);
  return hm; 
}
void saveHeightmap(World& world, objid id, std::string filepath){
  auto hm = getHeightmap(world, id);
  saveHeightmap(hm.heightmap, filepath);
}
bool isHeightmap(World& world, objid id){
  GameObjectObj& obj = world.objectMapping.at(id);
  return std::get_if<GameObjectHeightmap>(&obj) != NULL;
}

GameObjectCamera& getCamera(World& world, objid id){
  GameObjectObj& obj = world.objectMapping.at(id);
  GameObjectCamera*  cameraObj = std::get_if<GameObjectCamera>(&obj);
  assert(cameraObj != NULL);
  return *cameraObj;
}


std::string print(std::optional<objid> id){
  if (!id.has_value()){
    return "[no value]";
  }
  return std::to_string(id.value());
}

std::optional<glm::vec3> aiNavigate(World& world, objid id, glm::vec3 target, std::function<void(glm::vec3, glm::vec3)> drawLine){
  auto raycastWorld = [&world] (glm::vec3 posFrom, glm::quat direction, float maxDistance) -> std::vector<HitObject> {
    return raycast(world, posFrom, direction, maxDistance);
  };
  auto isNavmeshWorld = [&world](objid id) -> bool{ 
    return isNavmesh(world.objectMapping, id);
  };
  auto position = [&world](objid id) -> glm::vec3 {
    return fullTransformation(world.sandbox, id).position;
  };

  auto currentMeshId = targetNavmeshId(position(id), raycastWorld, isNavmeshWorld);
  auto destinationMeshId = targetNavmeshId(target, raycastWorld, isNavmeshWorld);
  bool onDestinationNavmesh = currentMeshId == destinationMeshId;

  //modlog("ai navigate : currrentMesh id", print(currentMeshId));
  //modlog("ai navigate : destinationMesh id", print(destinationMeshId));

  if (!currentMeshId.has_value()){
    //modassert(false, "current has no navmesh");
    goto simpleMoveTo;
  }
  if (!destinationMeshId.has_value()){
    //modassert(false, "destination has no navmesh");
    goto simpleMoveTo;
  }
  if (currentMeshId.value() != destinationMeshId.value()){
    auto navpath = findNavplanePath(currentMeshId.value(), destinationMeshId.value());
    if (navpath.has_value()){
      drawNavplanePath(navpath.value(), drawLine);
    }else{
      modlog("ai navpath: ", "no path");
    }


/*
    auto searchResult = aiNavSearchPath(navgraph, currentMesh, destinationMesh);
    if (!searchResult.found || searchResult.path.size() < 2){
      return position(id);
    }
    auto targetNav = searchResult.path.at(1);
    auto targetLink = aiTargetLink(navgraph, currentMesh, targetNav);
    return aiNavPosition(id, targetLink, position, raycastWorld, isNavmeshWorld);
    */
    goto simpleMoveTo;
  }


simpleMoveTo:

  return aiNavPosition(id, target, position, raycastWorld, isNavmeshWorld);
}

std::vector<HitObject> raycast(World& world, glm::vec3 posFrom, glm::quat direction, float maxDistance){
  return raycast(world.physicsEnvironment, world.rigidbodys, posFrom, direction, maxDistance);
}

std::vector<HitObject> contactTest(World& world, objid id){
  if (world.rigidbodys.find(id) == world.rigidbodys.end()){
    modlog("contact test", "warning - contact test on a non-rigidbody");
    return {};
  }
  return contactTest(world.physicsEnvironment, world.rigidbodys, world.rigidbodys.at(id).body);
}

std::optional<Texture> textureForId(World& world, objid id){
  return textureForId(world.objectMapping, id);
}

// Fn seems broken, b/c sometimes meshesToRender is 0 size
// Also probably should use generic bound info, not assume is mesh.
// ... but this functionality doesn't seem too important in reality
void setObjectDimensions(World& world, std::vector<objid>& ids, float width, float height, float depth){
  for (auto id : ids){
    auto selected = id;
    if (selected == -1 || !idExists(world.sandbox, selected)){
      return;
    }
    GameObjectObj& gameObjV = world.objectMapping.at(selected); 
    auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); 
    if (meshObj != NULL){
      // @TODO this is resizing based upon first mesh only, which is questionable
      auto newScale = getScaleEquivalent(meshObj -> meshesToRender.at(0).boundInfo, width, height, depth);   // this is correlated to logic in scene//getPhysicsInfoForGameObject, needs to be fixed
      std::cout << "new scale: (" << newScale.x << ", " << newScale.y << ", " << newScale.z << ")" << std::endl;
      physicsScaleSet(world, selected, newScale);
    } 
  }  
}

std::optional<objid> getIdForCollisionObject(World& world, const btCollisionObject* body){
  for (auto const&[id, physicsObj] : world.rigidbodys){
    if (physicsObj.body == body){
      modassert(idExists(world.sandbox, id), "get id for collision, id does not exist: " + std::to_string(id) + " but the rigid body still does");
      return id;
    }
  }
  return std::nullopt;
}

bool idInGroup(World& world, objid id, std::vector<objid> groupIds){
  auto groupId = getGroupId(world.sandbox, id);
  for (auto gId : groupIds){
    if (groupId == gId){
      return true;
    }
  }
  return false;
}

void emit(World& world, objid id, NewParticleOptions particleOpts){
  emitNewParticle(world.emitters, id, particleOpts);
}


void createGeneratedMesh(World& world, std::vector<glm::vec3>& face, std::vector<glm::vec3>& points, std::string destMesh){
  auto generatedMesh = generateMesh(face, points);
  loadMeshData(world, destMesh, generatedMesh, -1); // -1 since this mesh doesn't belong to an object, which it probably should.
}

std::vector<TextureAndName> worldTextures(World& world){
  std::vector<TextureAndName> textures;
  for (auto [textureName, texture] : world.textures){
    textures.push_back(TextureAndName{
      .texture = texture.texture,
      .textureName = textureName
    });
  }
  return textures;
}

std::optional<std::string> lookupNormalTexture(World& world, std::string textureName){
  if (!world.interface.modlayerFileExists(textureName)){
    return std::nullopt;
  }
  std::filesystem::path textureFile = std::filesystem::canonical(textureName);
  auto folder = textureFile.parent_path();
  auto newFileName = textureFile.stem().string() + ".normal" + textureFile.extension().string();
  std::filesystem::path fullNormalPath = std::filesystem::weakly_canonical(folder / newFileName); //  / is append operator 
  modlog("editor", "normal fullfilepath is: " + fullNormalPath.string());
  if (!world.interface.modlayerFileExists(fullNormalPath)){
    return std::nullopt;
  }
  return fullNormalPath;
}

void setTexture(World& world, objid index, std::string textureName, std::function<void(unsigned int)> setNavmeshTextureId){
  auto textureId = loadTextureWorld(world, textureName, index).textureId;
  auto normalTextureName = lookupNormalTexture(world, textureName);
  std::optional<Texture> normalTexture = std::nullopt;
  if (normalTextureName.has_value()){
    normalTexture = loadTextureWorld(world, normalTextureName.value(), index);
  }

  for (auto id : getIdsInGroupByObjId(world.sandbox, index)){
    GameObjectMesh* meshObj = std::get_if<GameObjectMesh>(&world.objectMapping.at(id));
    if (meshObj != NULL){
      meshObj -> texture.loadingInfo.textureString = textureName;
      meshObj -> texture.loadingInfo.textureId = textureId;

      if (normalTexture.has_value()){
        meshObj -> normalTexture.textureString = normalTextureName.value();
        meshObj -> normalTexture.textureId = normalTexture.value().textureId;
      }   
    }

    GameObjectHeightmap* heightmapObj = std::get_if<GameObjectHeightmap>(&world.objectMapping.at(id));
    if (heightmapObj != NULL){
      heightmapObj -> texture.loadingInfo.textureString = textureName;
      heightmapObj -> texture.loadingInfo.textureId = textureId;       
    }

    GameObjectNavmesh* navmeshObj = std::get_if<GameObjectNavmesh>(&world.objectMapping.at(id));
    if (navmeshObj != NULL){
      setNavmeshTextureId(textureId);
    }
  }
}

bool isPrefab(World& world, objid id){
  GameObjectObj& objectPrefab = world.objectMapping.at(id);
  auto prefabObject = std::get_if<GameObjectPrefab>(&objectPrefab);
  return prefabObject != NULL;
}

std::optional<objid> prefabId(World& world, objid id){
  if (isPrefab(world, id)){
    return id;
  }

  // maybe add an assertion here that the scene is a prefab scene? 
  auto parentObjId = listParentObjId(world.sandbox, sceneId(world.sandbox, id));
  if (!parentObjId.has_value()){
    return std::nullopt;
  }
  if (!isPrefab(world, parentObjId.value())){
    return std::nullopt;
  }
  return parentObjId; 
}