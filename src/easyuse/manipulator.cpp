#include "./manipulator.h"

ManipulatorData createManipulatorData(){
  return ManipulatorData {
    .state = "idle",

    .manipulatorId = 0,
    .manipulatorObject = NOAXIS,
    .initialDragPosition = std::nullopt,
    .initialDragPositions = {},
    .initialDragScales = {},
    .initialDragRotations = {},
  };
}


objid getManipulatorId(ManipulatorData& manipulatorState){
  return manipulatorState.manipulatorId;
}

void onManipulatorSelectItem(ManipulatorData& manipulatorState, objid selectedItem, std::string selectedItemName){
}
void onManipulatorMouseRelease(ManipulatorData& manipulatorState){

}

struct ManipulatorStateTransition {
  std::string transition;
  std::function<void()> fn;
};

struct ManipulatorNextState {
  std::string nextState;
  std::string transition;
};
struct ManipulatorState {
  std::string state;
  std::function<void(ManipulatorData&, ManipulatorTools&)> onEnterState;
  std::function<void(ManipulatorData&, ManipulatorTools&)> onExitState;
  std::vector<ManipulatorNextState> nextStates;
};


void manipulatorEnsureExists(ManipulatorData& manipulatorState, ManipulatorTools& tools){
  auto manipulatorExists = manipulatorState.manipulatorId != 0;
  if (!manipulatorExists){
    manipulatorState.manipulatorId = tools.makeManipulator();
  }
  auto manipulatorId = manipulatorState.manipulatorId;
  auto selectedObjs = tools.getSelectedIds();
  modassert(selectedObjs.mainObj.has_value(), "manipulator selected obj main value does not have a value");
  tools.setPosition(manipulatorId, tools.getPosition(selectedObjs.mainObj.value()));
}


void manipulatorEnsureDoesNotExist(ManipulatorData& manipulatorState, ManipulatorTools& tools){
  if (manipulatorState.manipulatorId != 0){
    tools.removeObjectById(manipulatorState.manipulatorId);
  }
  manipulatorState.manipulatorId = 0;
}

std::vector<ManipulatorState> manipulatorStates = {
  ManipulatorState {
    .state = "idle",
    .onEnterState = [](ManipulatorData& manipulatorState, ManipulatorTools& tools) -> void { },
    .onExitState = [](ManipulatorData& manipulatorState, ManipulatorTools& tools) -> void { },
    .nextStates = { 
      ManipulatorNextState { .nextState = "rotateMode", .transition = "gotoRotate" },
      ManipulatorNextState { .nextState = "translateMode", .transition = "gotoTranslate" },
      ManipulatorNextState { .nextState = "scaleMode", .transition = "gotoScale" },
    },
  },
  ManipulatorState {
    .state = "rotateMode",
    .onEnterState = [](ManipulatorData& manipulatorState, ManipulatorTools& tools) -> void { },
    .onExitState = manipulatorEnsureDoesNotExist,
    .nextStates = {
      ManipulatorNextState { .nextState = "translateMode", .transition = "gotoTranslate" },
      ManipulatorNextState { .nextState = "scaleMode", .transition = "gotoScale" },
      ManipulatorNextState { .nextState = "idle", .transition = "unselected" },
    },
  },
  ManipulatorState {
    .state = "translateMode",
    .onEnterState = manipulatorEnsureExists,
    .onExitState = manipulatorEnsureDoesNotExist,
    .nextStates = {
      ManipulatorNextState { .nextState = "rotateMode", .transition = "gotoRotate" },
      ManipulatorNextState { .nextState = "scaleMode", .transition = "gotoScale" },
      ManipulatorNextState { .nextState = "idle", .transition = "unselected" },   
    },
  },
  ManipulatorState {
    .state = "scaleMode",
    .onEnterState = manipulatorEnsureExists,
    .onExitState = manipulatorEnsureDoesNotExist,
    .nextStates = {
      ManipulatorNextState { .nextState = "rotateMode", .transition = "gotoRotate" },
      ManipulatorNextState { .nextState = "translateMode", .transition = "gotoTranslate" },
      ManipulatorNextState { .nextState = "idle", .transition = "unselected" },
    },
  },
};


ManipulatorState& manipulatorStateByName(std::string name){
  for (auto &manipulatorState : manipulatorStates){
    if (manipulatorState.state == name){
      return manipulatorState;
    }
  }
  modassert(false, "invalid manipulator state name");
  return manipulatorStates.at(0);
}
ManipulatorState* manipulatorTransitionByName(ManipulatorState& manipulatorState, std::string transitionName){
  for (auto &state : manipulatorState.nextStates){
    if (state.transition == transitionName){
      return &manipulatorStateByName(state.nextState);
    }
  }
  return NULL;
}
void manipulatorHandleTransition(ManipulatorData& manipulatorState, ManipulatorTools& manipulatorTools, std::string transitionName){
  ManipulatorState& currentState = manipulatorStateByName(manipulatorState.state);
  auto nextState = manipulatorTransitionByName(currentState, transitionName);
  if (nextState != NULL){
    std::cout << "calling on exit: " << currentState.state << std::endl;
    currentState.onExitState(manipulatorState, manipulatorTools);
    std::cout << "calling on enter: " << nextState -> state << std::endl;
    nextState -> onEnterState(manipulatorState, manipulatorTools);
    manipulatorState.state = nextState -> state;
  }
}


void onManipulatorUpdate(
  ManipulatorData& manipulatorState,
  glm::mat4 projection,
  glm::mat4 cameraViewMatrix, 
  ManipulatorMode mode,
  Axis defaultAxis,
  float mouseX, 
  float mouseY,
  glm::vec2 cursorPos,
  glm::vec2 screensize,
  ManipulatorOptions options,
  ManipulatorTools manipulatorTools
){
  modlog("manipulator", std::string("manipulator state: ") + manipulatorState.state);

  auto selectedObjs = manipulatorTools.getSelectedIds();
  bool objectSelected = selectedObjs.mainObj.has_value();

  if (!objectSelected){
    manipulatorHandleTransition(manipulatorState, manipulatorTools, "unselected");
    return;
  }

  if (mode == ROTATE && objectSelected){
    manipulatorHandleTransition(manipulatorState, manipulatorTools, "gotoRotate");
  }else if (mode == SCALE && objectSelected){
    manipulatorHandleTransition(manipulatorState, manipulatorTools, "gotoScale");
  }else if (mode == TRANSLATE && objectSelected){
    manipulatorHandleTransition(manipulatorState, manipulatorTools, "gotoTranslate");
  }



  // for current state, do the on update thing
}
