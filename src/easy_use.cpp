#include "./easy_use.h"

static std::vector<int> snapAngles = { 15, 30, 45, 90, 180 };
static int currentAngleIndex = 0;

void setSnapAngleUp(){
  currentAngleIndex = (currentAngleIndex + 1) % snapAngles.size();
  std::cout << "Snap angle is now: " << snapAngles.at(currentAngleIndex) << std::endl;
}
void setSnapAngleDown(){
  currentAngleIndex = (currentAngleIndex - 1);
  if (currentAngleIndex < 0){
    currentAngleIndex = snapAngles.size() - 1;
  }
  std::cout << "Snap angle is now: " << snapAngles.at(currentAngleIndex) << std::endl;
}

float getClosestNumber(float current, int snapAngle, bool isUp){
  return current + snapAngle;
}

glm::quat snapAngle(glm::quat angle, Axis rotationAxis, bool isUp){
  glm::vec3 euler = glm::degrees(glm::eulerAngles(angle));
 
  if (rotationAxis == NOAXIS || rotationAxis == XAXIS){
    euler.x = getClosestNumber(euler.x, snapAngles.at(currentAngleIndex), isUp);
  }else if (rotationAxis == YAXIS){
    euler.y = getClosestNumber(euler.y, snapAngles.at(currentAngleIndex), isUp);
  }else if (rotationAxis == ZAXIS){
    euler.z = getClosestNumber(euler.z, snapAngles.at(currentAngleIndex), isUp);
  }else{
    assert(false);
  }
  glm::vec3 newAngle = glm::radians(glm::vec3(euler.x, euler.y, euler.z));
  glm::quat newRotation = glm::quat(newAngle);
  return newRotation;
}

glm::quat snapAngleUp(glm::quat angle, Axis rotationAxis){
  std::cout << "snap angle up placeholder" << std::endl;
  return snapAngle(angle, rotationAxis, true);
}
glm::quat snapAngleDown(glm::quat angle, Axis rotationAxis){
  std::cout << "snap angle down placeholder" << std::endl;
  return snapAngle(angle, rotationAxis, false);
}
