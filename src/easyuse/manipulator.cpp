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
  std::function<glm::quat(objid)> getRotation;
  std::function<void(objid, glm::quat)> setRotation;
  std::function<glm::vec3(glm::vec3)> snapPosition;
  std::function<glm::vec3(glm::vec3)> snapScale;
  std::function<glm::quat(glm::quat, Axis)> snapRotate;
  std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine;
};

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

int sphereNumPoints = 30;
void drawRotationVisualization(std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, glm::vec3 position, glm::quat orientation, float radius){
  float radianPerPoint = 2 * MODPI / sphereNumPoints;

  glm::vec3 lastPos(0.f, 0.f, 0.f);
  glm::vec3 firstPos(0.f, 0.f, 0.f);
  for (int i = 0; i < sphereNumPoints; i++){
    auto xPos = glm::cos(radianPerPoint * i) * radius;
    auto yPos = glm::sin(radianPerPoint * i) * radius;
    auto deltaCirclePos = orientation * glm::vec3(xPos, yPos, 0.f);
    auto newPos = position + glm::vec3(deltaCirclePos.x, deltaCirclePos.y, deltaCirclePos.z);
    if (i > 0){
      drawLine(lastPos, newPos, RED);
    }else{
      firstPos = newPos;
    }
    lastPos = newPos;
  }
  drawLine(lastPos, firstPos, RED);
}

glm::quat axisToOrientation(Axis manipulatorObject){
  if (manipulatorObject == XAXIS){
    return MOD_ORIENTATION_RIGHT;
  }
  if (manipulatorObject == YAXIS){
    return MOD_ORIENTATION_UP;
  }
  if (manipulatorObject == ZAXIS){
    return MOD_ORIENTATION_FORWARD;
  }
  modassert(false, "axis to orientation invalid orientation");
  return glm::identity<glm::quat>();
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

float atanRadians360(float x, float y){
  auto angle = glm::atan(y / x);
  if (x < 0){
    return angle + MODPI;
  }
  return angle;
}

void visualizeRotation(std::vector<objid> targets, glm::vec3 meanPosition, glm::quat rotationOrientation, ManipulatorTools& tools, glm::mat4 cameraViewMatrix, glm::vec3 selectDir){
  float maxDistance = 0.f;
  if (targets.size() > 1){
    for (auto &targetId : targets){
      auto targetPosition = tools.getPosition(targetId);
      auto distanceToTarget = glm::distance(targetPosition, meanPosition);
      if (distanceToTarget > maxDistance){
        maxDistance = distanceToTarget;
      }
      drawRotationVisualization(tools.drawLine, meanPosition, rotationOrientation, distanceToTarget);
    }
  }

  auto lineAmount = rotationOrientation * glm::vec3(0, 0.f, -5.f);
  tools.drawLine(meanPosition + lineAmount, meanPosition - lineAmount, GREEN);

  auto position = getTransformationFromMatrix(glm::inverse(cameraViewMatrix)).position;
  auto vecDirection = directionFromQuat(rotationOrientation);

  ////////////////////////////////////
  // visualization for the selection dir
  glm::vec3 bias(-0.01f, -0.01f, 0.f);
  tools.drawLine(position + bias, position + glm::vec3(selectDir.x, selectDir.y, selectDir.z), RED);
  bias = glm::vec3(0.01f, 0.01f, 0.f);
  tools.drawLine(position + bias, position + glm::vec3(selectDir.x , selectDir.y, selectDir.z ), RED);

  bias = glm::vec3(0.01f, -0.01f, 0.f);
  tools.drawLine(position + bias, position +  glm::vec3(selectDir.x, selectDir.y, selectDir.z ), RED);
  bias = glm::vec3(-0.01f, 0.01f, 0.f);
  tools.drawLine(position + bias, position + glm::vec3(selectDir.x , selectDir.y , selectDir.z), RED);
  //////////////////////////////////////////////////

  auto optIntersection = findPlaneIntersection(meanPosition, vecDirection, position, selectDir);
  if (!optIntersection.has_value()){
    std::cout << "no intersection" << std::endl;
    return;
  }
  auto intersection = optIntersection.value();
  auto pointRelativeToPlane = glm::inverse(rotationOrientation) * (intersection - meanPosition);
  auto angle = atanRadians360(pointRelativeToPlane.x, pointRelativeToPlane.y);
  auto angleDegrees = glm::degrees(angle);

  std::cout << "position: " << print(position) << std::endl;
  std::cout << "selecdir: " << print(selectDir) << std::endl;
  std::cout << "intersection: " << print(intersection) << std::endl;
  std::cout << "xyz to plane: " << print(pointRelativeToPlane) << std::endl;
  std::cout << "angle: " << angleDegrees << std::endl;

  tools.drawLine(intersection, intersection + rotationOrientation * glm::vec3(0.1f, 0.1f, 0.f), RED);
  tools.drawLine(intersection, intersection + rotationOrientation * glm::vec3(-0.1f, -0.1f, 0.f), RED);
  tools.drawLine(intersection, intersection + rotationOrientation * glm::vec3(-0.1f, 0.1f, 0.f), RED);
  tools.drawLine(intersection, intersection + rotationOrientation * glm::vec3(0.1f, -0.1f, 0.f), RED);

  auto angleIndicator = rotationOrientation * (glm::normalize(glm::vec3(glm::cos(angle), glm::sin(angle), 0.f)) * maxDistance);
  tools.drawLine(meanPosition, meanPosition + angleIndicator, GREEN);


}

void handleRotate(
  glm::vec3 meanPosition, glm::quat rotationOrientation,
  glm::vec3 projectedPosition,
  std::vector<objid> targets, objid manipulatorTarget, objid manipulatorId, 
  ManipulatorOptions& options, ManipulatorTools& tools
){

  ////

  auto positionDiff = calcPositionDiff(projectedPosition, tools.getPosition, false);
  auto xRotation = (positionDiff.x / MODPI) * 360;  // not quite right
  auto yRotation = (positionDiff.y / MODPI) * 360;  // not quite right
  auto zRotation = (positionDiff.z / MODPI) * 360;  // not quite right

  if (targets.size() > 1){
  // this is effectively a different rotation type and the ui probably ought tp be different
    modlog("manipulator", std::string("rotation mean position: ") + print(meanPosition));

    float rotationAmount = 0.f;
    if (manipulatorObject == XAXIS){
      rotationAmount = xRotation;
    }else if (manipulatorObject == YAXIS){
      rotationAmount = yRotation;
    }else if (manipulatorObject == ZAXIS){
      rotationAmount = zRotation;
    }
    for (auto &targetId : targets){
      auto newTargetRotPos = rotateOverAxis(
        RotationPosition { .position = tools.getPosition(targetId), .rotation = tools.getRotation(targetId) },
        RotationPosition { .position = meanPosition, .rotation = rotationOrientation },
        rotationAmount
      );
      tools.setPosition(targetId, newTargetRotPos.position);
      tools.setRotation(targetId, newTargetRotPos.rotation);
    }
    return;
  }
  ////////////////////////////

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
  Axis defaultAxis,
  float mouseX, 
  float mouseY,
  glm::vec2 cursorPos,
  glm::vec2 screensize,
  std::function<glm::vec3(glm::vec3)> snapPosition,
  std::function<glm::vec3(glm::vec3)> snapScale,
  std::function<glm::quat(glm::quat, Axis)> snapRotate,
  ManipulatorOptions options
){

  clearLines();

  auto selectedObjs = getSelectedIds();
  handleShowManipulator(selectedObjs, setPosition, getPosition, removeObjectById, makeManipulator, mode);
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

  ManipulatorTools manipulatorTools {
    .getPosition = getPosition,
    .setPosition = setPosition,
    .setScale = setScale,
    .getRotation = getRotation,
    .setRotation = setRotation,
    .snapPosition = snapPosition,
    .snapScale = snapScale,
    .snapRotate = snapRotate,
    .drawLine = drawLine,
  };
  if (mode == ROTATE){
    auto meanPosition = calcMeanPosition(selectedObjs.selectedIds, manipulatorTools.getPosition);
    auto rotationOrientation = axisToOrientation(defaultAxis);

    auto selectDir = getCursorRayDirection(projection, cameraViewMatrix, cursorPos.x, cursorPos.y, screensize.x, screensize.y);

    visualizeRotation(selectedObjs.selectedIds, meanPosition, rotationOrientation, manipulatorTools, cameraViewMatrix, selectDir);
  }

  if (manipulatorId == 0 || manipulatorTarget == 0){
    return;
  }

  std::cout << "on manipulate update" << std::endl;


  if (manipulatorObject != XAXIS && manipulatorObject != YAXIS && manipulatorObject != ZAXIS){
    setPosition(manipulatorId, getPosition(manipulatorTarget));
    return;
  }


  auto projectedPosition = projectCursor(manipulatorTarget, drawLine, clearLines, getPosition, projection, cameraViewMatrix, cursorPos, screensize, manipulatorObject);
  if (!initialDragPosition.has_value()){
    initialDragPosition = projectedPosition;  
    initialDragScales = {};
    initialDragRotations = {};
    for (auto &selectedId : selectedObjs.selectedIds){
      initialDragScales.push_back(InitialDragScale {
        .id = selectedId,
        .scale = getScale(selectedId)
      });
      initialDragRotations.push_back(InitialDragRotation {
        .id = selectedId,
        .rotation = getRotation(selectedId)
      });
    }
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
    handleRotate(meanPosition, rotationOrientation, projectedPosition, selectedObjs.selectedIds, manipulatorTarget, manipulatorId, options, manipulatorTools);
    return;
  }
}

