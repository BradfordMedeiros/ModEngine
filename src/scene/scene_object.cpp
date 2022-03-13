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

void emit(World& world, objid id, NewParticleOptions particleOpts){
  emitNewParticle(world.emitters, id, particleOpts);
}


BoundInfo createBoundingAround(World& world, std::vector<objid> ids){
  std::vector<BoundInfo> infos;
  for (auto id : ids){
    auto transMatr = fullModelTransform(world.sandbox, id);
    auto transform = getTransformationFromMatrix(transMatr);
    auto transformedBoundInfo =  transformBoundInfo(
      getPhysicsInfoForGameObject(world, id).boundInfo, 
      transMatr
    );

    std::cout << "create bounding: " << getGameObject(world, id).name << std::endl;
    std::cout << "position: " << print(transform.position) << std::endl;
    printBoundInfo(transformedBoundInfo);
    infos.push_back(transformedBoundInfo);
  }
  auto unionBounding = getMaxUnionBoundingInfo(infos);
  auto bounding = centerBoundInfo(unionBounding); 
  bounding.zMax = 1.f;

  return bounding;
}


std::map<objid, glm::vec3> calcPositions(World& world, objid id, std::vector<std::string>& elements, objid currentSceneId, float spacing, UILayoutType layoutType){
  auto rootPosition = fullTransformation(world.sandbox, id).position; 
  auto horizontal = rootPosition.x;
  auto fixedY = rootPosition.y;
  auto vertical = rootPosition.y;
  auto fixedX = rootPosition.x;
  
  std::map<objid, glm::vec3> newPositions;

  if (layoutType == LAYOUT_HORIZONTAL){
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
    }
  }else if (layoutType == LAYOUT_VERTICAL){
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
    }  
  }else{
    std::cout << "layout type not supported" << std::endl;
    assert(false);
  }
  return newPositions;
}

glm::vec3 layoutAlignOffset(UILayoutType layoutType, UILayoutFlowType horizontal, UILayoutFlowType vertical, float halfBoundWidth, float halfBoundHeight){
  float horizontalOffset = 0.f;
  if (horizontal == UILayoutFlowNegative){
    horizontalOffset = -1 * halfBoundWidth;
  }else if (horizontal == UILayoutFlowPositive){
    horizontalOffset = halfBoundWidth;
  }
  float verticalOffset = 0.f;
  if (vertical == UILayoutFlowNegative){
    verticalOffset = -1 * halfBoundHeight;
  }else if (vertical == UILayoutFlowPositive){
    verticalOffset = halfBoundHeight;
  }
  return glm::vec3(horizontalOffset, verticalOffset, 0.f);
}

glm::vec3 layoutPositionOffset(UILayoutType layoutType, UILayoutFlowType horizontal, UILayoutFlowType vertical, float halfBoundWidth, float halfBoundHeight){
  if (layoutType == LAYOUT_HORIZONTAL){
    return glm::vec3(-1 * halfBoundWidth, 0, 0);
  }
  return glm::vec3(0, -1 * halfBoundHeight, 0);
}

void enforceLayout(World& world, objid id, GameObjectUILayout* layoutObject){
  auto layoutPos = fullTransformation(world.sandbox, id).position;
  auto layoutType = layoutObject -> type;
  auto currentSceneId = sceneId(world.sandbox, id);

  for (auto element : layoutObject -> elements){
    auto elementId = getGameObject(world.sandbox, element, currentSceneId).id;
    GameObjectUILayout* layoutObject = std::get_if<GameObjectUILayout>(&world.objectMapping.at(elementId));
    if (layoutObject != NULL){
      enforceLayout(world, elementId, layoutObject);
    }
  }


  // Figure out positions, starting from origin (layout should center elements so not quite right yet)
  auto newPositions = calcPositions(world, id, layoutObject -> elements, currentSceneId, layoutObject -> spacing, layoutType);
  std::vector<objid> elementIds;

  for (auto &[id, pos] : newPositions){
    elementIds.push_back(id);
  }
  std::cout << std::endl;
  
  auto obj = id;

  // Put elements into correct positions, so we can create a bounding box around them 
  for (auto [id, newPos] : newPositions){
    physicsTranslateSet(world, id, newPos, false);
  }

  layoutObject -> boundInfo = createBoundingAround(world, elementIds);

  auto halfBoundWidth = (layoutObject -> boundInfo.xMax - layoutObject -> boundInfo.xMin) / 2.f;
  auto halfBoundHeight = (layoutObject -> boundInfo.yMax - layoutObject -> boundInfo.yMin) / 2.f;
  std::cout << "1/2 bound width: (" << halfBoundWidth << ", " << halfBoundHeight << ")" << std::endl;

  auto offset = layoutPositionOffset(layoutType, layoutObject -> horizontal, layoutObject -> vertical, halfBoundWidth, halfBoundHeight);
  auto alignOffset = layoutAlignOffset(layoutType, layoutObject -> horizontal, layoutObject -> vertical, halfBoundWidth, halfBoundHeight);

  std::cout << "top align offset: " << print(alignOffset) << std::endl;
  // Offset all elements to the correct positions, so that they're centered
  for (auto [id, newPos] : newPositions){
    auto fullNewPos = newPos + offset  + alignOffset;
    std::cout << "setting position of " << getGameObject(world, id).name << " to: " << print(fullNewPos) << std::endl;
    physicsTranslateSet(world, id, fullNewPos, false);
    GameObjectUILayout* layoutObject = std::get_if<GameObjectUILayout>(&world.objectMapping.at(id));
    if (layoutObject != NULL){
      // Could just technically move the elements by the change from the holder position, but whatever for now
      enforceLayout(world, id, layoutObject); 
    }
  }

  layoutObject -> boundInfo.xMin -= layoutObject -> margin;
  layoutObject -> boundInfo.xMax += layoutObject -> margin;

  if (layoutObject -> minwidth.hasMinSize && layoutObject -> minwidth.type == UILayoutPercent){
    bool isMinWidth = (layoutObject -> boundInfo.xMax - layoutObject -> boundInfo.xMin) >= layoutObject -> minwidth.amount;
    if (!isMinWidth){
      float width = layoutObject -> minwidth.amount; 
      float halfWidth = width / 2.f;
      layoutObject -> boundInfo.xMin = -1 * halfWidth;;   // should this actually be centered?
      layoutObject -> boundInfo.xMax = halfWidth;
    }
  }

  layoutObject -> boundInfo.yMin -= layoutObject -> margin;
  layoutObject -> boundInfo.yMax += layoutObject -> margin;
  if (layoutObject -> minheight.hasMinSize && layoutObject -> minheight.type == UILayoutPercent){
    bool isMinHeight = (layoutObject -> boundInfo.yMax - layoutObject -> boundInfo.yMin) >= layoutObject -> minheight.amount;
    if (!isMinHeight){
      float height = layoutObject -> minheight.amount;  // 2 is fullscreen since ndi goes from (x,y) -> ((-1, 1), (-1, 1))
      float halfHeight = height / 2.f;
      layoutObject -> boundInfo.yMin = -1 * halfHeight;;
      layoutObject -> boundInfo.yMax = halfHeight;
    }
  }

  layoutObject -> boundInfo.zMin -= layoutObject -> margin;
  layoutObject -> boundInfo.zMax += layoutObject -> margin;

  std::cout << "layoutpos, offset, alignOffset : " << print(layoutPos) << " " << print(offset) << " " << print(alignOffset) << std::endl;
  layoutObject -> boundOrigin = layoutPos + alignOffset;
}

struct UILayoutAndId {
  objid id;
  GameObjectUILayout* layout;
};

std::vector<UILayoutAndId> allLayouts(World& world){
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
  return layouts;
}

// layout has redundant enforcement for nested hierachies, works, but inefficient, since will reinforce if nested
// esp if continuously done (or excessively nested i guess) should create the hierachy and only enforce each once
void enforceAllLayouts(World& world){
  auto layouts = allLayouts(world);
  for (auto &layout : layouts){
    enforceLayout(world, layout.id, layout.layout);
    if (hasPhysicsBody(world, layout.id)){
      updatePhysicsBody(world, layout.id);
    }
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