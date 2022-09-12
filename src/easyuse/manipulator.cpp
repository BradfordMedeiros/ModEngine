#include "./manipulator.h"

auto manipulatorId = 0;
Axis manipulatorObject = NOAXIS;

std::optional<glm::vec3> initialDragPosition = std::nullopt;

struct InitialDragScale {
  objid id;
  glm::vec3 scale;
};
std::vector<InitialDragScale> initialDragScales = {};

struct InitialDragRotation {
  objid id;
  glm::quat rotation;
};
std::vector<InitialDragRotation> initialDragRotations = {};

struct InitialDragPosition {
  objid id;
  glm::vec3 position;
};
std::vector<InitialDragPosition> initialDragPositions = {};

objid getManipulatorId(){
  return manipulatorId;
}
void unspawnManipulator(std::function<void(objid)> removeObjectById){
  if (manipulatorId != 0){
    removeObjectById(manipulatorId);
  }
  manipulatorId = 0;
}

void onManipulatorSelectItem(objid selectedItem, std::string selectedItemName){
  auto isTargetManipulator =  selectedItem == manipulatorId;
  if (isTargetManipulator){
    modlog("manipulator", std::string("item name is: ") + selectedItemName);
    if (selectedItemName == "manipulator/xaxis"){
      modlog("manipulator", "setting manipulator to xaxis");
      manipulatorObject = XAXIS;
    }else if (selectedItemName == "manipulator/yaxis"){
      modlog("manipulator", "setting manipulator to yaxis");
      manipulatorObject = YAXIS;
    }else if (selectedItemName == "manipulator/zaxis"){
      modlog("manipulator", "setting manipulator to zaxis");
      manipulatorObject = ZAXIS;
    }
    return;
  }
}
void onManipulatorMouseRelease(){
  manipulatorObject = NOAXIS;
  std::cout << "reset initial drag position" << std::endl;
  initialDragPosition = std::nullopt;
  initialDragScales = {};
  initialDragRotations = {};
}

struct ManipulatorUpdate {
  glm::vec3 manipulatorNew;
  glm::vec3 targetNew;
  bool shouldSet;
};



bool drawDebugLines = true;
glm::vec3 projectCursor(objid manipulatorTarget, std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, std::function<void()> clearLines, std::function<glm::vec3(objid)> getPosition, glm::mat4 projection, glm::mat4 view, glm::vec2 cursorPos, glm::vec2 screensize, Axis axis){
  ProjectCursorDebugInfo projectCursorInfo{};
  auto newPosition = projectCursorPositionOntoAxis(
    projection,
    view,
    cursorPos,  
    screensize, 
    axis,  
    getPosition(manipulatorTarget),
    &projectCursorInfo
  );

  // actual lengths
  drawHitMarker(drawLine, projectCursorInfo.intersectionPoint);

  if (drawDebugLines){
    drawLine(projectCursorInfo.positionFrom, projectCursorInfo.intersectionPoint, RED);
    drawLine(projectCursorInfo.positionFrom, projectCursorInfo.projectedTarget, GREEN);
    drawLine(projectCursorInfo.positionFrom, projectCursorInfo.target, BLUE);
  
    // directions
    drawDirectionalLine(drawLine, projectCursorInfo.positionFrom, projectCursorInfo.selectDir, BLUE);
    drawDirectionalLine(drawLine, projectCursorInfo.positionFrom, projectCursorInfo.targetAxis, RED);
  }

  return newPosition;
}

//  2 - 2 = 0 units, so 1x original scale
//  3 - 2 = 1 units, so 2x original scale
//  4 - 2 = 2 units, so 3x original scale 
// for negative
// (-2) - (-2) = 0 units, so 1x original scale
// (-3) - (-2) = -1 units, so 2x original scale
glm::vec3 calcPositionDiff(glm::vec3 projectedPosition, std::function<glm::vec3(objid)> getPosition, bool reverseOnMiddle){
  auto manipulatorPosition = getPosition(manipulatorId);
  auto effectProjPos = projectedPosition;
  auto effectInitialPos = initialDragPosition.value();
  if (reverseOnMiddle){
    bool draggingMoreNegX = projectedPosition.x < manipulatorPosition.x;
    bool draggingMoreNegY = projectedPosition.y < manipulatorPosition.y;
    bool draggingMoreNegZ = projectedPosition.z < manipulatorPosition.z;
    if (draggingMoreNegX){
      effectProjPos.x *= -1;
      effectInitialPos.x *= -1;
    }
    if (draggingMoreNegY){
      effectProjPos.y *= -1;
      effectInitialPos.y *= -1;
    }
    if (draggingMoreNegZ){
      effectProjPos.z *= -1;
      effectInitialPos.z *= -1;         
    }
  }
  //std::cout << "draggin more neg: " << draggingMoreNegX << " " << draggingMoreNegY << " " << draggingMoreNegZ << std::endl
  auto positionDiff = effectProjPos - effectInitialPos;  
  return positionDiff;
}

glm::vec3 findDragScale(objid id){
  for (auto &dragScale : initialDragScales){
    if (dragScale.id == id){
      return dragScale.scale;
    }
  }
  modassert(false, std::string("could not find manipulatorTarget = " + std::to_string(id)));
  return glm::vec3(0.f, 0.f, 0.f);
}
glm::quat findDragRotation(objid id){
  for (auto &dragRotation : initialDragRotations){
    if (dragRotation.id == id){
      return dragRotation.rotation;
    }
  }
  modassert(false, std::string("could not find manipulatorTarget = " + std::to_string(id)));
  return glm::identity<glm::quat>();
}
glm::vec3 findDragPosition(objid id){
  for (auto &dragPosition : initialDragPositions){
    if (dragPosition.id == id){
      return dragPosition.position;
    }
  }
  modassert(false, std::string("could not find manipulatorTarget = " + std::to_string(id)));
  return glm::vec3(0.f, 0.f, 0.f);
}

glm::vec3 calcMeanPosition(std::vector<objid>& targets, std::function<glm::vec3(objid)> getPosition){
  glm::vec3 positionSum(0.f, 0.f, 0.f);
  for (auto targetId : targets){
    auto position = getPosition(targetId);
    positionSum.x += position.x;
    positionSum.y += position.y;
    positionSum.z += position.z;
  }
  return glm::vec3(positionSum.x / targets.size(), positionSum.y / targets.size(), positionSum.z / targets.size());
}

void handleTranslate(
  glm::vec3 projectedPosition,
  std::vector<objid> targets, objid manipulatorTarget, objid manipulatorId, 
  ManipulatorOptions& options, ManipulatorTools& tools
){
  auto oldPos = tools.getPosition(manipulatorTarget);
  auto positionDiff = glm::vec3(0.f, 0.f, 0.f);
  //std::cout << "position diff: " << print(positionDiff) << std::endl;
  if (!options.snapManipulatorPositions){
    tools.setPosition(manipulatorId, projectedPosition);
    positionDiff = projectedPosition - oldPos;
  }else{
    auto newPosition = tools.snapPosition(projectedPosition);
    tools.setPosition(manipulatorId, newPosition);  
    positionDiff = newPosition - oldPos;    
  }
  for (auto &targetId : targets){
    auto oldPosition = tools.getPosition(targetId);
    auto newPosition = oldPosition + positionDiff;
    tools.setPosition(targetId, newPosition);
  }
}

void handleScale(
  glm::vec3 projectedPosition,
  std::vector<objid> targets, objid manipulatorTarget, objid manipulatorId, 
  ManipulatorOptions& options, ManipulatorTools& tools
){
  if (!options.snapManipulatorScales){
    auto scaleFactor = calcPositionDiff(projectedPosition, tools.getPosition, true) + glm::vec3(1.f, 1.f, 1.f);
    for (auto &targetId : targets){
      auto relativeScale = scaleFactor *  findDragScale(targetId);  
      tools.setScale(targetId, relativeScale);
    }
  }else{
    auto positionDiff = calcPositionDiff(projectedPosition, tools.getPosition, true);
    auto scaleFactor = tools.snapScale(positionDiff);
    if (options.preserveRelativeScale){  // makes the increase in scale magnitude proportion to length of the vec
      auto vecLength = glm::length(scaleFactor);
      bool negX = positionDiff.x < 0.f;
      bool negY = positionDiff.y < 0.f;
      bool negZ = positionDiff.z < 0.f;
      auto compLength = glm::sqrt(vecLength * vecLength / 3.f);  // because sqrt(x^2 + y^2 + z^2) =  sqrt(3x^2) = veclength  
      scaleFactor = glm::vec3(compLength * (negX ? -1.f : 1.f), compLength * (negY ? -1.f : 1.f), compLength * (negZ ? -1.f : 1.f));
    }
    
    for (auto &targetId : targets){
      std::cout << "scale factor: " << print(scaleFactor) << std::endl;
      auto initialDragScale = findDragScale(targetId);
      auto relativeScale = scaleFactor *  initialDragScale + initialDragScale;
      tools.setScale(targetId, relativeScale);
    }
  }
}

void handleRotate(
  glm::vec3 meanPosition, glm::quat rotationOrientation,
  glm::vec3 projectedPosition,
  std::vector<objid> targets, objid manipulatorTarget, objid manipulatorId, 
  ManipulatorOptions& options, ManipulatorTools& tools,
  float rotationAmount
){

  // this is effectively a different rotation type and the ui probably ought tp be different
  if (targets.size() > 1){
    modlog("manipulator", std::string("rotation mean position: ") + print(meanPosition));
    std::cout << "rotating: " << rotationAmount << std::endl;

    for (auto &targetId : targets){
      auto newTargetRotPos = rotateOverAxis(
        //RotationPosition { .position = tools.getPosition(targetId), .rotation = tools.getRotation(targetId) },
        RotationPosition { .position = findDragPosition(targetId), .rotation = findDragRotation(targetId) },
        RotationPosition { .position = meanPosition, .rotation = rotationOrientation },
        rotationAmount
      );
      tools.setPosition(targetId, newTargetRotPos.position);
      tools.setRotation(targetId, newTargetRotPos.rotation);
    }
    return;
  }
  ////////////////////////////

  auto positionDiff = calcPositionDiff(projectedPosition, tools.getPosition, false);
  auto xRotation = (positionDiff.x / MODPI) * 360;  // not quite right
  auto yRotation = (positionDiff.y / MODPI) * 360;  // not quite right
  auto zRotation = (positionDiff.z / MODPI) * 360;  // not quite right

  if (!options.snapManipulatorAngles){
    for (auto &targetId : targets){
      tools.setRotation(targetId,
        setFrontDelta(glm::identity<glm::quat>(), yRotation, xRotation, zRotation, 0.01f) *
        findDragRotation(targetId)
      );
    }
  }else if (options.rotateSnapRelative){
    auto newRotation = tools.snapRotate(setFrontDelta(glm::identity<glm::quat>(), yRotation, xRotation, zRotation, 0.01f), manipulatorObject) ;
    for (auto &targetId : targets){
      tools.setRotation(targetId, newRotation * findDragRotation(targetId));
    }
  }else{
    auto newRotation = setFrontDelta(glm::identity<glm::quat>(), yRotation, xRotation, zRotation, 0.01f) ;
    for (auto &targetId : targets){
      auto snappedRotation = tools.snapRotate( newRotation * findDragRotation(targetId), manipulatorObject);
      tools.setRotation(targetId, snappedRotation);
    }
  }
}

void handleShowManipulator(
  ManipulatorSelection& selectedObjs, 
  std::function<void(objid, glm::vec3)> setPosition, std::function<glm::vec3(objid)> getPosition, 
  std::function<void(objid)> removeObjectById, std::function<objid(void)> makeManipulator,
  ManipulatorMode mode
){
  if (!selectedObjs.mainObj.has_value() || (mode == ROTATE && selectedObjs.selectedIds.size() > 1)){
    unspawnManipulator(removeObjectById);
  }
  else if (selectedObjs.mainObj.has_value()){
    auto manipulatorExists = manipulatorId != 0;
    if (!manipulatorExists){
      manipulatorId = makeManipulator();
    }
    setPosition(manipulatorId, getPosition(selectedObjs.mainObj.value()));
  }
}

void onManipulatorUpdate(
  std::function<ManipulatorSelection()> getSelectedIds,
  std::function<objid(void)> makeManipulator,
  std::function<void(objid)> removeObjectById,
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

  manipulatorTools.clearLines();

  auto selectedObjs = getSelectedIds();
  handleShowManipulator(selectedObjs, manipulatorTools.setPosition, manipulatorTools.getPosition, removeObjectById, makeManipulator, mode);
  if (!selectedObjs.mainObj.has_value()){
    return;
  }
  //////////////////////////////

  auto manipulatorTarget = selectedObjs.mainObj.value();
  ///////////////////////////

  if (mouseX < 10 && mouseX > -10.f){
    mouseX = 0.f;
  }
  if (mouseY < 10 && mouseY > -10.f){
    mouseY = 0.f;
  }

  //if (manipulatorId == 0 || manipulatorTarget == 0){
  //  return;
  //}

//  //if (manipulatorObject != XAXIS && manipulatorObject != YAXIS && manipulatorObject != ZAXIS){
//  //  setPosition(manipulatorId, getPosition(manipulatorTarget));
//  //  //return;
  //}

  auto projectedPosition = projectCursor(manipulatorTarget, manipulatorTools.drawLine, manipulatorTools.clearLines, manipulatorTools.getPosition, projection, cameraViewMatrix, cursorPos, screensize, manipulatorObject);
  if (!initialDragPosition.has_value()){
    initialDragPosition = projectedPosition;
    initialDragPositions = {};
    initialDragScales = {};
    initialDragRotations = {};
    for (auto &selectedId : selectedObjs.selectedIds){
      initialDragPositions.push_back(InitialDragPosition{
        .id = selectedId,
        .position = manipulatorTools.getPosition(selectedId),
      });
      initialDragScales.push_back(InitialDragScale {
        .id = selectedId,
        .scale = manipulatorTools.getScale(selectedId)
      });
      initialDragRotations.push_back(InitialDragRotation {
        .id = selectedId,
        .rotation = manipulatorTools.getRotation(selectedId)
      });
    }
  }

  float rotationAmount = 0.f;
  if (mode == ROTATE){
    auto meanPosition = calcMeanPosition(selectedObjs.selectedIds, manipulatorTools.getPosition);
    auto rotationOrientation = axisToOrientation(defaultAxis);
    auto selectDir = getCursorRayDirection(projection, cameraViewMatrix, cursorPos.x, cursorPos.y, screensize.x, screensize.y);

    std::vector<IdVec3Pair> positions;
    for (auto &id : selectedObjs.selectedIds){
      positions.push_back(IdVec3Pair{
        .id = id,
        .value = manipulatorTools.getPosition(id),
      });
    }

    auto cameraPosition = getTransformationFromMatrix(glm::inverse(cameraViewMatrix)).position; 
    auto vecDirection = directionFromQuat(rotationOrientation);
    auto optIntersection = findPlaneIntersection(meanPosition, vecDirection, cameraPosition, selectDir);
    modassert(optIntersection.has_value(), "visualize rotation - could not find intersection");
    auto intersection = optIntersection.value();
    auto pointRelativeToPlane = glm::inverse(rotationOrientation) * (intersection - meanPosition);
    rotationAmount = atanRadians360(pointRelativeToPlane.x, pointRelativeToPlane.y);
    std::cout << "position: " << print(cameraPosition) << std::endl;
    std::cout << "selecdir: " << print(selectDir) << std::endl;
    std::cout << "intersection: " << print(intersection) << std::endl;
    std::cout << "xyz to plane: " << print(pointRelativeToPlane) << std::endl;
    std::cout << "angle: " << glm::degrees(rotationAmount) << std::endl;

    drawRotation(positions, meanPosition, rotationOrientation, cameraPosition, selectDir, intersection, rotationAmount, manipulatorTools.drawLine);
  }


  if (mode == TRANSLATE){
    handleTranslate(projectedPosition, selectedObjs.selectedIds, manipulatorTarget, manipulatorId, options, manipulatorTools);
    return;
  }
  if (mode == SCALE){
    handleScale(projectedPosition, selectedObjs.selectedIds, manipulatorTarget, manipulatorId, options, manipulatorTools);
    return;
  }
  if (mode == ROTATE){
    auto meanPosition = calcMeanPosition(selectedObjs.selectedIds, manipulatorTools.getPosition);
    auto rotationOrientation = axisToOrientation(defaultAxis);
    handleRotate(meanPosition, rotationOrientation, projectedPosition, selectedObjs.selectedIds, manipulatorTarget, manipulatorId, options, manipulatorTools, rotationAmount);
    return;
  }
}


/*

glm::vec3 valueForIdVec3(std::vector<IdVec3Pair> values, objid id){
  for (auto &value : values){
    if (value.id == id){
      return value.value;
    }
  }
  modassert(false, std::string("no id found for: " + std::to_string(id)));
  return glm::vec3(0.f, 0.f, 0.f);
}

*/