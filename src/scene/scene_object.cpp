#include "./scene_object.h"

std::vector<LightInfo> getLightInfo(World& world){
  auto lightsIndexs = getGameObjectsIndex<GameObjectLight>(world.objectMapping);
  std::vector<LightInfo> lights;
  for (int i = 0; i < lightsIndexs.size(); i++){
    auto objectId =  lightsIndexs.at(i);
    auto objectLight = world.objectMapping.at(objectId);
    auto lightObject = std::get_if<GameObjectLight>(&objectLight);

    auto lightTransform = fullTransformation(world.sandbox, objectId);
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
  auto transform = getGameObject(world, portalObject -> camera, getGameObjectH(world.sandbox, id).sceneId).transformation;
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

glm::mat4 renderPortalView(PortalInfo info, Transformation transform){
  if (!info.perspective){
    return renderView(info.cameraPos, info.cameraRotation);
  }
  auto cameraToPortalOffset = transform.position - info.portalPos;
  return glm::inverse(renderView(glm::vec3(0.f, 0.f, 0.f), info.portalRotation) *  glm::inverse(renderView(cameraToPortalOffset, transform.rotation))) * renderView(info.cameraPos, info.cameraRotation);
}

// TODO - needs to be done relative to parent, not local space
void teleportObject(World& world, objid objectId, objid portalId){
  std::cout << "teleporting object: " << objectId << std::endl;
  GameObject& gameobject = getGameObject(world, objectId);
  auto portalView = glm::inverse(renderPortalView(getPortalInfo(world, portalId), gameobject.transformation));
  auto newTransform = getTransformationFromMatrix(portalView);
  auto newPosition = newTransform.position;
  physicsTranslateSet(world, objectId, newPosition);
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

std::optional<GameObjectVoxel*> getVoxel(World& world, objid id){
  if (world.objectMapping.find(id) == world.objectMapping.end()){
    return std::nullopt;
  }
  GameObjectObj& objectVoxel = world.objectMapping.at(id);
  GameObjectVoxel* voxelObject = std::get_if<GameObjectVoxel>(&objectVoxel);
  if (voxelObject == NULL){
    return std::nullopt;
  }
  return voxelObject;
}

void handleVoxelRaycast(World& world, objid id, glm::vec3 fromPos, glm::vec3 toPosDirection){
  auto voxel = getVoxel(world, id);
  if (!voxel.has_value()){
    return;
  }
  GameObjectVoxel* voxelPtr = voxel.value();

  auto voxelPtrModelMatrix = fullModelTransform(world.sandbox, id);
  glm::vec4 fromPosModelSpace = glm::inverse(voxelPtrModelMatrix) * glm::vec4(fromPos.x, fromPos.y, fromPos.z, 1.f);
  glm::vec4 toPos =  glm::vec4(fromPos.x, fromPos.y, fromPos.z, 1.f) + glm::vec4(toPosDirection.x, toPosDirection.y, toPosDirection.z, 1.f);
  glm::vec4 toPosModelSpace = glm::inverse(voxelPtrModelMatrix) * toPos;
  glm::vec3 rayDirectionModelSpace =  toPosModelSpace - fromPosModelSpace;
  // This raycast happens in model space of voxel, so specify position + ray in voxel model space
  auto collidedVoxels = raycastVoxels(voxelPtr -> voxel, fromPosModelSpace, rayDirectionModelSpace);
  std::cout << "length is: " << collidedVoxels.size() << std::endl;
  if (collidedVoxels.size() > 0){
    auto collision = collidedVoxels.at(0);
    voxelPtr -> voxel.selectedVoxels.push_back(collision);
    applyTextureToCube(voxelPtr -> voxel, voxelPtr -> voxel.selectedVoxels, 2);
  } 
}


void applyHeightmapMasking(World& world, objid id, float amount, float uvx, float uvy, bool shouldAverage){
  auto heightmaps = getHeightmaps(world.objectMapping);
  if (heightmaps.find(id) == heightmaps.end()){
    return;
  }
  GameObjectHeightmap& hm = *heightmaps.at(id);

  int cellX = uvx * hm.heightmap.width;
  int cellY = uvy * hm.heightmap.height;
  //std::cout << "cell (" << cellX << ", " << cellY << " )" << std::endl;
  //std::cout << "uv: ( " << uvx << ", " << uvy << " )" << std::endl;  
  applyMasking(hm.heightmap, cellX, cellY, loadMask("./res/brush/ramp_5x5.png"), amount, [&world, id]() -> void { 
      // We change *data fed to bullet.
      // This can be dynamic, however according to docs min + maxHeight must fall in range. 
      // Recreating simply ensures that the min/max height is always valid. 
    updatePhysicsBody(world, id); 
  }, hm.mesh, shouldAverage);
}
void saveHeightmap(World& world, objid id){
  auto heightmaps = getHeightmaps(world.objectMapping);
  if (heightmaps.find(id) == heightmaps.end()){
    return;
  }
  GameObjectHeightmap& hm = *heightmaps.at(id);
  saveHeightmap(hm.heightmap);
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
    return fullTransformation(world.sandbox, id).position;
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

std::vector<HitObject> raycast(World& world, glm::vec3 posFrom, glm::quat direction, float maxDistance){
  return raycast(world.physicsEnvironment, world.rigidbodys, posFrom, direction, maxDistance);
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
    auto gameObjV = world.objectMapping.at(selected); 
    auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); 
    if (meshObj != NULL){
      // @TODO this is resizing based upon first mesh only, which is questionable
      auto newScale = getScaleEquivalent(meshObj -> meshesToRender.at(0).boundInfo, width, height, depth);   // this is correlated to logic in scene//getPhysicsInfoForGameObject, needs to be fixed
      std::cout << "new scale: (" << newScale.x << ", " << newScale.y << ", " << newScale.z << ")" << std::endl;
      getGameObject(world, selected).transformation.scale = newScale;
    } 
  }  
}

objid getIdForCollisionObject(World& world, const btCollisionObject* body){
  for (auto const&[id, rigidbody] : world.rigidbodys){
    if (rigidbody == body){
      return id;
    }
  }
  return -1;
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

void emit(World& world, objid id){
  emitNewParticle(world.emitters, id);
}


BoundInfo createBoundingAround(World& world, std::vector<objid> ids){
  std::vector<BoundInfo> infos;
  std::cout << "infos:----------------------" << std::endl;
  for (auto id : ids){
    auto transformedBoundInfo =  transformBoundInfo(
      getPhysicsInfoForGameObject(world, id).boundInfo, 
      fullModelTransform(world.sandbox, id)
    );
    std::cout << "bound info: " << std::endl;
    printBoundInfo(transformedBoundInfo);
    infos.push_back(transformedBoundInfo);
  }
  auto bounding = getMaxUnionBoundingInfo(infos); 
  bounding.zMax = 1.f;

  return bounding;
}


void enforceLayout(World& world, objid id, GameObjectUILayout* layoutObject, glm::vec3 layoutPos){
  auto elements = layoutObject -> elements;
  auto currentSceneId = sceneId(world.sandbox, id);
  auto spacing = layoutObject -> spacing;
  auto layoutType = layoutObject -> type;

  // Also parenting/transforms use the relative transform, so nesting (in scenegraph) can get fucked
  // Should consoilate the vertical/horizontal cases in terms of code, identical just dereffing different properties (x vs y)
  std::vector<objid> elementIds;
  std::map<objid, glm::vec3> newPositions;

  // Measure elements, and place them offset from one another
  // Starts at origin of element since cannot know how wide it is until after measuring
  if (layoutType == LAYOUT_HORIZONTAL){
    auto rootPosition = getGameObject(world.sandbox, id).transformation.position;
    auto horizontal = rootPosition.x;
    auto fixedY = rootPosition.y;
    for (int i = 0; i < elements.size(); i++){
      GameObject& obj = getGameObject(world.sandbox, elements.at(i), currentSceneId);
      auto physicsInfo = getPhysicsInfoForGameObject(world, obj.id);  
      auto boundingWidth = (physicsInfo.boundInfo.xMax - physicsInfo.boundInfo.xMin);
      auto objectWidth =  boundingWidth * physicsInfo.transformation.scale.x;
      auto left = horizontal + objectWidth / 2.f;
      auto effectiveSpacing = spacing == 0.f ? objectWidth : (objectWidth + spacing);

      glm::vec3 newPos = obj.transformation.position;
      newPos.x = left;
      newPos.y = fixedY;
      newPositions[obj.id] = newPos;
      horizontal += effectiveSpacing;
      elementIds.push_back(obj.id);
    }
  }else if (layoutType == LAYOUT_VERTICAL){
    auto rootPosition = getGameObject(world.sandbox, id).transformation.position;
    auto vertical = rootPosition.y;
    auto fixedX = rootPosition.x;
    for (int i = 0; i < elements.size(); i++){
      GameObject& obj = getGameObject(world.sandbox, elements.at(i), currentSceneId);
      auto physicsInfo = getPhysicsInfoForGameObject(world, obj.id);  
      auto boundingHeight = (physicsInfo.boundInfo.yMax - physicsInfo.boundInfo.yMin);
      auto objectHeight =  boundingHeight * physicsInfo.transformation.scale.y;
      auto top = vertical + objectHeight / 2.f;
      auto effectiveSpacing = spacing == 0.f ? objectHeight : (objectHeight + spacing);

      glm::vec3 newPos = obj.transformation.position;
      newPos.x = fixedX;
      newPos.y = top;
      newPositions[obj.id] = newPos;
      vertical += effectiveSpacing;
      elementIds.push_back(obj.id);
    }  
  }
  
  layoutObject -> boundInfo = createBoundingAround(world, elementIds);

  // Offset each element starting from the left 
  auto boundingWidth = layoutObject -> boundInfo.xMax - layoutObject -> boundInfo.xMin;
  auto boundingHeight = layoutObject -> boundInfo.yMax - layoutObject -> boundInfo.yMin;
  for (auto [id, newPos] : newPositions){
    auto offset = layoutType == LAYOUT_HORIZONTAL ? glm::vec3(boundingWidth / 2, 0.f, 0.f) : glm::vec3(0.f, boundingHeight / 2, 0.f);
    physicsTranslateSet(world, id, newPos - offset);
  }
  layoutObject -> boundInfo.xMin -= layoutObject -> margin;
  layoutObject -> boundInfo.xMax += layoutObject -> margin;
  layoutObject -> boundInfo.yMin -= layoutObject -> margin;
  layoutObject -> boundInfo.yMax += layoutObject -> margin;
  layoutObject -> boundInfo.zMin -= layoutObject -> margin;
  layoutObject -> boundInfo.zMax += layoutObject -> margin;
  layoutObject -> boundOrigin = layoutPos;
}

struct UILayoutAndId {
  objid id;
  GameObjectUILayout* layout;
};

std::vector<UILayoutAndId> layoutsSortedByOrder(World& world){
  std::vector<UILayoutAndId> layouts;
  auto layoutIndexs = getGameObjectsIndex<GameObjectUILayout>(world.objectMapping);
  for (auto id : layoutIndexs){
    GameObjectObj& obj = world.objectMapping.at(id);
    GameObjectUILayout* layoutObject = std::get_if<GameObjectUILayout>(&obj);
    layouts.push_back(UILayoutAndId{
      .id = id,
      .layout = layoutObject,
    });
  }
  std::sort(
    std::begin(layouts), 
    std::end(layouts), 
    [](UILayoutAndId layout1, UILayoutAndId layout2) { 
      return layout1.layout -> order < layout2.layout -> order; 
    }
  );
  return layouts;
}

void enforceAllLayouts(World& world){
  auto layouts = layoutsSortedByOrder(world);
  for (auto layout : layouts){
    enforceLayout(world, layout.id, layout.layout, fullTransformation(world.sandbox, layout.id).position);
    updatePhysicsBody(world, layout.id);
  }
}

void createGeneratedMesh(World& world, std::vector<glm::vec3>& face, std::vector<glm::vec3>& points, std::string destMesh){
  auto generatedMesh = generateMesh(face, points);
  loadMeshData(world, destMesh, generatedMesh);
}

std::vector<TextureAndName> worldTextures(World& world){
  std::vector<TextureAndName> textures;
  for (auto [textureName, texture] : world.textures){
    textures.push_back(TextureAndName{
      .texture = texture,
      .textureName = textureName
    });
  }
  return textures;
}

void setTexture(World& world, objid index, std::string textureName){
  if (world.textures.find(textureName) == world.textures.end()){
    loadTextureWorld(world, textureName);
  }

  auto textureId = world.textures.at(textureName).textureId;
  for (auto id : getIdsInGroup(world.sandbox, index)){
    GameObjectMesh* meshObj = std::get_if<GameObjectMesh>(&world.objectMapping.at(id));
    if (meshObj != NULL){
      meshObj -> texture.textureOverloadName = textureName;
      meshObj -> texture.textureOverloadId = textureId;       
    }

    GameObjectUIButton* buttonObj = std::get_if<GameObjectUIButton>(&world.objectMapping.at(id));
    if (buttonObj != NULL){
      buttonObj -> onTextureString = textureName;
      buttonObj -> onTexture = textureId;
      buttonObj -> offTextureString = textureName;
      buttonObj -> offTexture = textureId;
    }
  }
}