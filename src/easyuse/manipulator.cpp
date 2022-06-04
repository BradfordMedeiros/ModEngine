#include "./manipulator.h"

auto manipulatorId = 0;
auto manipulatorTarget = 0;
Axis manipulatorObject = NOAXIS;

std::optional<glm::vec3> initialDragPosition = std::nullopt;
std::optional<glm::vec3> initialDragScale = std::nullopt;
std::optional<glm::quat> initialDragRotation = std::nullopt;

objid getManipulatorId(){
  return manipulatorId;
}
void unspawnManipulator(std::function<void(objid)> removeObjectById){
  std::cout << "unspawn manipulator called" << std::endl;
  if (manipulatorId != 0){
    removeObjectById(manipulatorId);
  }
  manipulatorId = 0;
  manipulatorTarget = 0;
}

void onManipulatorSelectItem(
  objid selectedItem, 
  std::string selectedItemName,
  std::function<objid(void)> makeManipulator,
  std::function<void(objid)> removeObjectById,
  std::function<glm::vec3(objid)> getPosition,
  std::function<void(objid, glm::vec3)> setPosition
){
  auto isTargetManipulator =  selectedItem == manipulatorId;
  auto manipulatorExists = manipulatorId != 0;

  if (!isTargetManipulator){
    if (!manipulatorExists){
      manipulatorId = makeManipulator();
    }
    if (manipulatorTarget == selectedItem){
      unspawnManipulator(removeObjectById);
    }else{
      manipulatorTarget = selectedItem;
      setPosition(manipulatorId, getPosition(manipulatorTarget));
    }
  }else{
    std::cout << "item name is: " << selectedItemName << std::endl;
    if (selectedItemName == "manipulator/xaxis"){
      std::cout << "settting manipulator to xaxis!" << std::endl;
      manipulatorObject = XAXIS;
    }else if (selectedItemName == "manipulator/yaxis"){
      std::cout << "settting manipulator to yaxis!" << std::endl;
      manipulatorObject = YAXIS;
    }else if (selectedItemName == "manipulator/zaxis"){
      std::cout << "settting manipulator to zaxis!" << std::endl;
      manipulatorObject = ZAXIS;
    }
  }
}
void onManipulatorMouseRelease(){
  manipulatorObject = NOAXIS;
  std::cout << "reset initial drag position" << std::endl;
  initialDragPosition = std::nullopt;
  initialDragScale = std::nullopt;
  initialDragRotation = std::nullopt;
}

struct ManipulatorTarget {
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




bool manipulatorInstantClickMode = true;
bool drawDebugLines = true;
glm::vec3 projectCursor(std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, std::function<void()> clearLines, std::function<glm::vec3(objid)> getPosition, glm::mat4 projection, glm::mat4 view, glm::vec2 cursorPos, glm::vec2 screensize, Axis axis){
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
ManipulatorTarget newManipulatorValues(
  std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine,
  std::function<void()> clearLines,
  std::function<glm::vec3(objid)> getPosition, 
  std::function<glm::vec3(objid)> getScale,
  glm::mat4 projection, 
  glm::mat4 cameraViewMatrix, 
  ManipulatorMode mode,
  float mouseX, 
  float mouseY,
  glm::vec2 cursorPos,
  glm::vec2 screensize
){
  glm::vec4 moveVector = cameraViewMatrix * glm::vec4(mouseX, mouseY, 0, 0.f);
  glm::vec3 vecc = glm::vec3(moveVector.x, moveVector.y, moveVector.z);
  auto xVector = glm::dot(vecc, glm::vec3(1.f, 0.f, 0.f));
  auto yVector = glm::dot(vecc, glm::vec3(0.f, 1.f, 0.f));
  auto zVector = glm::dot(vecc, glm::vec3(0.f, 0.f, -1.f));
  auto moveVec = glm::vec3(xVector, yVector, zVector);
  if (mode == TRANSLATE){
    auto targetPosition = getPosition(manipulatorTarget);
    auto manipulatorPosition = getPosition(manipulatorId);
    if (manipulatorObject == XAXIS){
      auto position = manipulatorPosition + glm::vec3(0.01f * moveVec.x, 0.f, 0.f);
      return ManipulatorTarget {
        .manipulatorNew = position,
        .targetNew = position,
        .shouldSet = true,
      };
    }else if (manipulatorObject == YAXIS){
      auto position = manipulatorPosition + glm::vec3(0.f, 0.01f * moveVec.y, 0.f);
      return ManipulatorTarget {
        .manipulatorNew = position,
        .targetNew = position,
        .shouldSet = true,
      };
    }else if (manipulatorObject == ZAXIS){
      auto position = manipulatorPosition + glm::vec3(0.f, 0.0f, 0.01f * moveVec.z);
      return ManipulatorTarget {
        .manipulatorNew = position,
        .targetNew = position,
        .shouldSet = true,
      };
    }
  }else if (mode == SCALE){
    auto targetScale = getScale(manipulatorTarget);
    if (manipulatorObject == XAXIS){
      auto scale = targetScale + glm::vec3(0.01f * moveVec.x, 0.f, 0.f);
      return ManipulatorTarget {
        .manipulatorNew = scale,
        .targetNew = scale,
        .shouldSet = true,
      };
    }else if (manipulatorObject == YAXIS){
      auto scale = targetScale + glm::vec3(0.f, 0.01f * moveVec.y, 0.f);
      return ManipulatorTarget {
        .manipulatorNew = scale,
        .targetNew = scale,
        .shouldSet = true,
      };
    }else if (manipulatorObject == ZAXIS){
      auto scale = targetScale + glm::vec3(0.f, 0.0f, 0.01f * moveVec.z);
      return ManipulatorTarget {
        .manipulatorNew = scale,
        .targetNew = scale,
        .shouldSet = true,
      };
    } 
  }
  return ManipulatorTarget {
    .manipulatorNew = glm::vec3(0.f, 0.f, 0.f),
    .targetNew = glm::vec3(0.f, 0.f, 0.f),
    .shouldSet = false,
  };
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

void onManipulatorUpdate(
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
  if (mouseX < 10 && mouseX > -10.f){
    mouseX = 0.f;
  }
  if (mouseY < 10 && mouseY > -10.f){
    mouseY = 0.f;
  }

  if (manipulatorId != 0 && manipulatorTarget != 0){
    if (manipulatorInstantClickMode){
      if (manipulatorObject != XAXIS && manipulatorObject != YAXIS && manipulatorObject != ZAXIS){
        setPosition(manipulatorId, getPosition(manipulatorTarget));
        return;
      }
      auto projectedPosition = projectCursor(drawLine, clearLines, getPosition, projection, cameraViewMatrix, cursorPos, screensize, manipulatorObject);
      if (!initialDragPosition.has_value()){
        initialDragPosition = projectedPosition;  
        initialDragScale = getScale(manipulatorTarget);
        initialDragRotation = getRotation(manipulatorTarget);
      }
    
      if (mode == TRANSLATE){
        if (!options.snapManipulatorPositions){
          setPosition(manipulatorTarget, projectedPosition);
          setPosition(manipulatorId, projectedPosition);
          return;
        }
        auto newPosition = snapPosition(projectedPosition);
        setPosition(manipulatorTarget, newPosition);
        setPosition(manipulatorId, newPosition);
      }else if (mode == SCALE) {
        if (!options.snapManipulatorScales){
          auto scaleFactor = calcPositionDiff(projectedPosition, getPosition, true) + glm::vec3(1.f, 1.f, 1.f);
          auto relativeScale = scaleFactor *  initialDragScale.value();  
          setScale(manipulatorTarget, relativeScale);
          return;
        }

        auto positionDiff = calcPositionDiff(projectedPosition, getPosition, true);
        auto scaleFactor = snapScale(positionDiff) ;
        if (options.preserveRelativeScale){  // makes the increase in scale magnitude proportion to length of the vec
          auto vecLength = glm::length(scaleFactor);
          bool negX = positionDiff.x < 0.f;
          bool negY = positionDiff.y < 0.f;
          bool negZ = positionDiff.z < 0.f;
          auto compLength = glm::sqrt(vecLength * vecLength / 3.f);  // because sqrt(x^2 + y^2 + z^2) =  sqrt(3x^2) = veclength  
          scaleFactor = glm::vec3(compLength * (negX ? -1.f : 1.f), compLength * (negY ? -1.f : 1.f), compLength * (negZ ? -1.f : 1.f));
        }
        auto relativeScale = scaleFactor *  initialDragScale.value() + initialDragScale.value();  
        setScale(manipulatorTarget, relativeScale);
      }else if (mode == ROTATE){
        auto positionDiff = calcPositionDiff(projectedPosition, getPosition, false);
        auto xRotation = (positionDiff.x / 3.1416) * 360;  // not quite right
        auto yRotation = (positionDiff.y / 3.1416) * 360;  // not quite right
        auto zRotation = (positionDiff.z / 3.1416) * 360;  // not quite right

        if (!options.snapManipulatorAngles){
          setRotation(manipulatorTarget,
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
          auto newRotation = snapRotate(setFrontDelta(glm::identity<glm::quat>(), yRotation, xRotation, zRotation, 0.01f), manipulatorObject) ;
          setRotation(manipulatorTarget, newRotation * initialDragRotation.value());          
        }else{
          auto newRotation =  setFrontDelta(glm::identity<glm::quat>(), yRotation, xRotation, zRotation, 0.01f) ;
          auto snappedRotation = snapRotate( newRotation * initialDragRotation.value(), manipulatorObject);
          setRotation(manipulatorTarget, snappedRotation);
        }
      }
    }else{
      auto newValues = newManipulatorValues(drawLine, clearLines, getPosition, getScale, projection, cameraViewMatrix, mode, mouseX, mouseY, cursorPos, screensize);
      if (!newValues.shouldSet){
        return;
      }
      if (mode == TRANSLATE){
        setPosition(manipulatorTarget, newValues.manipulatorNew);
        setPosition(manipulatorId, newValues.targetNew);
      }else if (mode == SCALE){
        setScale(manipulatorTarget, newValues.targetNew);
      }
    }

  }
}

void onManipulatorUnselect(std::function<void(objid)> removeObjectById){
  std::cout << "on manipulator unselect" << std::endl;
  unspawnManipulator(removeObjectById);
}

void onManipulatorIdRemoved(objid id, std::function<void(objid)> removeObjectById){
  if (id == manipulatorTarget){
    onManipulatorUnselect(removeObjectById);
  }
}