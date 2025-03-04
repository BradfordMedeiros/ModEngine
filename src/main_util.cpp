#include "./main_util.h"

extern World world;
extern engineState state;
extern glm::mat4 view;
extern Stats statistics;
extern LineData lineData;

objid createManipulator(){
  GameobjAttributes manipulatorAttr {
      .attr = {
        {"mesh", "./res/models/ui/manipulator.gltf" }, 
        {"layer", "scale" },
        { "scale", glm::vec3(10.f, 10.f, 10.f) },
      },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes = {
    {"manipulator/xaxis", { GameobjAttributes { .attr = {{ "tint", glm::vec4(1.f, 1.f, 0.f, 0.8f) }} }}},
    {"manipulator/yaxis", { GameobjAttributes { .attr = {{ "tint", glm::vec4(1.f, 0.f, 1.f, 0.8f) }} }}},
    {"manipulator/zaxis", { GameobjAttributes { .attr = {{ "tint", glm::vec4(0.f, 0.f, 1.f, 0.8f) }} }}},
  };
  return makeObjectAttr(0, "manipulator", manipulatorAttr, submodelAttributes).value();
}

ManipulatorSelection onManipulatorSelected(){
  std::vector<objid> ids;
  for (auto &id : selectedIds(state.editor)){
    if (getLayerForId(id).selectIndex != -2){
      ids.push_back(id);
    }
  }
  return ManipulatorSelection {
    .mainObj = ids.size() > 0 ? std::optional<objid>(ids.at(ids.size() - 1)) : std::optional<objid>(std::nullopt),
    .selectedIds = ids,
  }; 
}

glm::mat4 projectionFromLayer(LayerInfo& layer){
  // this means that as the window is dragged wider (say 2560x1980) you simply see more
  return glm::perspective(glm::radians(layer.fov), (float)state.viewportSize.x / (float)state.viewportSize.y, layer.nearplane, layer.farplane); 

  // this would show a constant amount in the screen, but then just stretch it, which might be more "fair" for a  multiplayer game, but looks super shitty
  // if i care about "fair" should just use a smaller viewport configuration and not render to whole scene
  // this is not how it should be done, but leaving this just since it is conceptually interesting :)
  //return glm::perspective(glm::radians(layer.fov), (float)state.resolution.x / (float)state.resolution.y, layer.nearplane, layer.farplane); 
}

LayerInfo getLayerForId(objid id){
  return layerByName(world, getGameObject(world, id).layer);
}
RotationDirection getCursorInfoWorld(float ndix, float ndiy){
  auto layer = layerByName(world, "");
  auto projection = projectionFromLayer(layer);
  float screenXPosNdi = ndix;
  float screenYPosNdi = ndiy;
  //float screenXPosNdi = convertBase(state.cursorLeft, 0.f, state.currentScreenWidth, -1.f, 1.f);
  //float screenYPosNdi = convertBase(state.currentScreenHeight - state.cursorTop, 0.f, state.currentScreenHeight, -1.f, 1.f);
  auto positionAndRotation = getCursorInfoWorldNdi(projection, view, screenXPosNdi, screenYPosNdi, -1.f);
  return positionAndRotation;
}


struct ExtractSuffix {
  std::string suffix;
  std::string rest;
};
ExtractSuffix extractSuffix(std::string& value, char delimeter){
  auto tagSplit = split(value, delimeter);
  auto tagRest = tagSplit.size() > 1 ? join(subvector(tagSplit, 0, tagSplit.size() - 1), delimeter) : value;
  auto tagPart = tagSplit.size() > 1 ? tagSplit.at(tagSplit.size() -1) : "";
  return ExtractSuffix {
    .suffix = tagPart,
    .rest = tagRest,
  };
}
std::vector<ParsedLoadScene> parseSceneArgs(std::vector<std::string>& rawScenes){
  std::vector<ParsedLoadScene> parsedScenes;
  for (auto rawScene : rawScenes){
    // :scenename
    // =tag1,tag2,tag3 
    auto tagExtract = extractSuffix(rawScene, '=');
    auto tags = split(tagExtract.suffix, '.');
    auto scenenameExtract = extractSuffix(tagExtract.rest, ':');
    std::optional<std::string> sceneFileName = scenenameExtract.suffix.size() > 0 ? std::optional<std::string>(scenenameExtract.suffix) : std::nullopt;
    auto sceneToLoad = scenenameExtract.rest;
  
    ParsedLoadScene parsedScene {
      .sceneToLoad = sceneToLoad,
      .sceneFileName = sceneFileName,
      .tags = tags,
    };
    parsedScenes.push_back(parsedScene);
  }
  return parsedScenes;
}

std::optional<unsigned int> getTextureId(std::string& texture){
  if (world.textures.find(texture) == world.textures.end()){
    return std::nullopt;
  }
  return world.textures.at(texture).texture.textureId;
}


glm::vec3 getTintIfSelected(bool isSelected){
  if (isSelected){
    return glm::vec3(1.0f, 0.0f, 0.0f);
  }
  return glm::vec3(0.f, 1.f, 1.f);
}

float exposureAmount(){
  float elapsed = statistics.now - state.exposureStart;
  float amount = elapsed / 1.f;   
  float exposureA = glm::clamp(amount, 0.f, 1.f);
  float effectiveExposure = glm::mix(state.oldExposure, state.targetExposure, exposureA);
  return effectiveExposure;
}

ManipulatorTools tools {
  .getPosition = [](objid id) -> glm::vec3 { return getGameObjectPosition(id, true); },
  .setPosition = setGameObjectPosition,
  .getScale = getGameObjectScale,
  .setScale = [](int32_t index, glm::vec3 scale) -> void { setGameObjectScale(index, scale, true); },
  .getRotation = [](objid id) -> glm::quat { return getGameObjectRotation(id, false); },
  .setRotation = [](objid id, glm::quat rot) -> void { setGameObjectRotation(id, rot, true); },
  .snapPosition = [](glm::vec3 pos) -> glm::vec3 {
    return snapTranslate(state.easyUse, pos);
  },
  .snapScale = [](glm::vec3 scale) -> glm::vec3 {
    return snapScale(state.easyUse, scale);
  },
  .snapRotate = [](glm::quat rot, Axis snapAxis, float extraRadians) -> glm::quat {
    return snapRotate(state.easyUse, rot, snapAxis, extraRadians);
  },
  .drawLine = [](glm::vec3 frompos, glm::vec3 topos, LineColor color) -> void {
    if (state.manipulatorLineId == 0){
      state.manipulatorLineId = getUniqueObjId();
    }
    addLineToNextCycle(lineData, frompos, topos, true, state.manipulatorLineId, color, std::nullopt);
  },
  .getSnapRotation = []() -> std::optional<glm::quat> { 
    return getSnapTranslateSize(state.easyUse).orientation; 
  },
  .clearLines = []() -> void {
    if (state.manipulatorLineId != 0){
      removeLinesByOwner(lineData, state.manipulatorLineId);
    }
  },
  .removeObjectById = removeObjectById,
  .makeManipulator = createManipulator,
  .getSelectedIds = onManipulatorSelected,
};



glm::vec3 positionToNdi(glm::vec3 position){
  auto viewTransform = getCameraTransform();
  auto view = renderView(viewTransform.position, viewTransform.rotation);
  auto projection = projectionFromLayer(world.sandbox.layers.at(0));
  auto transformedValue = projection * view * glm::vec4(position.x, position.y, position.z, 1.f);
  auto dividedValue = glm::vec4(transformedValue.x / transformedValue.w, transformedValue.y / transformedValue.w, transformedValue.z / transformedValue.w, transformedValue.z / glm::abs(transformedValue.w));
  //modlog("waypoint transformedValue1", print(transformedValue));
  //modlog("waypoint transformedValue2", print(finalValue));
  //modlog("waypoint transformedValue2", print(dividedValue));
  return glm::vec3(dividedValue.x, dividedValue.y, dividedValue.w);
}