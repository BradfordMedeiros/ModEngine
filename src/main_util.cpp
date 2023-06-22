#include "./main_util.h"

extern World world;
extern engineState state;
extern glm::mat4 view;

objid createManipulator(){
  GameobjAttributes manipulatorAttr {
      .stringAttributes = {
        {"mesh", "./res/models/ui/manipulator.gltf" }, 
        {"layer", "scale" },
      },
      .numAttributes = {},
      .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes = {
    {"manipulator/xaxis", { GameobjAttributes { .vecAttr = { .vec4 = {{ "tint", glm::vec4(1.f, 1.f, 0.f, 0.8f) }} }}}},
    {"manipulator/yaxis", { GameobjAttributes { .vecAttr = { .vec4 = {{ "tint", glm::vec4(1.f, 0.f, 1.f, 0.8f) }} }}}},
    {"manipulator/zaxis", { GameobjAttributes { .vecAttr = { .vec4 = {{ "tint", glm::vec4(0.f, 0.f, 1.f, 0.8f) }} }}}},
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

LayerInfo layerByName(std::string layername){
  for (auto &layer : world.sandbox.layers){
    if (layer.name == layername){
      return layer;
    }
  }
  modassert(false, std::string("layer does not exist: " + layername));
  return LayerInfo{};
}
LayerInfo getLayerForId(objid id){
  return layerByName(getGameObject(world, id).layer);
}
RotationDirection getCursorInfoWorld(float ndix, float ndiy){
  auto layer = world.sandbox.layers.at(0);
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
  return world.textures.at(texture).texture.textureId;
}


