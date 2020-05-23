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
glm::quat snapAngleUp(glm::quat angle, Axis rotationAxis){
  std::cout << "snap angle up placeholder" << std::endl;
  return angle;
}
glm::quat snapAngleDown(glm::quat angle, Axis rotationAxis){
  std::cout << "snap angle down placeholder" << std::endl;
  return angle;
}
