#include "./tools.h"

extern CustomApiBindings* mainApi;


// shouldn't really depend on this stuff
#include "../../../state.h"
extern World world;
extern glm::mat4 view;
extern engineState state;

glm::mat4 projectionFromLayer(LayerInfo& layer);
LayerInfo& layerByName(World& world, std::string layername);

void drawNormals(){
  auto selectedIds = mainApi -> selected();
  for (auto idInScene : selectedIds){
    auto groupId = mainApi -> groupId(idInScene);
    auto name = mainApi -> getGameObjNameForId(idInScene).value();
    auto position = mainApi -> getGameObjectPos(idInScene, true);
    auto rotation = mainApi -> getGameObjectRotation(idInScene, true);
    auto toPosition = position + (rotation * glm::vec3(0.f, 0.f, -1.f));
    auto leftArrow = position + (rotation * glm::vec3(-0.2f, 0.f, -0.8f));
    auto rightArrow = position + (rotation * glm::vec3(0.2f, 0.f, -0.8f));

    mainApi -> drawLine(position, toPosition, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    mainApi -> drawLine(leftArrow, toPosition, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    mainApi -> drawLine(rightArrow, toPosition, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    modlog("tools", print(position) + " " + print(toPosition));
    modlog("tools name", name);
  }
}

struct LineData;
extern LineData lineData;

std::optional<objid> makeObjectAttr(objid sceneId, std::string name, GameobjAttributes& attributes, std::map<std::string, GameobjAttributes>& submodelAttributes);
LayerInfo getLayerForId(objid id);
glm::vec3 getGameObjectPosition(int32_t index, bool isWorld);
void setGameObjectPosition(int32_t index, glm::vec3 pos, bool isWorld);
glm::quat getGameObjectRotation(int32_t index, bool isWorld);
void setGameObjectScale(int32_t index, glm::vec3 scale, bool isWorld);
glm::vec3 getGameObjectScale(int32_t index);
void setGameObjectRotation(int32_t index, glm::quat rotation, bool isWorld);
void removeObjectById(objid id);
void removeLinesByOwner(LineData& lineData, objid owner);
objid addLineToNextCycle(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, LineColor color, std::optional<unsigned int> textureId);
bool idInGroup(World& world, objid id, std::vector<objid> groupIds);

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
  for (auto &id : mainApi -> selected()){
    if (getLayerForId(id).selectIndex != -2){
      ids.push_back(id);
    }
  }
  return ManipulatorSelection {
    .mainObj = ids.size() > 0 ? std::optional<objid>(ids.at(ids.size() - 1)) : std::optional<objid>(std::nullopt),
    .selectedIds = ids,
  }; 
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


void drawAABB(objid id){
  auto aabb = mainApi -> getModAABBModel(id);
  if (!aabb.has_value()){
    std::cout << "could not get the aabb for this model" << std::endl;
    return;
  }
  auto bounds = toBounds(aabb.value());
  mainApi -> drawLine(bounds.topLeftFront, bounds.topRightFront, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  mainApi -> drawLine(bounds.bottomLeftFront, bounds.bottomRightFront, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  mainApi -> drawLine(bounds.topLeftFront, bounds.topLeftBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  mainApi -> drawLine(bounds.topRightFront, bounds.topRightBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  mainApi -> drawLine(bounds.topLeftBack, bounds.topRightBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  mainApi -> drawLine(bounds.bottomLeftBack, bounds.bottomRightBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  mainApi -> drawLine(bounds.bottomLeftFront, bounds.bottomLeftBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  mainApi -> drawLine(bounds.bottomRightFront, bounds.bottomRightBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  mainApi -> drawLine(bounds.topLeftFront, bounds.bottomLeftFront, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  mainApi -> drawLine(bounds.topRightFront, bounds.bottomRightFront, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  mainApi -> drawLine(bounds.topLeftBack, bounds.bottomLeftBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  mainApi -> drawLine(bounds.topRightBack, bounds.bottomRightBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
}

void drawBounding(objid id){
  auto physicsInfo = mainApi -> getPhysicsInfo(id);
  auto position = mainApi -> getGameObjectPos(id, true);
  if (physicsInfo.has_value()){
    if (physicsInfo.value().offset.has_value()){
      position += physicsInfo.value().offset.value();
    }
          
    auto boundInfo = physicsInfo.value().boundInfo;
    float width = boundInfo.xMax - boundInfo.xMin;
    float height = boundInfo.yMax - boundInfo.yMin;
    float depth = boundInfo.zMax - boundInfo.zMin;

    glm::vec3 bottomLeft(-0.5f * width, -0.5f * height, -0.5f * depth);
    glm::vec3 bottomRight(0.5f * width, -0.5f * height, -0.5f * depth);
    glm::vec3 topLeft(-0.5f * width, 0.5f * height, -0.5f * depth);
    glm::vec3 topRight(0.5f * width, 0.5f * height, -0.5f * depth);

    glm::vec3 bottomLeftBack(-0.5f * width, -0.5f * height, 0.5f * depth);
    glm::vec3 bottomRightBack(0.5f * width, -0.5f * height, 0.5f * depth);
    glm::vec3 topLeftBack(-0.5f * width, 0.5f * height, 0.5f * depth);
    glm::vec3 topRightBack(0.5f * width, 0.5f * height, 0.5f * depth);        

    auto rotation = mainApi -> getGameObjectRotation(id, true);
    auto scale = mainApi -> getGameObjectScale(id, true);

    bottomLeft *= scale;
    bottomRight *= scale;
    topLeft *= scale;
    topRight *= scale;

    bottomLeftBack *= scale;
    bottomRightBack *= scale;
    topLeftBack *= scale;
    topRightBack *= scale;

    bottomLeft = rotation * bottomLeft;
    bottomRight = rotation * bottomRight;
    topLeft = rotation * topLeft;
    topRight = rotation * topRight;

    bottomLeftBack = rotation * bottomLeftBack;
    bottomRightBack = rotation * bottomRightBack;
    topLeftBack = rotation * topLeftBack;
    topRightBack = rotation * topRightBack;

    bottomLeft += position;
    bottomRight += position;
    topLeft += position;
    topRight += position;

    bottomLeftBack += position;
    bottomRightBack += position;
    topLeftBack += position;
    topRightBack += position;

    mainApi -> drawLine(bottomLeft, bottomRight, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    mainApi -> drawLine(topLeft,    topRight, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    mainApi -> drawLine(topLeft,    bottomLeft, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    mainApi -> drawLine(topRight,   bottomRight, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);

    mainApi -> drawLine(bottomLeftBack, bottomRightBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    mainApi -> drawLine(topLeftBack,    topRightBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    mainApi -> drawLine(topLeftBack,    bottomLeftBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    mainApi -> drawLine(topRightBack,   bottomRightBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);

    mainApi -> drawLine(bottomLeft, bottomLeftBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    mainApi -> drawLine(topLeft,    topLeftBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    mainApi -> drawLine(topRight,    topRightBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    mainApi -> drawLine(bottomRight,   bottomRightBack, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  }
}


CScriptBinding cscriptCreateToolsBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/tools", api);
  binding.onFrame = [](int32_t id, void* data) -> void {
    //drawNormals();

    std::set<objid> alreadyDrawn;
    auto selectedIds = mainApi -> selected();
    for (auto oldid : selectedIds){
      auto id = mainApi -> groupId(oldid);
      if (alreadyDrawn.count(id) > 0){
        continue;
      }
      alreadyDrawn.insert(id);

      drawAABB(oldid);
      //drawBounding(id);
    }

    // utilities 
    static auto manipulatorLayer = layerByName(world, "");
    onManipulatorUpdate(
      state.manipulatorState, 
      projectionFromLayer(manipulatorLayer),
      view, 
      state.manipulatorMode, 
      state.manipulatorAxis,
      state.offsetX, 
      state.offsetY,
      glm::vec2(state.adjustedCoords.x, state.adjustedCoords.y),
      glm::vec2(state.resolution.x, state.resolution.y),
      ManipulatorOptions {
         .manipulatorPositionMode = state.manipulatorPositionMode,
         .relativePositionMode = state.relativePositionMode,
         .translateMirror = state.translateMirror,
         .rotateMode = state.rotateMode,
         .scalingGroup = state.scalingGroup,
         .snapManipulatorScales = state.snapManipulatorScales,
         .preserveRelativeScale = state.preserveRelativeScale,
      },
      tools,
      !(state.inputMode == ENABLED)
    );
  };

  binding.onObjectSelected = [](objid scriptId, void* data, int32_t id, glm::vec3 color, int selectIndex) -> void {
    modlog("tools", std::to_string(selectIndex));
    if (selectIndex == -4){ // just make this unique to the manipulator
      auto idToUse = state.groupSelection ? mainApi -> groupId(id) : id;
      auto selectedSubObj = mainApi -> getGameObjNameForId(id).value();
      modlog("tools", selectedSubObj);
      onManipulatorSelectItem(state.manipulatorState, idToUse, selectedSubObj);
    }
  };

  return binding;
}
