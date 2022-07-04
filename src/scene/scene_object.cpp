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

PortalInfo getPortalInfo(World& world, objid id){
  GameObjectObj& objectPortal = world.objectMapping.at(id);
  auto portalObject = std::get_if<GameObjectPortal>(&objectPortal);
  
  auto cameraId = getGameObject(world, portalObject -> camera, getGameObjectH(world.sandbox, id).sceneId).id;
  auto cameraFullTransform = fullTransformation(world.sandbox, cameraId);
  
  auto portalGameObject = getGameObject(world, id);
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
    portals.push_back(getPortalInfo(world, portalIndexes.at(i)));
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
  auto portalView = glm::inverse(renderPortalView(getPortalInfo(world, portalId), gameobject.transformation));
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
bool isVoxel(World& world, objid id){
  GameObjectObj& obj = world.objectMapping.at(id);
  return std::get_if<GameObjectVoxel>(&obj) != NULL;
}

void handleVoxelRaycast(World& world, objid id, glm::vec3 fromPos, glm::vec3 toPosDirection, int textureId){
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
    applyTextureToCube(voxelPtr -> voxel, voxelPtr -> voxel.selectedVoxels, textureId);
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
    GameObjectObj& gameObjV = world.objectMapping.at(selected); 
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
  for (auto const&[id, physicsObj] : world.rigidbodys){
    if (physicsObj.body == body){
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

void emit(World& world, objid id, NewParticleOptions particleOpts){
  emitNewParticle(world.emitters, id, particleOpts);
}


bool isLayout(World& world, objid id){
  GameObjectUILayout* layoutObject = std::get_if<GameObjectUILayout>(&world.objectMapping.at(id));
  return (layoutObject != NULL);
}


// kind of nasty, layouts use an absolute boundinfo, where as everything else is scaled
glm::vec3 fixScaleIfLayoutType(World& world, objid id, glm::vec3 actualScale){
  if (isLayout(world, id)){
    return glm::vec3(1.f, 1.f, 1.f);
  }
  return actualScale;
}
float fixScaleIfLayoutType(World& world, objid id, float scaleDimension){
  if (isLayout(world, id)){
    return 1.f;
  }
  return scaleDimension;
}

BoundInfo createBoundingAround(World& world, std::vector<objid> ids){
  std::vector<BoundInfo> infos;
  for (auto id : ids){
    
    auto transform = fullTransformation(world.sandbox, id);    
    transform.scale = fixScaleIfLayoutType(world, id, transform.scale);
    
    auto transMatr = matrixFromComponents(transform);

    auto transformedBoundInfo =  transformBoundInfo(
      getPhysicsInfoForGameObject(world, id).boundInfo, 
      transMatr
    );

    /*std::cout << "create bounding: " << getGameObject(world, id).name << std::endl;
    std::cout << "position: " << print(transform.position) << std::endl;

    std::cout << "transformed bounding: " << std::endl;
    printBoundInfo(transformedBoundInfo);

    std::cout << "raw bounding: " << std::endl;
    printBoundInfo(getPhysicsInfoForGameObject(world, id).boundInfo);*/

    infos.push_back(transformedBoundInfo);
  }
  auto unionBounding = getMaxUnionBoundingInfo(infos);
  auto bounding = centerBoundInfo(unionBounding); 
  bounding.zMax = 1.f;

  return bounding;
}


float getScaledBoundingWidth(World& world, objid id){
  auto physicsInfo = getPhysicsInfoForGameObject(world, id);  
  auto boundingWidth = physicsInfo.boundInfo.xMax - physicsInfo.boundInfo.xMin;
  auto objectWidth =  glm::abs(boundingWidth * fixScaleIfLayoutType(world, id, physicsInfo.transformation.scale.x));  
  return objectWidth;
}
float getScaledBoundingHeight(World& world, objid id){
  auto physicsInfo = getPhysicsInfoForGameObject(world, id);  
  auto boundingHeight = (physicsInfo.boundInfo.yMax - physicsInfo.boundInfo.yMin);
  auto objectHeight = glm::abs(boundingHeight * fixScaleIfLayoutType(world, id, physicsInfo.transformation.scale.y));
  return objectHeight;
}


struct calcPositionNewPosition {
  objid id;
  glm::vec3 position;
};
std::vector<calcPositionNewPosition>  calcPositions(World& world, glm::vec3 rootPosition, std::vector<std::string>& elements, objid currentSceneId, float spacing, float minSpacing, UILayoutType layoutType, LayoutContentSpacing contentSpacing){
  //std::cout << "root position: " << print(rootPosition) << std::endl;
  auto horizontal = rootPosition.x;
  auto fixedY = rootPosition.y;
  auto vertical = rootPosition.y;
  auto fixedX = rootPosition.x;
  
  std::vector<calcPositionNewPosition> newPositions;

  if (layoutType == LAYOUT_HORIZONTAL){
    for (int i = 0; i < elements.size(); i++){
      GameObject& obj = getGameObject(world.sandbox, elements.at(i), currentSceneId);
      auto objectWidth =  getScaledBoundingWidth(world, obj.id);
      auto left = horizontal + objectWidth / 2.f;
      auto effectiveSpacing = objectWidth + spacing;
      if (effectiveSpacing < minSpacing){
        effectiveSpacing = minSpacing;
      }
      //std::cout << "(boundingWidth = " << boundingWidth << ", objectWidth = " << objectWidth << ")" << std::endl;

      glm::vec3 newPos = obj.transformation.position;
      newPos.x = left;
      newPos.y = fixedY;
      newPositions.push_back(calcPositionNewPosition{
        .id = obj.id,
        .position = newPos,
      });
      horizontal += effectiveSpacing;
    }
  }else if (layoutType == LAYOUT_VERTICAL){
    for (int i = 0; i < elements.size(); i++){
      GameObject& obj = getGameObject(world.sandbox, elements.at(i), currentSceneId);
      auto objectHeight = getScaledBoundingHeight(world, obj.id);
      auto top = vertical + objectHeight / 2.f;
      auto effectiveSpacing = objectHeight + spacing;
      if (effectiveSpacing < minSpacing){
        effectiveSpacing = minSpacing;
      }

      glm::vec3 newPos = obj.transformation.position;
      newPos.x = fixedX;
      newPos.y = top;
      newPositions.push_back(calcPositionNewPosition{
        .id = obj.id,
        .position = newPos,
      });
      vertical += effectiveSpacing;
    }  
  }else{
    std::cout << "layout type not supported" << std::endl;
    assert(false);
  }
  return newPositions;
}

/*
  float xMin, xMax;
  float yMin, yMax;
  float zMin, zMax;

*/

std::vector<calcPositionNewPosition> contentAlignPosition(World& world, std::vector<calcPositionNewPosition> positions, BoundInfo& boundInfo,  UILayoutType layoutType, LayoutContentAlignmentType contentAlign, LayoutContentSpacing contentSpacing){
  float overallWidth = boundInfo.xMax - boundInfo.xMin;
  float overallHeight = boundInfo.yMax - boundInfo.yMin;
  std::cout << "width = " << overallWidth << ", height = " << overallHeight << std::endl;

  std::vector<calcPositionNewPosition> newPositions;
  for (auto &newPosition : positions){
    auto id = newPosition.id;
    auto pos = newPosition.position;
    // if it's pack, it's just normal
    // if it's space for first/last, find the index
    // then if it's > index, add delta to it, else add nothing
    // delta is far right bounding width (not including margin) - ending element width

    auto boundingWidth = getScaledBoundingWidth(world, id);
    auto boundingHeight = getScaledBoundingHeight(world, id);
    std::cout << "name = " << getGameObject(world, id).name << ", pos = " << print(pos) << " height = " << boundingHeight << ", width = " << boundingWidth << std::endl;

    float diffX = overallWidth - boundingWidth;
    float diffY = overallHeight - boundingHeight;
    glm::vec2 alignOffset(0.f, 0.f);

    if (layoutType == LAYOUT_HORIZONTAL){
      //    newPositions[id] = glm::vec3(pos.x + (0.5f * diffX), pos.y, pos.z);
      if (contentAlign == LayoutContentAlignment_Negative){
        alignOffset.y += 0.5f * diffY;
      }else if (contentAlign == LayoutContentAlignment_Neutral){
        // do nothing
      }else if (contentAlign == LayoutContentAlignment_Positive){
        alignOffset.y -= 0.5f * diffY;
      }else{
        modassert(false, "content align not supported");
      }
    }else if (layoutType == LAYOUT_VERTICAL){
      if (contentAlign == LayoutContentAlignment_Negative){
        alignOffset.x += 0.5f * diffX;
      }else if (contentAlign == LayoutContentAlignment_Neutral){
        // do nothing
      }else if (contentAlign == LayoutContentAlignment_Positive){
        alignOffset.x -= 0.5f * diffX;
      }else{
        modassert(false, "content align not supported");
      }
    }else{
      modassert(false, "layout type - content align - not supported");
    }
    std::cout << "diff is: " << diffX << std::endl << std::endl;

    newPositions.push_back(calcPositionNewPosition{
      .id = id,
      .position = glm::vec3(pos.x + alignOffset.x, pos.y + alignOffset.y, pos.z),
    });
  }

  return newPositions;
}

glm::vec3 calcMarginOffset(GameObjectUILayout* layoutObject){
  glm::vec3 marginOffset(0.f, 0.f, 0.f);
  if (layoutObject -> alignment.horizontal == LayoutContentAlignment_Negative){
    marginOffset += glm::vec3(layoutObject -> marginValues.marginLeft, 0.f, 0.f);
  }else if (layoutObject -> alignment.horizontal == LayoutContentAlignment_Neutral){
    // do nothing
  }else if (layoutObject -> alignment.horizontal == LayoutContentAlignment_Positive){
    marginOffset += glm::vec3(-1.f * layoutObject -> marginValues.marginRight, 0.f, 0.f);
  }else{
    std::cout << "enforce layout: invalid horizontal align items" << std::endl;
    assert(false);
  }

  if (layoutObject -> alignment.vertical == LayoutContentAlignment_Negative){
    marginOffset += glm::vec3(0.f, layoutObject -> marginValues.marginBottom, 0.f);
  }else if (layoutObject -> alignment.vertical == LayoutContentAlignment_Neutral){
    // do nothing
  }else if (layoutObject -> alignment.vertical == LayoutContentAlignment_Positive){
    marginOffset += glm::vec3(0.f, -1.f * layoutObject -> marginValues.marginTop, 0.f);
  }else{
    std::cout << "enforce layout: invalid vertical align items" << std::endl;
    assert(false);
  }
  return marginOffset; 
}

// this should measure for the alignment and the margin 
// assume it's addressed from the bounding box, within margins
glm::vec3 layoutAlignItemsAdjustment(GameObjectUILayout* layoutObject, float boundingWidth, float boundingHeight, float elementsWidth, float elementsHeight){
  glm::vec3 alignItemsAdjustment(0.f, 0.f, 0.f);
  if (layoutObject -> alignment.horizontal == LayoutContentAlignment_Negative){
    // do nothing this is the default
  }else if (layoutObject -> alignment.horizontal == LayoutContentAlignment_Neutral){
    auto distanceToCenterFromLeft = boundingWidth - elementsWidth;
    alignItemsAdjustment += glm::vec3(distanceToCenterFromLeft * 0.5f, 0.f, 0.f);
  }else if (layoutObject -> alignment.horizontal == LayoutContentAlignment_Positive){
    auto remainderRight = boundingWidth - elementsWidth;
    alignItemsAdjustment += glm::vec3(remainderRight, 0.f, 0.f);
  }else{
    std::cout << "enforce layout: invalid horizontal align items" << std::endl;
    assert(false);
  }

  if (layoutObject -> alignment.vertical == LayoutContentAlignment_Negative){
    // do nothing this is the default
  }else if (layoutObject -> alignment.vertical == LayoutContentAlignment_Neutral){
    auto distanceToCenterFromBottom = boundingHeight - elementsHeight;
    alignItemsAdjustment += glm::vec3(0.f, distanceToCenterFromBottom * 0.5f, 0.f);
  }else if (layoutObject -> alignment.vertical == LayoutContentAlignment_Positive){
    auto remainderTop = boundingHeight - elementsHeight;
    alignItemsAdjustment += glm::vec3(0.f, remainderTop, 0.f);
  }else{
    std::cout << "enforce layout: invalid vertical align items" << std::endl;
    assert(false);
  }
  return alignItemsAdjustment;
}

std::vector<glm::vec3> calcAlignItemsAdjustments(GameObjectUILayout* layoutObject, 
  float boundingWidth, float boundingHeight,
  float elementsWidth, float elementsHeight,
  std::vector<calcPositionNewPosition>& newPositions
){
  std::vector<glm::vec3> alignItemsAdjustments;

  auto alignItemsAdjustment = layoutAlignItemsAdjustment(layoutObject, boundingWidth, boundingHeight, elementsWidth, elementsHeight);
  auto marginOffset = calcMarginOffset(layoutObject);

  float spaceableWidth = boundingWidth - (layoutObject -> marginValues.marginLeft + layoutObject -> marginValues.marginRight);
  float spaceableHeight = boundingHeight - (layoutObject -> marginValues.marginTop + layoutObject -> marginValues.marginBottom);

  std::cout << "boundingsize = " << boundingWidth << ", " << boundingHeight << ", spaceable = " << spaceableWidth << ", " << spaceableHeight << std::endl;
  std::cout << "elements = " << elementsWidth << ", " << elementsHeight << std::endl;

  auto rightSideRemaining = spaceableWidth - elementsWidth;
  auto heightRemaining = spaceableHeight - elementsHeight;
  std::cout << "right = " << rightSideRemaining << " height = " << heightRemaining << std::endl << std::endl;

  if (layoutObject -> contentSpacing == LayoutContentSpacing_Pack){
    for (int i = 0; i < newPositions.size(); i++){
      alignItemsAdjustments.push_back(alignItemsAdjustment + marginOffset);
    }
  }else if (layoutObject -> type == LAYOUT_HORIZONTAL){
      int spaceBetweenIndex =  INT_MAX;
      if (layoutObject -> contentSpacing == LayoutContentSpacing_SpaceForFirst){
        spaceBetweenIndex = 0;
      }else if (layoutObject -> contentSpacing == LayoutContentSpacing_SpaceForLast){
        spaceBetweenIndex = newPositions.size() - 2;
      }else{
        modassert(false, "invalid code path");
      }
      for (int i = 0; i < newPositions.size(); i++){
        glm::vec3 additionalOffset(0.f, 0.f, 0.f);
        if (i > spaceBetweenIndex){
         // additionalOffset = glm::vec3(0.2f, 0.f, 0.f);
          additionalOffset = glm::vec3(rightSideRemaining, 0.f, 0.f);
        }
  
      alignItemsAdjustments.push_back(additionalOffset + glm::vec3(0.f, alignItemsAdjustment.y, 0.f) + glm::vec3(layoutObject -> marginValues.marginLeft, marginOffset.y, 0));
    //  alignItemsAdjustments.push_back(glm::vec3(0.f, 0.f, 0.f));
    }
  }else if (layoutObject -> type == LAYOUT_VERTICAL){
    int spaceBetweenIndex =  INT_MAX;
    if (layoutObject -> contentSpacing == LayoutContentSpacing_SpaceForFirst){
      spaceBetweenIndex = 0;
    }else if (layoutObject -> contentSpacing == LayoutContentSpacing_SpaceForLast){
      spaceBetweenIndex = newPositions.size() - 2;
    }else{
      modassert(false, "invalid code path");
    }
    for (int i = 0; i < newPositions.size(); i++){
      glm::vec3 additionalOffset(0.f, 0.f, 0.f);
      if (i > spaceBetweenIndex){
       // additionalOffset = glm::vec3(0.2f, 0.f, 0.f);
        additionalOffset = glm::vec3(0.f, heightRemaining, 0.f);
      }

      alignItemsAdjustments.push_back(additionalOffset + glm::vec3(alignItemsAdjustment.x, 0.f, 0.f) + glm::vec3(marginOffset.x, layoutObject -> marginValues.marginBottom, 0));
    //  alignItemsAdjustments.push_back(glm::vec3(0.f, 0.f, 0.f));
    }
  }else{
    modassert(false, "invalid layout type");
  }


  return alignItemsAdjustments;
}

void enforceLayout(World& world, objid id, GameObjectUILayout* layoutObject){
  auto layoutType = layoutObject -> type;
  auto currentSceneId = sceneId(world.sandbox, id);

   // Set position of the layout based on anchor target
  if (layoutObject -> anchor.target != ""){
    auto anchorElement = getGameObjectByNamePrefix(world, layoutObject -> anchor.target, currentSceneId, false);
    if (anchorElement.has_value()){
      auto anchorId = anchorElement.value();
      auto anchorBoundInfo = getPhysicsInfoForGameObject(world, id).boundInfo;
      //std::cout << "minx, maxx" << anchorBoundInfo.xMin << " " << anchorBoundInfo.xMax << std::endl;
      //std::cout << "anchor target: " << layoutObject -> anchor.target << std::endl;

      auto boundDirectionOffset = layoutObject -> anchor.offset;
      if (layoutObject -> anchor.horizontal == UILayoutFlowPositive){
        boundDirectionOffset.x += anchorBoundInfo.xMax;
      }else if (layoutObject -> anchor.horizontal == UILayoutFlowNegative){
        boundDirectionOffset.x += anchorBoundInfo.xMin;
      }
      if (layoutObject -> anchor.vertical == UILayoutFlowPositive){
        boundDirectionOffset.y += anchorBoundInfo.yMax;
      }else if (layoutObject -> anchor.vertical == UILayoutFlowNegative){
        boundDirectionOffset.y += anchorBoundInfo.yMin;
      }

      //std::cout << "anchor boundoffset: " << print(boundDirectionOffset) << std::endl;
      auto anchorElementPos = fullTransformation(world.sandbox, anchorId).position + boundDirectionOffset;
      //std::cout << "anchor pos: " << print(anchorElementPos) << std::endl;
      physicsTranslateSet(world, id, anchorElementPos, false);
    
      }else{
      std::cout << "anchor target: " << layoutObject -> anchor.target << " does not exist" << std::endl;
      assert(false);
    }
  }

  //std::cout << "anchor direction: " << std::endl;
  auto anchorHorzCentered = layoutObject -> anchor.horizontal == UILayoutFlowNone;
  auto anchorVertCentered = layoutObject -> anchor.vertical == UILayoutFlowNone;

  //std::cout << "anchor centered(horz, vert) : " << (anchorHorzCentered ? "true" : "false") << " , " << (anchorVertCentered ? "true" : "false") << std::endl;
  //std::cout << "------------------------------" << std::endl;

  enforceLayoutsByName(world, layoutObject -> elements, currentSceneId);
  //std::cout << "enforcing layout: " << getGameObject(world, id).name << std::endl;

  auto layoutPos = fullTransformation(world.sandbox, id).position;
  // Figure out positions, starting from rootPosition (layout should center elements so not quite right yet)
  auto newPositions = calcPositions(world, /*root pos */ layoutPos, layoutObject -> elements, currentSceneId, layoutObject -> spacing, layoutObject -> minSpacing, layoutType, layoutObject -> contentSpacing);
  for (auto newPosition : newPositions){   // Put elements into correct positions, so we can create a bounding box around them 
    physicsTranslateSet(world, newPosition.id, newPosition.position, false);
  }

  std::vector<objid> ids;
  for (auto &newPosition : newPositions){
    ids.push_back(newPosition.id);
  }
  layoutObject -> boundInfo = createBoundingAround(world, ids);

  std::cout << "enforcing layout: " << getGameObject(world, id).name << std::endl;

  /*std::cout << "set bounding to: " << std::endl;
  printBoundInfo(layoutObject -> boundInfo);
  std::cout << std::endl;*/

  float elementsWidth = layoutObject -> boundInfo.xMax - layoutObject -> boundInfo.xMin;
  float elementsHeight = layoutObject -> boundInfo.yMax - layoutObject -> boundInfo.yMin;

  float boundingWidth = elementsWidth + layoutObject -> marginValues.marginLeft + layoutObject -> marginValues.marginRight;
  float boundingHeight = elementsHeight + layoutObject -> marginValues.marginBottom + layoutObject -> marginValues.marginTop;

  if (layoutObject -> minwidth.hasMinSize && layoutObject -> minwidth.type == UILayoutPercent){
    if (boundingWidth < layoutObject -> minwidth.amount){
      boundingWidth = layoutObject -> minwidth.amount;
    }
  }
  if (layoutObject -> minheight.hasMinSize && layoutObject -> minheight.type == UILayoutPercent){
    if (boundingHeight < layoutObject -> minheight.amount){
      boundingHeight = layoutObject -> minheight.amount;
    }
  }


  newPositions = contentAlignPosition(world, newPositions, layoutObject -> boundInfo, layoutType, layoutObject -> contentAlign, layoutObject -> contentSpacing);
  for (auto [id, newPos] : newPositions){   // Put elements into correct positions, so we can create a bounding box around them 
    physicsTranslateSet(world, id, newPos, false);
  }


  ////
  float halfElementsWidth = 0.5f * elementsWidth;
  float halfElementsHeight = 0.5f * elementsHeight;

  float halfBoundingWidth = 0.5f * boundingWidth;
  float halfBoundingHeight = 0.5f * boundingHeight;

  layoutObject -> boundInfo.xMin = -1 * halfBoundingWidth;
  layoutObject -> boundInfo.xMax = halfBoundingWidth;
  layoutObject -> boundInfo.yMin = -1 * halfBoundingHeight;
  layoutObject -> boundInfo.yMax = halfBoundingHeight;
  /////


  // Elements start from layout pos, this makes it relative to the bottom left side of bounding
  auto elementsLeftSideOffset = glm::vec3(0.f, 0.f, 0.f);
  if (layoutObject -> type == LAYOUT_HORIZONTAL){
    elementsLeftSideOffset = glm::vec3(-1 * halfBoundingWidth,  -1 * halfBoundingHeight + halfElementsHeight, 0.f);
  }else if (layoutObject -> type == LAYOUT_VERTICAL){
    elementsLeftSideOffset = glm::vec3(-1 * halfBoundingWidth + halfElementsWidth,  -1 * halfBoundingHeight, 0.f);
  }
  


  // This moves all elements and the bounding box to adjust for the a
  auto mainAlignmentOffset = glm::vec3(0.f, 0.f, 0.f);
  if (layoutObject -> horizontal == UILayoutFlowNegative){
    mainAlignmentOffset.x = -1 * halfBoundingWidth;
  }else if (layoutObject -> horizontal == UILayoutFlowPositive){
    mainAlignmentOffset.x = halfBoundingWidth;
  }
  if (layoutObject -> vertical == UILayoutFlowNegative){
    mainAlignmentOffset.y = -1 * halfBoundingHeight;
  }else if (layoutObject -> vertical == UILayoutFlowPositive){
    mainAlignmentOffset.y = halfBoundingHeight;
  }

  // Items alighment code.  Measure the bounding width / height and move from left,right  / top,down or center

  //std::cout << std::endl;

  // Applies the margin.
  std::vector<glm::vec3> alignItemsAdjustments = calcAlignItemsAdjustments(layoutObject, boundingWidth, boundingHeight, elementsWidth, elementsHeight, newPositions);

  //std::cout << "( elements(width, height) = " << print(glm::vec2(elementsWidth, elementsHeight)) << ", bounding(width, height) = " << print(glm::vec2(boundingWidth, boundingHeight)) << std::endl;
  //std::cout << "( totalAdjustment = " << print(totalAdjustment) << ", elementsLeftSideOffset = " << print(elementsLeftSideOffset) << ", marginOffset = " << print(marginOffset) << ", mainAlignmentOffset = " << print(mainAlignmentOffset) << ", alignItemsAdjustment = " << print(alignItemsAdjustment) << " ) " << std::endl;

  for (int i = 0; i < newPositions.size(); i++){
    auto newPosition = newPositions.at(i);
    auto id = newPosition.id;
    auto newPos = newPosition.position;
    auto fullNewPos = newPos + elementsLeftSideOffset + mainAlignmentOffset + alignItemsAdjustments.at(i);
    physicsTranslateSet(world, id, fullNewPos, false);
    GameObjectUILayout* layoutObject = std::get_if<GameObjectUILayout>(&world.objectMapping.at(id));
    if (layoutObject != NULL){
      // Could just technically move the elements by the change from the holder position, but whatever for now
      enforceLayout(world, id, layoutObject); 
    }
  }
  layoutObject -> panelDisplayOffset = mainAlignmentOffset;
}

void enforceLayoutsByName(World& world, std::vector<std::string>& elements, objid currentSceneId){
  for (auto element : elements){
    auto elementId = getGameObject(world.sandbox, element, currentSceneId).id;
    GameObjectUILayout* layoutObject = std::get_if<GameObjectUILayout>(&world.objectMapping.at(elementId));
    if (layoutObject != NULL){
      enforceLayout(world, elementId, layoutObject);
    }
  }
}


void enforceLayout(World& world, objid layoutId){
  GameObjectObj& obj = world.objectMapping.at(layoutId);
  GameObjectUILayout* layoutObject = std::get_if<GameObjectUILayout>(&obj);
  assert(layoutObject != NULL);
  enforceLayout(world, layoutId, layoutObject);
  if (hasPhysicsBody(world, layoutId)){
    updatePhysicsBody(world, layoutId);
  }
}

// layout has redundant enforcement for nested hierachies, works, but inefficient, since will reinforce if nested
// esp if continuously done (or excessively nested i guess) should create the hierachy and only enforce each once
void enforceAllLayouts(World& world){
  auto layoutIds = getGameObjectsIndex<GameObjectUILayout>(world.objectMapping);
  for (auto &layoutId : layoutIds){
    enforceLayout(world, layoutId);
  }
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

void setTexture(World& world, objid index, std::string textureName){
  auto textureId = loadTextureWorld(world, textureName, index).textureId;
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