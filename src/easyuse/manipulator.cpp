#include "./manipulator.h"

auto manipulatorId = 0;
Axis manipulatorObject = NOAXIS;

std::optional<glm::vec3> initialDragPosition = std::nullopt;
std::optional<glm::vec3> initialDragScale = std::nullopt;
std::optional<glm::quat> initialDragRotation = std::nullopt;

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
  initialDragScale = std::nullopt;
  initialDragRotation = std::nullopt;
}

struct ManipulatorUpdate {
  glm::vec3 manipulatorNew;
  glm::vec3 targetNew;
  bool shouldSet;
};


void drawDirectionalLine(std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, glm::vec3 fromPos, glm::vec3 direction, LineColor color){
  glm::vec3 normalizedDirection = glm::normalize(direction);
  auto rotation = quatFromDirection(normalizedDirection);
  for (int i = 0; i < 10; i++){
    auto newPos = fromPos + glm::vec3(normalizedDirection.x * i, normalizedDirection.y * i, normalizedDirection.z * i);
    auto leftDash = newPos + rotation * glm::vec3(-0.1f, -0.01f, 0.5f);
    auto rightDash = newPos + rotation * glm::vec3(0.1f, -0.01f, 0.5f);
    //std::cout << "drawLine from: " << print(leftDash) << " to " << print(rightDash) << std::endl;
    drawLine(leftDash, newPos, color);
    drawLine(rightDash, newPos, color);
  }
}

void drawHitMarker(std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, glm::vec3 position){
  float length = 5.f;
  drawLine(position + glm::vec3(0.f, length * 2.f, 0.f), position + glm::vec3(0.f, length * -2.f, 0.f), RED);
  drawLine(position + glm::vec3(length * 2.f, 0.f, 0.f), position + glm::vec3(length *  -2.f, 0.f, 0.f), RED);
  drawLine(position + glm::vec3(0.f, 0.f, length *  -2.f), position + glm::vec3(0.f, 0.f, length *  2.f), RED);
}




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

  clearLines();

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

struct ManipulatorTools {
  std::function<glm::vec3(objid)> getPosition;
  std::function<void(objid, glm::vec3)> setPosition;
  std::function<void(objid, glm::vec3)> setScale;
  std::function<void(objid, glm::quat)> setRotation;
  std::function<glm::vec3(glm::vec3)> snapPosition;
  std::function<glm::vec3(glm::vec3)> snapScale;
  std::function<glm::quat(glm::quat, Axis)> snapRotate;
};

void updateTransform(
  std::vector<objid> targets, objid manipulatorTarget, objid manipulatorId, 
  ManipulatorMode mode, ManipulatorOptions options, glm::vec3 projectedPosition, ManipulatorTools tools
){
  if (mode == TRANSLATE){
    auto oldPos = tools.getPosition(manipulatorTarget);
    auto positionDiff = glm::vec3(0.f, 0.f, 0.f);
    std::cout << "position diff: " << print(positionDiff) << std::endl;
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
    return;
  }

  //////////////////////////
  if (mode == SCALE) {
    if (!options.snapManipulatorScales){
      auto scaleFactor = calcPositionDiff(projectedPosition, tools.getPosition, true) + glm::vec3(1.f, 1.f, 1.f);
      auto relativeScale = scaleFactor *  initialDragScale.value();  
      tools.setScale(manipulatorTarget, relativeScale);
      return;
    }

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
    auto relativeScale = scaleFactor *  initialDragScale.value() + initialDragScale.value();  
    tools.setScale(manipulatorTarget, relativeScale);
    return;
  } 


  if (mode == ROTATE){
    auto positionDiff = calcPositionDiff(projectedPosition, tools.getPosition, false);
    auto xRotation = (positionDiff.x / 3.1416) * 360;  // not quite right
    auto yRotation = (positionDiff.y / 3.1416) * 360;  // not quite right
    auto zRotation = (positionDiff.z / 3.1416) * 360;  // not quite right

   if (!options.snapManipulatorAngles){
      tools.setRotation(manipulatorTarget,
        setFrontDelta(glm::identity<glm::quat>(), yRotation, xRotation, zRotation, 0.01f) *
        initialDragRotation.value() 
      );
      return;
    }

    int numXRotates = (int)(xRotation / 90.f);
    int numYRotates = (int)(yRotation / 90.f);
    int numZRotates = (int)(zRotation / 90.f);

    std::cout << "rotations: (" << print(glm::vec3(xRotation, yRotation, zRotation)) << ")" << std::endl;
    std::cout << "num rotates: (" << print(glm::vec3(numXRotates, numYRotates, numZRotates)) << ")" << std::endl;

    int numRotates = 0;
    if (manipulatorObject == XAXIS){
      numRotates = numXRotates;
    }else if (manipulatorObject == YAXIS){
      numRotates = numYRotates;
    }else if (manipulatorObject == ZAXIS){
      numRotates = numZRotates;
    }
    std::cout << "num rotates: " << numRotates << std::endl;

    if (options.rotateSnapRelative){
      auto newRotation = tools.snapRotate(setFrontDelta(glm::identity<glm::quat>(), yRotation, xRotation, zRotation, 0.01f), manipulatorObject) ;
      tools.setRotation(manipulatorTarget, newRotation * initialDragRotation.value());          
    }else{
      auto newRotation = setFrontDelta(glm::identity<glm::quat>(), yRotation, xRotation, zRotation, 0.01f) ;
      auto snappedRotation = tools.snapRotate( newRotation * initialDragRotation.value(), manipulatorObject);
      tools.setRotation(manipulatorTarget, snappedRotation);
    }
  }

}

void onManipulatorUpdate(
  std::function<ManipulatorSelection()> getSelectedIds,
  std::function<objid(void)> makeManipulator,
  std::function<void(objid)> removeObjectById,
  std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine,
  std::function<void()> clearLines,
  std::function<glm::vec3(objid)> getPosition, 
  std::function<void(objid, glm::vec3)> setPosition, 
  std::function<glm::vec3(objid)> getScale,
  std::function<void(objid, glm::vec3)> setScale,
  std::function<glm::quat(objid)> getRotation,
  std::function<void(objid, glm::quat)> setRotation,
  glm::mat4 projection,
  glm::mat4 cameraViewMatrix, 
  ManipulatorMode mode,
  float mouseX, 
  float mouseY,
  glm::vec2 cursorPos,
  glm::vec2 screensize,
  std::function<glm::vec3(glm::vec3)> snapPosition,
  std::function<glm::vec3(glm::vec3)> snapScale,
  std::function<glm::quat(glm::quat, Axis)> snapRotate,
  ManipulatorOptions options
){

  auto selectedObjs = getSelectedIds();

  //// create manipulator
  if (selectedObjs.mainObj.has_value()){
    auto manipulatorExists = manipulatorId != 0;
    if (!manipulatorExists){
      manipulatorId = makeManipulator();
    }
    setPosition(manipulatorId, getPosition(selectedObjs.mainObj.value()));
  }

  //////// delete manipulator
  if (!selectedObjs.mainObj.has_value()){
    unspawnManipulator(removeObjectById);
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

  if (manipulatorId == 0 || manipulatorTarget == 0){
    return;
  }

  if (manipulatorObject != XAXIS && manipulatorObject != YAXIS && manipulatorObject != ZAXIS){
    setPosition(manipulatorId, getPosition(manipulatorTarget));
    return;
  }
  auto projectedPosition = projectCursor(manipulatorTarget, drawLine, clearLines, getPosition, projection, cameraViewMatrix, cursorPos, screensize, manipulatorObject);
  if (!initialDragPosition.has_value()){
    initialDragPosition = projectedPosition;  
    initialDragScale = getScale(manipulatorTarget);
    initialDragRotation = getRotation(manipulatorTarget);
  }
    
  /// actual do the transformation
  updateTransform(selectedObjs.selectedIds, manipulatorTarget, manipulatorId, mode, options, projectedPosition, 
    ManipulatorTools {
      .getPosition = getPosition,
      .setPosition = setPosition,
      .setScale = setScale,
      .setRotation = setRotation,
      .snapPosition = snapPosition,
      .snapScale = snapScale,
      .snapRotate = snapRotate,
    }
  );
}

