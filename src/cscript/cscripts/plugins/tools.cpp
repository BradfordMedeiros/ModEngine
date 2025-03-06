#include "./tools.h"

extern CustomApiBindings* mainApi;


// shouldn't really depend on this stuff
#include "../../../state.h"
extern World world;
extern glm::mat4 view;
extern ManipulatorTools tools;
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


CScriptBinding cscriptCreateToolsBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/tools", api);
  binding.onFrame = [](int32_t id, void* data) -> void {
    //drawNormals();

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
