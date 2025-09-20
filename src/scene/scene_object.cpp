#include "./scene_object.h"

std::vector<std::string> listLightTextures();

std::vector<LightInfo> getLightInfo(World& world){
  auto lightsIndexs = getAllLightsIndexs(world.objectMapping);
  std::vector<LightInfo> lights;
  for (int i = 0; i < lightsIndexs.size(); i++){
    auto objectId =  lightsIndexs.at(i);
    auto lightObject = getLight(world.objectMapping, objectId);
    modassert(lightObject, "getLightInfo - not a light");
    auto lightTransform = fullTransformation(world.sandbox, objectId, "getLightInfo");

    auto rotateMatrix = matrixFromComponents(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f), lightTransform.rotation);

    static auto lightTextures = listLightTextures();
    int lightTextureIndex = -1;
    for (int i = 0; i < lightTextures.size(); i++){
      if (lightObject -> texture == lightTextures.at(i)){
        lightTextureIndex = i;
      }
    }

    int numImagesWide = calculateAtlasImageDimension(lightTextures.size());

    int xIndex = lightTextureIndex % numImagesWide;
    int yIndex = lightTextureIndex / numImagesWide;

    float width = 1.f / numImagesWide;
    float height = 1.f / numImagesWide;


    float xOffset = xIndex * width;
    float yOffset = yIndex * height;

    if (lightTextureIndex == -1){
      xOffset = -5.f;
    }

    LightInfo light {
      .transform = lightTransform,
      .transformMatrix = rotateMatrix,
      .light = *lightObject,
      .id = objectId,
      .textureCoords = glm::vec4(xOffset, yOffset, 0.25f, 0.25f),
    };
    lights.push_back(light);
  }
  return lights;
}

// These values are tied to whats in the shader
int getLightsArrayIndex(std::vector<LightInfo>& lights, objid lightId){
  for (int i = 0; i < lights.size(); i++){
    if (lights.at(i).id == lightId){
      return i;
    }
  }
  if (lightId == -2){
    return -2;
  }
  return -1;
}

void recalculateLighting(World& world){
  std::vector<LightUpdate> updates;
  updates.push_back(LightUpdate{
    .lightIndex = 0,
    .position = glm::vec3(0.f, 0.f, 0.f),
    .radius = 2,
  });

  auto lightIndexs = getAllLightsIndexs(world.objectMapping);
  for (auto id : lightIndexs){
    auto lightObject = getLight(world.objectMapping, id);
    auto lightTransform = fullTransformation(world.sandbox, id, "recalculateLighting");
    updates.push_back(LightUpdate{
      .lightIndex = id,
      .position = lightTransform.position,
      .radius = lightObject -> voxelSize,
    });
  }

  recalculateLights(updates);
}


std::optional<PortalInfo> getPortalInfo(World& world, objid id){
  auto portalObject = getPortal(world.objectMapping, id);
  modassert(portalObject, "not a portal");
  if (portalObject -> camera == ""){
    return std::nullopt;
  }
  auto sceneId = getGameObjectH(world.sandbox, id).sceneId;
  auto cameraId = getGameObject(world, portalObject -> camera, sceneId).id;
  auto cameraFullTransform = fullTransformation(world.sandbox, cameraId, "getPortalInfo cameraFullTransform");
  auto portalFullTransform = fullTransformation(world.sandbox, id, "getPortalInfo portalFullTransform");

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
  auto portalIndexes = getAllPortalIndexs(world.objectMapping);
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
  auto portalObject = getPortal(world.objectMapping, id);
  return portalObject != NULL;
}

glm::mat4 renderPortalView(PortalInfo info, Transformation transform){
  if (!info.perspective){
    return renderView(info.cameraTransform.position, info.cameraTransform.rotation);
  }
  auto cameraToPortalOffset = transform.position - info.portalPos;
  return glm::inverse(renderView(glm::vec3(0.f, 0.f, 0.f), info.portalRotation) *  glm::inverse(renderView(cameraToPortalOffset, transform.rotation))) * renderView(info.cameraTransform.position, info.cameraTransform.rotation);
}

bool isOctree(World& world, objid id){
  return getOctree(world.objectMapping, id) != NULL;
}

GameObjectCamera& getCamera(World& world, objid id){
  GameObjectCamera*  cameraObj = getCameraObj(world.objectMapping, id);
  assert(cameraObj != NULL);
  return *cameraObj;
}


std::optional<glm::vec3> aiNavigate(World& world, objid id, glm::vec3 target, std::function<void(glm::vec3, glm::vec3)> drawLine){
  auto raycastWorld = [&world] (glm::vec3 posFrom, glm::quat direction, float maxDistance) -> std::vector<HitObject> {
    return raycast(world, posFrom, direction, maxDistance, std::nullopt);
  };
  auto isNavmeshWorld = [&world](objid id) -> bool{ 
    return isNavmesh(world.objectMapping, id);
  };
  auto position = [&world](objid id) -> glm::vec3 {
    return fullTransformation(world.sandbox, id, "aiNavigate").position;
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

std::vector<HitObject> raycast(World& world, glm::vec3 posFrom, glm::quat direction, float maxDistance, std::optional<int> mask){
  return raycast(world.physicsEnvironment, world.rigidbodys, posFrom, direction, maxDistance, mask);
}

std::vector<HitObject> contactTest(World& world, objid id){
  if (world.rigidbodys.find(id) == world.rigidbodys.end()){
    modlog("contact test", "warning - contact test on a non-rigidbody");
    return {};
  }
  return contactTest(world.physicsEnvironment, world.rigidbodys, world.rigidbodys.at(id).body);
}

std::optional<Texture> textureForId(World& world, objid id){
  return textureForId(world.objectMapping, id, getObjTypeLookup(world.sandbox, id));
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
    auto meshObj = getMesh(world.objectMapping, selected, getObjTypeLookup(world.sandbox, selected));
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
  emitNewParticle(getEmitterSystem(), id, particleOpts);
}


void createGeneratedMesh(World& world, std::vector<glm::vec3>& face, std::vector<glm::vec3>& points, std::string destMesh){
  auto generatedMesh = generateMesh(face, points);
  ModelDataCore modelDataCore {
    .modelData = ModelData {
      .meshIdToMeshData = {{ 0, generatedMesh }},
      .nodeToMeshId = {{ 0, { 0 }}},
      .childToParent = {},
      .nodeTransform = {{ 0, Transformation {
          .position = glm::vec3(0.f, 0.f, 0.f),
          .scale = glm::vec3(1.f, 1.f, 1.f),
          .rotation = MOD_ORIENTATION_FORWARD,
        }}
      },
      .names = {{ 0, "test" }},
      .animations = {},      
    },
    .loadedRoot = "test",
  };
  modelDataFromCacheFromData(world, destMesh, "testrootname", -1, modelDataCore);
}

void createGeneratedMeshRaw(World& world, std::vector<glm::vec3>& verts, std::vector<glm::vec2>& uvCoords, std::vector<unsigned int>& indexs, std::string destMesh){
  auto generatedMesh = generateMeshRaw(verts, uvCoords, indexs);
  ModelDataCore modelDataCore {
    .modelData = ModelData {
      .meshIdToMeshData = {{ 0, generatedMesh }},
      .nodeToMeshId = {{ 0, { 0 }}},
      .childToParent = {},
      .nodeTransform = {{ 0, Transformation {
          .position = glm::vec3(0.f, 0.f, 0.f),
          .scale = glm::vec3(1.f, 1.f, 1.f),
          .rotation = MOD_ORIENTATION_FORWARD,
        }}
      },
      .names = {{ 0, "test" }},
      .animations = {},      
    },
    .loadedRoot = "test",
  };
  modelDataFromCacheFromData(world, destMesh, "testrootname", -1, modelDataCore);
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
  std::filesystem::path textureFile(textureName);
  auto folder = textureFile.parent_path();
  auto newFileName = textureFile.stem().string() + ".normal" + textureFile.extension().string();
  std::filesystem::path fullNormalPath = folder / newFileName; //  / is append operator 
  modlog("editor", "normal fullfilepath is: " + fullNormalPath.string() + ", orig was: " + textureName);
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
    GameObjectMesh* meshObj = getMesh(world.objectMapping, id, getObjTypeLookup(world.sandbox, id));
    if (meshObj != NULL){
      meshObj -> texture.loadingInfo.textureString = textureName;
      meshObj -> texture.loadingInfo.textureId = textureId;

      if (normalTexture.has_value()){
        meshObj -> normalTexture.textureString = normalTextureName.value();
        meshObj -> normalTexture.textureId = normalTexture.value().textureId;
      }else {
        meshObj -> normalTexture.textureString = "";
        meshObj -> normalTexture.textureId = -1;
      }
    }

    GameObjectNavmesh* navmeshObj = getNavmesh(world.objectMapping, id);
    if (navmeshObj != NULL){
      setNavmeshTextureId(textureId);
    }
  }
}

bool isPrefab(World& world, objid id){
  auto prefabObject = getPrefab(world.objectMapping, id);
  return prefabObject != NULL;
}

// Prefab is original is the parent element that is a prefab, that first is in a different scene
std::optional<objid> prefabId(World& world, objid id){
  if (isPrefab(world, id)){
    return id;
  }
  return getGameObjectH(world.sandbox, id).prefabId;
}


float getViewspaceDepth(World& world, glm::mat4& transView, objid elementId){
  auto viewPosition = transView * fullModelTransform(world.sandbox, elementId);
  return getTransformationFromMatrix(viewPosition).position.z;
}

LayerInfo& layerByName(World& world, std::string layername){
  for (auto &layer : world.sandbox.layers){
    if (layer.name == layername){
      return layer;
    }
  }
  modassert(false, std::string("layer does not exist: " + layername));
  return world.sandbox.layers.at(0);
}

RenderStagesDofInfo getDofInfo(World& world, bool* _shouldRender, GameObjectCamera* activeCameraData, glm::mat4 view){
  bool depthEnabled = false;
  float minBlurDistance = 0.f;
  float maxBlurDistance = 0.f;
  float targetDepth = 0.f;
  float nearplane = 0.1f;
  float farplane = 100.f;
  unsigned int blurAmount = 1;

  if (activeCameraData != NULL){
    depthEnabled = activeCameraData -> enableDof;
    minBlurDistance = activeCameraData -> minBlurDistance;
    maxBlurDistance = activeCameraData -> maxBlurDistance;
    blurAmount = activeCameraData -> blurAmount;

    if (activeCameraData -> target != ""){
      auto elements = getByName(world.sandbox, activeCameraData -> target); // TODO PEROBJECT
      modassert(elements.size() == 1, std::string("elements size = ") + std::to_string(elements.size()));
      auto elementId = elements.at(0);
      auto halfBlurDistance = (maxBlurDistance - minBlurDistance) * 0.5f;
      targetDepth = -1 * getViewspaceDepth(world, view, elementId);
      minBlurDistance = targetDepth - halfBlurDistance;
      maxBlurDistance = targetDepth + halfBlurDistance;
      //std::cout << "dof info: (" << minBlurDistance << " " << maxBlurDistance << " " << targetDepth << ")" << std::endl;
      auto layerName = getGameObject(world, elementId).layer;
      auto targetObjLayer = layerByName(world, layerName);
      nearplane = targetObjLayer.nearplane;
      farplane = targetObjLayer.farplane;
    }
  }
  *_shouldRender = depthEnabled;
  RenderStagesDofInfo info {
    .blurAmount = blurAmount,
    .minBlurDistance = minBlurDistance,
    .maxBlurDistance = maxBlurDistance,
    .nearplane = nearplane,
    .farplane = farplane,
  };  
  return info;
}

////// octree stuff ///////////////////

void doOctreeRaycast(World& world, objid id, glm::vec3 fromPos, glm::vec3 toPos, bool alt){
  if (!idExists(world.sandbox, id) || (!isOctree(world, id))){
    return;
  }
  auto octreeModelMatrix = fullModelTransform(world.sandbox, id);
  auto adjustedPosition = glm::inverse(octreeModelMatrix) * glm::vec4(fromPos.x, fromPos.y, fromPos.z, 1.f);
  auto adjustedToPos = glm::inverse(octreeModelMatrix) * glm::vec4(toPos.x, toPos.y, toPos.z, 1.f);
  auto adjustedDir = adjustedToPos - adjustedPosition;

  GameObjectOctree* octreeObj = getOctree(world.objectMapping, id);
  if (octreeObj){
    modassert(octreeObj, "draw selection grid onFrame not octree type");
    handleOctreeRaycast(octreeObj -> octree, adjustedPosition, adjustedDir, alt, id);
    setSelectedOctreeId(id);    
  }
}
void setPrevOctreeTexture(){
  setOctreeTextureId(getOctreeTextureId() - 1);
}
void setNextOctreeTexture(){
  setOctreeTextureId(getOctreeTextureId() + 1);
}
void loadOctree(World& world, objid selectedIndex){
  GameObjectOctree* octreeObject = getOctree(world.objectMapping, selectedIndex);
  modassert(octreeObject, "octree object is null");
  loadOctree(
    *octreeObject, 
    [&world](std::string filepath) -> std::string {
      return world.interface.readFile(filepath);
    }, 
    createScopedLoadMesh(world, selectedIndex)
  );
  updatePhysicsBody(world, selectedIndex);
}
void saveOctree(World& world, objid selectedIndex){
  GameObjectOctree* octreeObject = getOctree(world.objectMapping, selectedIndex);
  modassert(octreeObject, "octree object null");
  saveOctree(*octreeObject, world.interface.saveFile);
}

void writeOctreeTexture(World& world, objid selectedIndex, bool unitTexture){
  GameObjectOctree* octreeObject = getOctree(world.objectMapping, selectedIndex);
  modassert(octreeObject, "octree object is null");
  writeOctreeTexture(*octreeObject, octreeObject -> octree, createScopedLoadMesh(world, selectedIndex), unitTexture, TEXTURE_UP);
}

GameObjectOctree* getMainOctree(World& world, objid* id){
  *id = 0;
  GameObjectOctree* mainOctree = NULL;
  for (auto &[octreeId, octree] : world.objectMapping.octree){
    *id = octreeId;
    modassert(mainOctree == NULL, "more than one octree");
    return &octree;
  }
  return NULL;
}
std::vector<TagInfo> getTag(World& world, int tag, glm::vec3 position){
  objid id = 0;
  GameObjectOctree* octreeObject = getMainOctree(world, &id);
  if (!octreeObject){
    return {};
  }

  auto octreeModelMatrix = fullModelTransform(world.sandbox, id);
  auto octreeSpaceCamPos = glm::inverse(octreeModelMatrix) * glm::vec4(position.x, position.y, position.z, 1.f);

  return getTag(octreeObject -> octree, tag, glm::vec3(octreeSpaceCamPos.x, octreeSpaceCamPos.y, octreeSpaceCamPos.z), 10); // subdivision 10 is stupid, this should just retrieve to relevant depth
}

std::vector<TagInfo> getAllTags(World& world, int tag){
  objid id = 0;
  GameObjectOctree* octreeObject = getMainOctree(world, &id);
  if (!octreeObject){
    return {};
  }
  
  std::set<std::string> tagValues;
  OctreeDivision& rootNode = octreeObject -> octree.rootNode;
  std::deque<OctreeDivision*> divisions;
  divisions.push_back(&rootNode);
  while(divisions.size() > 0){
    OctreeDivision* node = divisions.front();
    divisions.pop_front();
    for (auto &divisionTag : node -> tags){
      if (divisionTag.key == tag){
        tagValues.insert(divisionTag.value);
      }
    }

    for (OctreeDivision& division : node -> divisions){
      divisions.push_back(&division);
    }
  }

  std::vector<TagInfo> allTags;
  for (auto& tagValue : tagValues){
    allTags.push_back(TagInfo{
      .key = tag,
      .value =  tagValue,
    });
  }
  return allTags;
}


std::optional<OctreeMaterial> getMaterial(World& world, glm::vec3 position){
  objid id = 0;
  GameObjectOctree* octreeObject = getMainOctree(world, &id);
  if (!octreeObject){
    return {};
  }

  auto octreeModelMatrix = fullModelTransform(world.sandbox, id);
  auto octreeSpaceCamPos = glm::inverse(octreeModelMatrix) * glm::vec4(position.x, position.y, position.z, 1.f);
  return getMaterial(octreeObject -> octree, glm::vec3(octreeSpaceCamPos.x, octreeSpaceCamPos.y, octreeSpaceCamPos.z), 10); // subdivision 10 is stupid, this should just retrieve to relevant depth
}

void setMeshEnabled(World& world, int32_t id, bool enabled){
  auto meshObj = getMesh(world.objectMapping, id, getObjTypeLookup(world.sandbox, id));
  if (meshObj){
    meshObj -> isDisabled = !enabled;
  }
}