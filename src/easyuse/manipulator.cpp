#include "./manipulator.h"

ManipulatorData createManipulatorData(){
  return ManipulatorData {
    .state = "idle",
    .mouseClickedLastFrame = false,
    .mouseReleasedLastFrame = false,
    .selectedItemLastFrame = false,

    .manipulatorId = 0,
    .manipulatorObject = NOAXIS,

    .initialDragPosition = std::nullopt,
    .initialTransforms = {},
    .rotationAmount = 0.f,

  };
}


objid getManipulatorId(ManipulatorData& manipulatorState){
  return manipulatorState.manipulatorId;
}

void onManipulatorSelectItem(ManipulatorData& manipulatorState, objid selectedItem, std::string selectedItemName){
  auto isTargetManipulator = selectedItem == manipulatorState.manipulatorId;
  if (isTargetManipulator){
    modlog("manipulator", std::string("item name is: ") + selectedItemName);
    if (selectedItemName == "manipulator/xaxis"){
      modlog("manipulator", "setting manipulator to xaxis");
      manipulatorState.manipulatorObject = XAXIS;
    }else if (selectedItemName == "manipulator/yaxis"){
      modlog("manipulator", "setting manipulator to yaxis");
      manipulatorState.manipulatorObject = YAXIS;
    }else if (selectedItemName == "manipulator/zaxis"){
      modlog("manipulator", "setting manipulator to zaxis");
      manipulatorState.manipulatorObject = ZAXIS;
    }
  }
  manipulatorState.selectedItemLastFrame = true;
}

void onManipulatorMouseDown(ManipulatorData& manipulatorState){
  manipulatorState.mouseClickedLastFrame = true;
}
void onManipulatorMouseRelease(ManipulatorData& manipulatorState){
  manipulatorState.mouseReleasedLastFrame = true;
}

void onManipulatorEvent(ManipulatorData& manipulatorState, ManipulatorTools& tools, MANIPULATOR_EVENT event){
  auto selectedObjs = tools.getSelectedIds();
  if (event == OBJECT_ORIENT_UP){
    modlog("manipulator", "event - object_orient_up");
    for (auto id: selectedObjs.selectedIds){
      tools.setRotation(id, quatFromDirection(glm::vec3(0.f, 1.f, 0.f)));
    }
  }else if (event == OBJECT_ORIENT_DOWN){
    modlog("manipulator", "event - object_orient_down");
    for (auto id: selectedObjs.selectedIds){
      tools.setRotation(id, quatFromDirection(glm::vec3(0.f, -1.f, 0.f)));
    }
  }else if (event == OBJECT_ORIENT_RIGHT){
    modlog("manipulator", "event - object_orient_right");
    for (auto id: selectedObjs.selectedIds){
      tools.setRotation(id, quatFromDirection(glm::vec3(1.f, 0.f, 0.f)));
    }
  }else if (event == OBJECT_ORIENT_LEFT){
    modlog("manipulator", "event - object_orient_left");
    for (auto id: selectedObjs.selectedIds){
      tools.setRotation(id, quatFromDirection(glm::vec3(-1.f, 0.f, 0.f)));
    }
  }else if (event == OBJECT_ORIENT_FORWARD){
    modlog("manipulator", "event - object_orient_forward");
    for (auto id: selectedObjs.selectedIds){
      tools.setRotation(id, quatFromDirection(glm::vec3(0.f, 0.f, -1.f)));
    }
  }else if (event == OBJECT_ORIENT_BACK){
    modlog("manipulator", "event - object_orient_back");
    for (auto id: selectedObjs.selectedIds){
      tools.setRotation(id, quatFromDirection(glm::vec3(0.f, 0.f, 1.f)));
    }
  }
}

struct ManipulatorStateTransition {
  std::string transition;
  std::function<void()> fn;
};

struct ManipulatorNextState {
  std::string nextState;
  std::string transition;
  std::function<void(ManipulatorData&, ManipulatorTools&, ManipulatorUpdateInfo&)> fn;
};
struct ManipulatorState {
  std::string state;
  std::function<void(ManipulatorData& manipulatorState, ManipulatorTools& tools, ManipulatorUpdateInfo& updateInfo)> onState;
  std::vector<ManipulatorNextState> nextStates;
};


void manipulatorEnsureExists(ManipulatorData& manipulatorState, ManipulatorTools& tools, ManipulatorUpdateInfo& update){
  auto manipulatorExists = manipulatorState.manipulatorId != 0;
  if (!manipulatorExists){
    manipulatorState.manipulatorId = tools.makeManipulator();
  }
  auto manipulatorId = manipulatorState.manipulatorId;
  auto selectedObjs = tools.getSelectedIds();
  modassert(selectedObjs.mainObj.has_value(), "manipulator selected obj main value does not have a value");
  tools.setPosition(manipulatorId, tools.getPosition(selectedObjs.mainObj.value()));
}


void manipulatorEnsureDoesNotExist(ManipulatorData& manipulatorState, ManipulatorTools& tools, ManipulatorUpdateInfo& update){
  if (manipulatorState.manipulatorId != 0){
    tools.removeObjectById(manipulatorState.manipulatorId);
  }
  manipulatorState.manipulatorId = 0;
}

void manipulatorPopulateInitialPositions(ManipulatorData& manipulatorState, ManipulatorTools& tools, ManipulatorUpdateInfo& update){
  manipulatorState.initialDragPosition = std::nullopt;
  manipulatorState.initialTransforms = {};
  manipulatorState.meanPosition = std::nullopt;
  std::vector<objid> ids = update.selectedObjs.selectedIds;
  ids.push_back(manipulatorState.manipulatorId);
  for (auto &targetId : ids){
    manipulatorState.initialTransforms.push_back(InitialTransformData {
      .id = targetId,
      .transform = Transformation {
        .position = tools.getPosition(targetId),
        .scale = tools.getScale(targetId),
        .rotation = tools.getRotation(targetId),
      },
    });
  }
}
Transformation getInitialTransformation(ManipulatorData& manipulatorState, objid targetId){
  for (auto &idTransform : manipulatorState.initialTransforms){
    if (idTransform.id == targetId){
      return idTransform.transform;
    }
  }
  modassert(false, std::string("no id for: ") + std::to_string(targetId));
  return Transformation {};
}

glm::vec3 projectAndVisualize(ManipulatorData& manipulatorState, ManipulatorTools& tools, ManipulatorUpdateInfo& update){
  auto selectedObjs = tools.getSelectedIds();
  modassert(selectedObjs.mainObj.has_value(), "should not be in translate state with the obj selected");
  ProjectCursorDebugInfo projectCursorInfo{};
  auto projectedPosition =  projectCursorPositionOntoAxis(update.projection, update.cameraViewMatrix, update.cursorPos, update.screensize, manipulatorState.manipulatorObject, getInitialTransformation(manipulatorState, selectedObjs.mainObj.value()).position, &projectCursorInfo);
  drawProjectionVisualization(tools.drawLine, projectCursorInfo);
  return projectedPosition;
}

void manipulatorDoNothing(ManipulatorData& manipulatorState, ManipulatorTools& tools, ManipulatorUpdateInfo& update){ }

std::string print(ManipulatorSelection& selection){
  std::string value = "";
  value += std::string("mainobj hasvalue? = ") + (selection.mainObj.has_value() ? "true" : "false") + ", value = " + (selection.mainObj.has_value() ? std::to_string(selection.mainObj.value()) : "no data") + "\n";
  value += "ids = [ ";
  for (auto &id : selection.selectedIds){
    value += std::to_string(id) + " ";
  }
  value += " ]";
  return value;
}

struct RotateInfo {
   float rotationAmount;
   glm::vec3 cameraPosition;
   glm::vec3 selectDir;
   glm::vec3 meanPosition;
   glm::vec3 intersection;
   glm::quat rotationOrientation;
};

RotateInfo calcRotateInfo(ManipulatorData& manipulatorState, ManipulatorTools& tools, ManipulatorUpdateInfo& update){
  auto rotationOrientation = axisToOrientation(update.defaultAxis);
  auto meanPosition = manipulatorState.meanPosition.has_value() ? manipulatorState.meanPosition.value() : calcMeanPosition(idToPositions(update.selectedObjs.selectedIds, tools.getPosition));
  manipulatorState.meanPosition = meanPosition;
  float rotationAmount = 0.f;
  auto selectDir = getCursorRayDirection(update.projection, update.cameraViewMatrix, update.cursorPos.x, update.cursorPos.y, update.screensize.x, update.screensize.y);

  auto cameraPosition = getTransformationFromMatrix(glm::inverse(update.cameraViewMatrix)).position; 
  auto vecDirection = directionFromQuat(rotationOrientation);
  auto optIntersection = findPlaneIntersection(meanPosition, vecDirection, cameraPosition, selectDir);
  modassert(optIntersection.has_value(), "visualize rotation - could not find intersection");
  auto intersection = optIntersection.value();
  auto pointRelativeToPlane = glm::inverse(rotationOrientation) * (intersection - meanPosition);
  rotationAmount = atanRadians360(pointRelativeToPlane.x, pointRelativeToPlane.y);
  //  std::cout << "position: " << print(cameraPosition) << std::endl;
  // std::cout << "meanposition: " << print(meanPosition) << std::endl;
  // std::cout << "selectdir: " << print(selectDir) << std::endl;
  // std::cout << "intersection: " << print(intersection) << std::endl;
  // std::cout << "xyz to plane: " << print(pointRelativeToPlane) << std::endl;
  // std::cout << "angle: " << glm::degrees(rotationAmount) << std::endl;
  return RotateInfo {
    .rotationAmount = rotationAmount,
    .cameraPosition = cameraPosition,
    .selectDir = selectDir,
    .meanPosition = meanPosition,
    .intersection = intersection,
    .rotationOrientation = rotationOrientation,
  };
}

void visualizeSubrotations(ManipulatorTools& tools, ManipulatorUpdateInfo& update, RotateInfo& rotateInfo){
  std::vector<IdVec3Pair> positions;
  for (auto &id : update.selectedObjs.selectedIds){
    positions.push_back(IdVec3Pair{
      .id = id,
      .value = tools.getPosition(id),
    });
  }
  drawRotation(positions, rotateInfo.meanPosition, rotateInfo.rotationOrientation, rotateInfo.cameraPosition, rotateInfo.selectDir, rotateInfo.intersection, rotateInfo.rotationAmount, tools.drawLine);
}

bool isPositivePartition(glm::vec3 meanPosition, glm::vec3 position, Axis axis){
  if (axis == XAXIS){
    return position.x > meanPosition.x;
  }else if (axis == YAXIS){
    return position.y > meanPosition.y;
  }else if (axis == ZAXIS){
    return position.z > meanPosition.z;
  }
  modassert(axis != NOAXIS, "invalid axis, must not be noaxis");
  return true;
}

void visualizePartitionPlane(ManipulatorData& manipulatorState, ManipulatorTools& tools, glm::vec3 meanPosition){
  float size = 100.f;
  auto orientation = axisToOrientation(manipulatorState.manipulatorObject);
  auto cornerBottomLeft = size * (orientation * glm::vec3 (-1.f, -1.f, 0.f)) + meanPosition;
  auto cornerTopLeft = size * (orientation * glm::vec3 (-1.f, 1.f, 0.f)) + meanPosition;
  auto cornerTopRight = size * (orientation * glm::vec3 (1.f, 1.f, 0.f)) + meanPosition;
  auto coronerBottomRight = size * (orientation * glm::vec3 (1.f, -1.f, 0.f)) + meanPosition;

  tools.drawLine(cornerBottomLeft, cornerTopLeft, BLUE);
  tools.drawLine(cornerTopLeft, cornerTopRight, BLUE);
  tools.drawLine(cornerTopRight, coronerBottomRight, BLUE);
  tools.drawLine(coronerBottomRight, cornerBottomLeft, BLUE);

  tools.drawLine(cornerTopLeft, coronerBottomRight, BLUE);
  tools.drawLine(cornerTopRight, cornerBottomLeft, BLUE);
}

void updateManipulatorToCurrentObjTranslate(ManipulatorData& manipulatorState, ManipulatorTools& tools, ManipulatorUpdateInfo& update){
  auto manipulatorId = manipulatorState.manipulatorId;
  auto selectedObjs = tools.getSelectedIds();
  modassert(selectedObjs.mainObj.has_value(), "manipulator selected obj main value does not have a value");
  tools.setPosition(manipulatorId, tools.getPosition(selectedObjs.mainObj.value()));
  if (!update.options.relativePositionMode){
    tools.setRotation(manipulatorId, MOD_ORIENTATION_FORWARD);
  }else{
    tools.setRotation(manipulatorState.manipulatorId, tools.getRotation(selectedObjs.mainObj.value()));
  }
}
void updateManipulatorToCurrentObjScale(ManipulatorData& manipulatorState, ManipulatorTools& tools, ManipulatorUpdateInfo& update){
  auto manipulatorId = manipulatorState.manipulatorId;
  auto selectedObjs = tools.getSelectedIds();
  modassert(selectedObjs.mainObj.has_value(), "manipulator selected obj main value does not have a value");
  tools.setPosition(manipulatorId, tools.getPosition(selectedObjs.mainObj.value()));
  tools.setRotation(manipulatorState.manipulatorId, tools.getRotation(selectedObjs.mainObj.value()));
}

std::vector<ManipulatorState> manipulatorStates = {
  ManipulatorState {
    .state = "idle",
    .onState = [](ManipulatorData& manipulatorState, ManipulatorTools& tools, ManipulatorUpdateInfo& updateInfo) -> void {},
    .nextStates = { 
      ManipulatorNextState { .nextState = "translateIdle", .transition = "gotoTranslateIdle", .fn = manipulatorEnsureExists },
      ManipulatorNextState { .nextState = "scaleIdle", .transition = "gotoScaleIdle", .fn = manipulatorEnsureExists },
      ManipulatorNextState { .nextState = "rotateIdle", .transition = "gotoRotateIdle", .fn = manipulatorPopulateInitialPositions },
    },
  },
  ManipulatorState {
    .state = "translateIdle",
    .onState = updateManipulatorToCurrentObjTranslate,
    .nextStates = {
      ManipulatorNextState { .nextState = "idle", .transition = "unselected", .fn = manipulatorEnsureDoesNotExist },
      ManipulatorNextState { .nextState = "translateMode", .transition = "axisSelected", .fn = manipulatorPopulateInitialPositions },
      ManipulatorNextState { .nextState = "scaleIdle", .transition = "gotoScaleIdle", .fn = manipulatorDoNothing },
      ManipulatorNextState { .nextState = "rotateIdle", .transition = "gotoRotateIdle", .fn = manipulatorEnsureDoesNotExist },
    },
  },
  ManipulatorState {
    .state = "translateMode",
    .onState = [](ManipulatorData& manipulatorState, ManipulatorTools& tools, ManipulatorUpdateInfo& update) -> void {
      modassert(update.selectedObjs.mainObj.has_value(), "cannot have no obj selected in this mode");
      
      auto projectedPositionRaw = projectAndVisualize(manipulatorState, tools, update);
      if (!manipulatorState.initialDragPosition.has_value()){
         manipulatorState.initialDragPosition = projectedPositionRaw;
      }
      auto meanPosition = manipulatorState.meanPosition.has_value() ? manipulatorState.meanPosition.value() : calcMeanPosition(idToPositions(update.selectedObjs.selectedIds, tools.getPosition));
      manipulatorState.meanPosition = meanPosition;

      auto positionDiff = projectedPositionRaw - manipulatorState.initialDragPosition.value();
      positionDiff = (update.options.manipulatorPositionMode == SNAP_RELATIVE) ? tools.snapPosition(positionDiff) : positionDiff;

      auto oldManipulatorPos = getInitialTransformation(manipulatorState, manipulatorState.manipulatorId).position;
      auto manipulatorPos =  (update.options.manipulatorPositionMode == SNAP_ABSOLUTE) ? tools.snapPosition(oldManipulatorPos + positionDiff) : (oldManipulatorPos + positionDiff);
      auto actualPositionDiff = manipulatorPos - oldManipulatorPos;
      tools.setPosition(manipulatorState.manipulatorId, manipulatorPos);

      auto translateMirror = update.options.translateMirror && update.selectedObjs.selectedIds.size() > 1;
      if (translateMirror){
        visualizePartitionPlane(manipulatorState, tools, meanPosition);  
      }

      auto mainObjOnPositivePartition = isPositivePartition(meanPosition, getInitialTransformation(manipulatorState, update.selectedObjs.mainObj.value()).position, manipulatorState.manipulatorObject);
      for (auto &targetId : update.selectedObjs.selectedIds){
        auto oldPosition = getInitialTransformation(manipulatorState, targetId).position;
        auto deltaPosition = actualPositionDiff;
        if (translateMirror){
          auto currObjOnPositivePartition = isPositivePartition(meanPosition, oldPosition, manipulatorState.manipulatorObject);
          if (currObjOnPositivePartition != mainObjOnPositivePartition){
            deltaPosition *= -1.f;
          }
        }
        auto newPosition = oldPosition + deltaPosition;
        tools.setPosition(targetId, newPosition);
      }
    },
    .nextStates = {
       ManipulatorNextState { .nextState = "translateIdle", .transition = "axisReleased", .fn = manipulatorDoNothing },
    },
  },
  ManipulatorState {
    .state = "scaleIdle",
    .onState = updateManipulatorToCurrentObjScale,
    .nextStates = {
      ManipulatorNextState { .nextState = "idle", .transition = "unselected", .fn = manipulatorEnsureDoesNotExist },
      ManipulatorNextState { .nextState = "scaleMode", .transition = "axisSelected", .fn = manipulatorPopulateInitialPositions },
      ManipulatorNextState { .nextState = "translateIdle", .transition = "gotoTranslateIdle", .fn = manipulatorDoNothing },
      ManipulatorNextState { .nextState = "rotateIdle", .transition = "gotoRotateIdle", .fn = manipulatorEnsureDoesNotExist },
    },
  },
  ManipulatorState {
    .state = "scaleMode",
    .onState = [](ManipulatorData& manipulatorState, ManipulatorTools& tools, ManipulatorUpdateInfo& update) -> void {
      modassert(update.selectedObjs.mainObj.has_value(), "cannot have no obj selected in this mode");
      modassert(manipulatorState.manipulatorId != 0, "manipulator must exist in this mode");

      auto projectedPosition = projectAndVisualize(manipulatorState, tools, update);
      if (!manipulatorState.initialDragPosition.has_value()){
         manipulatorState.initialDragPosition = projectedPosition;
      }

      auto meanPosition = manipulatorState.meanPosition.has_value() ? manipulatorState.meanPosition.value() : calcMeanPosition(idToPositions(update.selectedObjs.selectedIds, tools.getPosition));
      manipulatorState.meanPosition = meanPosition;
      auto positionDiff = calcPositionDiff(manipulatorState.initialDragPosition.value(), projectedPosition, getInitialTransformation(manipulatorState, manipulatorState.manipulatorId).position, true);
      auto scaleFactor = update.options.snapManipulatorScales ? tools.snapScale(positionDiff) : positionDiff;
      if (update.options.preserveRelativeScale){  // makes the increase in scale magnitude proportion to length of the vec
        auto oldScaleFactor = scaleFactor;
        auto vecLength = glm::length(scaleFactor);
        auto compLength = glm::sqrt(vecLength * vecLength / 3);
       
        auto positiveValue = glm::dot(scaleFactor, glm::vec3(1.f, 1.f, 1.f));
        //std::cout << "positive value: " << positiveValue << std::endl;

        if (positiveValue < 0){
          compLength *= -1.f;
        }
        scaleFactor = glm::vec3(compLength, compLength, compLength);
        //std::cout << "old scale = " << print(oldScaleFactor) << ", new scale = " << print(scaleFactor) << std::endl;
        modassert(aboutEqual(glm::length(scaleFactor), glm::length(oldScaleFactor)), "scale magnitude was not preserved");
      }

      for (auto &targetId : update.selectedObjs.selectedIds){
        std::cout << "scale factor: " << print(scaleFactor) << std::endl;
        auto initialTransform = getInitialTransformation(manipulatorState, targetId);
        auto initialDragScale = initialTransform.scale; 
        auto relativeScale = scaleFactor *  initialDragScale + initialDragScale;
        if (update.options.scalingGroup == GROUP_SCALING){
          auto scaleRatio = relativeScale / initialDragScale;
          auto offsetFromMean = initialTransform.position - manipulatorState.meanPosition.value();
          auto scaledOffsetFromMean = scaleRatio * offsetFromMean;
          std::cout << "scale ratio: " << print(scaleRatio) << std::endl;
          std::cout << "mean position: " << print(initialTransform.position - manipulatorState.meanPosition.value()) << std::endl;
          tools.setPosition(targetId, manipulatorState.meanPosition.value() + scaledOffsetFromMean);
        }
        tools.setScale(targetId, relativeScale);
      }
    },
    .nextStates = {
      ManipulatorNextState { .nextState = "scaleIdle", .transition = "axisReleased", .fn = manipulatorDoNothing },
      ManipulatorNextState { .nextState = "scaleIdle", .transition = "itemSelected", .fn = manipulatorDoNothing },
    },
  },
  ManipulatorState {
    .state = "rotateIdle",
    .onState = [](ManipulatorData& manipulatorState, ManipulatorTools& tools, ManipulatorUpdateInfo& update) -> void {
      modassert(update.selectedObjs.mainObj.has_value(), "cannot have no obj selected in this mode");
      auto rotateInfo = calcRotateInfo(manipulatorState, tools, update);
      manipulatorState.rotationAmount = rotateInfo.rotationAmount;
      visualizeSubrotations(tools, update, rotateInfo);
    },
    .nextStates = {
      ManipulatorNextState { .nextState = "idle", .transition = "unselected", .fn = manipulatorDoNothing },
      ManipulatorNextState { .nextState = "rotateMode", .transition = "mouseDown", .fn = manipulatorPopulateInitialPositions },
      ManipulatorNextState { .nextState = "scaleIdle", .transition = "gotoScaleIdle", .fn = manipulatorEnsureExists },
      ManipulatorNextState { .nextState = "translateIdle", .transition = "gotoTranslateIdle", .fn = manipulatorEnsureExists },
    },
  },
  ManipulatorState {
    .state = "rotateMode",
    .onState = [](ManipulatorData& manipulatorState, ManipulatorTools& tools, ManipulatorUpdateInfo& update) -> void {
        modassert(update.selectedObjs.mainObj.has_value(), "cannot have no obj selected in this mode");
        auto rotateInfo = calcRotateInfo(manipulatorState, tools, update);
        float rotationDiff = rotateInfo.rotationAmount - manipulatorState.rotationAmount;
        visualizeSubrotations(tools, update, rotateInfo);

        float extraRadiansCorrection = 0.f;
        if (update.options.rotateMode == SNAP_ABSOLUTE){
          auto targetId = update.selectedObjs.mainObj.value();
          auto mainObjTransform = getInitialTransformation(manipulatorState, update.selectedObjs.mainObj.value());
          auto mainobjDiff = mainObjTransform.position - rotateInfo.meanPosition;
          auto rotationOrientation = axisToOrientation(update.defaultAxis);

          if (aboutEqual(mainobjDiff, glm::vec3(0.f, 0.f, 0.f))){  // captured for single obj since mean is it's position, and stacked objected
            if (update.selectedObjs.selectedIds.size() > 1){
              modlog("manipulator", "rotation zero difference from mean", MODLOG_WARNING);  // not handled, theoretically should loop over objects and rotate them all?
              return;
            }
            auto relativeOrientation = glm::inverse(rotationOrientation) * mainObjTransform.rotation;
            auto mainobjRelative = relativeOrientation * glm::vec3(1.0f, 0.f, 0.f);
            float radiansValue = atanRadians360(mainobjRelative.x, mainobjRelative.y);
            extraRadiansCorrection = -1.f * radiansValue;
           
          }else{
            auto mainobjRelative = glm::inverse(rotationOrientation) * mainobjDiff;
            float radiansValue = atanRadians360(mainobjRelative.x, mainobjRelative.y);
            extraRadiansCorrection = -1.f * radiansValue;
          }
        }
        
        std::optional<std::function<glm::quat(glm::quat)>> snapFn = (update.options.rotateMode == SNAP_CONTINUOUS) ? std::optional<std::function<glm::quat(glm::quat)>>(std::nullopt) : [&update, &tools, extraRadiansCorrection](glm::quat rotation) -> glm::quat {
          return tools.snapRotate(rotation, update.defaultAxis, extraRadiansCorrection);
        };

        //std::cout << "manipulator = " << manipulatorState.rotationAmount << ", rotationAmount = " << rotateInfo.rotationAmount << std::endl;
        for (auto &targetId : update.selectedObjs.selectedIds){
          auto newTargetRotPos = rotateOverAxis(
              RotationPosition { 
                .position = getInitialTransformation(manipulatorState, targetId).position, 
                .rotation = getInitialTransformation(manipulatorState, targetId).rotation,
              },
              RotationPosition { .position = rotateInfo.meanPosition, .rotation = rotateInfo.rotationOrientation },
            rotationDiff,
            snapFn
          );
          tools.setPosition(targetId, newTargetRotPos.position);
          tools.setRotation(targetId, newTargetRotPos.rotation);
        }
    },
    .nextStates = {
      ManipulatorNextState { .nextState = "rotateIdle", .transition = "mouseUp", .fn = manipulatorDoNothing },
      ManipulatorNextState { .nextState = "rotateIdle", .transition = "itemSelected", .fn = manipulatorDoNothing },
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


ManipulatorNextState* manipulatorTransitionByName(ManipulatorState& manipulatorState, std::string transitionName){
  for (auto &state : manipulatorState.nextStates){
    if (state.transition == transitionName){
      return &state;
    }
  }
  return NULL;
}
void manipulatorHandleTransition(ManipulatorData& manipulatorState, ManipulatorTools& manipulatorTools, ManipulatorUpdateInfo& update, std::string transitionName){
  while (true){
    std::string currentStateName = manipulatorState.state;
    ManipulatorState& currentState = manipulatorStateByName(currentStateName);
    auto transition = manipulatorTransitionByName(currentState, transitionName);
    if (transition != NULL){
      // call transition fn
      transition -> fn(manipulatorState, manipulatorTools, update);
      ManipulatorState& nextState = manipulatorStateByName(transition -> nextState);
      manipulatorState.state = nextState.state;
    }else{
      break;
    }
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
  //modlog("manipulator", std::string("manipulator state: ") + manipulatorState.state);
  manipulatorTools.clearLines();

  auto selectedObjs = manipulatorTools.getSelectedIds();

  ManipulatorUpdateInfo updateInfo {
    .projection = projection,
    .cameraViewMatrix = cameraViewMatrix, 
    .mode = mode,
    .defaultAxis = defaultAxis,
    .mouseX = mouseX, 
    .mouseY = mouseY,
    .cursorPos = cursorPos,
    .screensize = screensize,
    .options = options,
    .selectedObjs = selectedObjs,
  };

  if (!selectedObjs.mainObj.has_value()){
    manipulatorHandleTransition(manipulatorState, manipulatorTools, updateInfo, "unselected");
    return;
  }
  if (mode == ROTATE){
    manipulatorHandleTransition(manipulatorState, manipulatorTools, updateInfo, "gotoRotateIdle");
  }
  if (mode == SCALE){
    manipulatorHandleTransition(manipulatorState, manipulatorTools, updateInfo, "gotoScaleIdle");
  }
  if (mode == TRANSLATE){
    manipulatorHandleTransition(manipulatorState, manipulatorTools, updateInfo, "gotoTranslateIdle");
  }
  if (manipulatorState.manipulatorObject == XAXIS || manipulatorState.manipulatorObject == YAXIS || manipulatorState.manipulatorObject == ZAXIS){
    manipulatorHandleTransition(manipulatorState, manipulatorTools, updateInfo, "axisSelected");
  }
  if (manipulatorState.mouseReleasedLastFrame){
    manipulatorState.mouseReleasedLastFrame = false;
    manipulatorState.manipulatorObject = NOAXIS;
    //manipulatorState.initialDragPosition = std::nullopt;
    manipulatorHandleTransition(manipulatorState, manipulatorTools, updateInfo, "axisReleased");
    manipulatorHandleTransition(manipulatorState, manipulatorTools, updateInfo, "mouseUp");
  }
  if (manipulatorState.mouseClickedLastFrame){
    manipulatorState.mouseClickedLastFrame = false;
    manipulatorHandleTransition(manipulatorState, manipulatorTools, updateInfo, "mouseDown");
  }

  if (manipulatorState.selectedItemLastFrame){
    manipulatorState.selectedItemLastFrame = false;
    manipulatorHandleTransition(manipulatorState, manipulatorTools, updateInfo, "itemSelected");
  }

  manipulatorStateByName(manipulatorState.state).onState(manipulatorState, manipulatorTools, updateInfo);
}

