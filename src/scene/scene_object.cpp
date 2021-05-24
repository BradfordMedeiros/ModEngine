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

void enforceLayout(World& world, objid id, GameObjectUILayout* layoutObject){
  auto elements = layoutObject -> elements;
  auto currentSceneId = sceneId(world.sandbox, id);
  auto spacing = layoutObject -> spacing;
  auto layoutType = layoutObject -> type;

  // Doesn't account for rotational effects of the objects, so boundingwidth/height incorrect if object is rotated
  // Also parenting/transforms use the relative transform, so nesting (in scenegraph) can get fucked
  // Should consoilate the vertical/horizontal cases in terms of code, identical just dereffing different properties (x vs y)
  std::vector<BoundInfo> infos;
  std::vector<glm::vec3> scales;
  if (layoutType == LAYOUT_HORIZONTAL){
    auto rootPosition = getGameObject(world.sandbox, id).transformation.position;
    auto horizontal = rootPosition.x;
    auto fixedY = rootPosition.y;
    for (int i = 0; i < elements.size(); i++){
      GameObject& obj = getGameObject(world.sandbox, elements.at(i), currentSceneId);
      auto physicsInfo = getPhysicsInfoForGameObject(world, obj.id);  
      auto isCentered = !physicsInfo.boundInfo.isNotCentered;
      auto boundingWidth = (physicsInfo.boundInfo.xMax - physicsInfo.boundInfo.xMin);
      auto objectWidth =  boundingWidth * physicsInfo.transformation.scale.x;
      auto left = physicsInfo.boundInfo.isNotCentered ? horizontal : (horizontal + objectWidth / 2.f);
      auto effectiveSpacing = spacing == 0.f ? objectWidth : (objectWidth + spacing);
      obj.transformation.position.x = left;
      obj.transformation.position.y = fixedY;
      horizontal += effectiveSpacing;
      infos.push_back(physicsInfo.boundInfo);
      scales.push_back(physicsInfo.transformation.scale);
    }
  }else if (layoutType == LAYOUT_VERTICAL){
    auto rootPosition = getGameObject(world.sandbox, id).transformation.position;
    auto vertical = rootPosition.y;
    auto fixedX = rootPosition.x;
    for (int i = 0; i < elements.size(); i++){
      GameObject& obj = getGameObject(world.sandbox, elements.at(i), currentSceneId);
      auto physicsInfo = getPhysicsInfoForGameObject(world, obj.id);  
      auto isCentered = !physicsInfo.boundInfo.isNotCentered;
      auto boundingHeight = (physicsInfo.boundInfo.yMax - physicsInfo.boundInfo.yMin);
      auto objectHeight =  boundingHeight * physicsInfo.transformation.scale.y;
      auto top = physicsInfo.boundInfo.isNotCentered ? vertical : (vertical + objectHeight / 2.f);
      auto effectiveSpacing = spacing == 0.f ? objectHeight : (objectHeight + spacing);
      obj.transformation.position.x = fixedX;
      obj.transformation.position.y = top;
      vertical += effectiveSpacing;
      infos.push_back(physicsInfo.boundInfo);
      scales.push_back(physicsInfo.transformation.scale);
    }  
  }
  BoundInfo newBoundingInfo = getScaledMaxUnionBoundingInfo(infos, scales);
  layoutObject -> boundInfo = newBoundingInfo;
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
    enforceLayout(world, layout.id, layout.layout);
  }
}

