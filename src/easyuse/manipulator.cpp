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
  std::function<GameObject&(objid)> getGameObject
){
  auto isTargetManipulator =  selectedItem == manipulatorId;
  auto manipulatorExists = manipulatorId != 0;

  if (!isTargetManipulator){
    if (!manipulatorExists){
      manipulatorId = makeManipulator();
    }
    manipulatorTarget = selectedItem;
    getGameObject(manipulatorId).transformation.position = getGameObject(manipulatorTarget).transformation.position;
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
void onManipulatorMouseRelease(std::function<GameObject&(objid)> getGameObject){
  manipulatorObject = NOAXIS;
  std::cout << "manipulator release!" << std::endl;
}

void onManipulatorUpdate(std::function<GameObject&(objid)> getGameObject, float mouseX, float mouseY){
  if (manipulatorId != 0 && manipulatorTarget != 0){
    GameObject& target = getGameObject(manipulatorTarget);
    GameObject& manipulator = getGameObject(manipulatorId);
    if (manipulatorObject == XAXIS){
      std::cout << "manipulator x axis" << std::endl;
      auto position = manipulator.transformation.position + glm::vec3(mouseX * 0.01f, 0.f, 0.f);
      manipulator.transformation.position = position;
      target.transformation.position = position;
    }else if (manipulatorObject == YAXIS){
      std::cout << "manipulator y axis" << std::endl;
      auto position = manipulator.transformation.position + glm::vec3(0.f, mouseY * -0.01f, 0.f);
      manipulator.transformation.position = position;
      target.transformation.position = position;
    }else if (manipulatorObject == ZAXIS){
      std::cout << "manipulator z axis" << std::endl;
      auto position = manipulator.transformation.position + glm::vec3(0.f, 0.0f, mouseX * 0.01f);
      manipulator.transformation.position = position;
      target.transformation.position = position;
    }
  }
