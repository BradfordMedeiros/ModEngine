#include "./manipulator.h"

auto manipulatorId = 0;
auto manipulatorTarget = 0;
Axis manipulatorObject = NOAXIS;

void unspawnManipulator(std::function<void(objid)> removeObjectById){
  std::cout << "unspawn manipulator called" << std::endl;
  if (manipulatorId != 0){
    removeObjectById(manipulatorId);
  }
  manipulatorId = 0;
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
      manipulatorTarget = 0;
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
  std::cout << "manipulator release!" << std::endl;
}

void onManipulatorUpdate(
  std::function<glm::vec3(objid)> getPosition, 
  std::function<void(objid, glm::vec3)> setPosition, 
  std::function<glm::vec3(objid)> getScale,
  std::function<void(objid, glm::vec3)> setScale,
  glm::mat4 cameraViewMatrix, 
  ManipulatorMode mode,
  float mouseX, 
  float mouseY
){
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

  if (manipulatorId != 0 && manipulatorTarget != 0){
    if (mode == TRANSLATE){
      auto targetPosition = getPosition(manipulatorTarget);
      auto manipulatorPosition = getPosition(manipulatorId);
      if (manipulatorObject == XAXIS){
        std::cout << "manipulator x axis" << std::endl;
        auto position = manipulatorPosition + glm::vec3(0.01f * moveVec.x, 0.f, 0.f);
        setPosition(manipulatorTarget, position);
        setPosition(manipulatorId, position);
      }else if (manipulatorObject == YAXIS){
        std::cout << "manipulator y axis" << std::endl;
        auto position = manipulatorPosition + glm::vec3(0.f, 0.01f * moveVec.y, 0.f);
        setPosition(manipulatorTarget, position);
        setPosition(manipulatorId, position);
      }else if (manipulatorObject == ZAXIS){
        std::cout << "manipulator z axis" << std::endl;
        auto position = manipulatorPosition + glm::vec3(0.f, 0.0f, 0.01f * moveVec.z);
        setPosition(manipulatorTarget, position);
        setPosition(manipulatorId, position);
      }
    }else if (mode == SCALE){
      auto targetScale = getScale(manipulatorTarget);
      if (manipulatorObject == XAXIS){
        std::cout << "manipulator x axis" << std::endl;
        auto scale = targetScale + glm::vec3(0.01f * moveVec.x, 0.f, 0.f);
        setScale(manipulatorTarget, scale);
      }else if (manipulatorObject == YAXIS){
        std::cout << "manipulator y axis" << std::endl;
        auto scale = targetScale + glm::vec3(0.f, 0.01f * moveVec.y, 0.f);
        setScale(manipulatorTarget, scale);
      }else if (manipulatorObject == ZAXIS){
        std::cout << "manipulator z axis" << std::endl;
        auto scale = targetScale + glm::vec3(0.f, 0.0f, 0.01f * moveVec.z);
        setScale(manipulatorTarget, scale);
      }      
    }
  }
}
