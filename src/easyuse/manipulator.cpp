#include "./manipulator.h"

auto manipulatorId = 0;
auto manipulatorTarget = 0;
Axis manipulatorObject = NOAXIS;

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
ManipulatorTarget newValuesInstanceClick(std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, std::function<void()> clearLines, std::function<glm::vec3(objid)> getPosition, glm::mat4 projection, glm::mat4 view, glm::vec2 cursorPos, glm::vec2 screensize, Axis axis){
  if (axis != XAXIS && axis != YAXIS && axis != ZAXIS){
    return ManipulatorTarget {
      .manipulatorNew = glm::vec3(0.f, 0.f, 0.f),
      .targetNew = glm::vec3(0.f, 0.f, 0.f),
      .shouldSet = false,
    };
  }

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

  return ManipulatorTarget {
    .manipulatorNew = newPosition,
    .targetNew = newPosition,
    .shouldSet = true,
  };
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
  if (manipulatorInstantClickMode){
    return newValuesInstanceClick(drawLine, clearLines, getPosition, projection, cameraViewMatrix, cursorPos, screensize, manipulatorObject);
  }

  if (mouseX < 10 && mouseX > -10.f){
    mouseX = 0.f;
  }
  if (mouseY < 10 && mouseY > -10.f){
    mouseY = 0.f;
  }
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

void onManipulatorUpdate(
  std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine,
  std::function<void()> clearLines,
  std::function<glm::vec3(objid)> getPosition, 
  std::function<void(objid, glm::vec3)> setPosition, 
  std::function<glm::vec3(objid)> getScale,
  std::function<void(objid, glm::vec3)> setScale,
  glm::mat4 projection,
  glm::mat4 cameraViewMatrix, 
  ManipulatorMode mode,
  float mouseX, 
  float mouseY,
  glm::vec2 cursorPos,
  glm::vec2 screensize
){
  if (manipulatorId != 0 && manipulatorTarget != 0){
    auto newValues = newManipulatorValues(drawLine, clearLines, getPosition, getScale, projection, cameraViewMatrix, mode, mouseX, mouseY, cursorPos, screensize);
    //std::cout << "info: manipulator: (shouldset, id, target, movevec) => (" << newValues.shouldSet << ", " << manipulatorId << ", " << manipulatorTarget << ", " << print(newValues.targetNew) << ")" << std::endl; 
    if (!newValues.shouldSet){
      return;
    }
    if (mode == TRANSLATE){
      setPosition(manipulatorTarget, newValues.manipulatorNew);
      setPosition(manipulatorId, newValues.targetNew);
    }else if (mode == SCALE){
      std::cout << "set scale: " << print(newValues.targetNew) << std::endl;
      setScale(manipulatorTarget, newValues.targetNew);
    }
  }
}

void onManipulatorUnselect(std::function<void(objid)> removeObjectById){
  std::cout << "on manipulator unselect" << std::endl;
  unspawnManipulator(removeObjectById);
}