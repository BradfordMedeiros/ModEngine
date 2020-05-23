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

float getClosestNumber(float angle, int snapAngle, bool isUp){
  int numIterations = (360 / snapAngle) + 1;
  float current = angle < 0 ? (360 + angle) : angle;
  assert(current >= 0);

  if (isUp){
    for (int i = -numIterations; i <= numIterations; i++){
      int angleToSnapTo = snapAngle * i;
      if ((int)current < angleToSnapTo){
        return angleToSnapTo;
      }
    }  
  }else{
    for (int i = numIterations; i >= -numIterations; i--){
      int angleToSnapTo = snapAngle * i;
      if ((int)current > angleToSnapTo){
        return angleToSnapTo;
      }
    }  
  }
  assert(false);
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


static std::vector<float> snapTranslates = { 15, 30, 45, 90, 180 };
static int currentTranslateIndex = 0;

void setSnapTranslateUp(){
  currentTranslateIndex = (currentTranslateIndex + 1) % snapTranslates.size();
  std::cout << "Snap translate is now: " << snapTranslates.at(currentTranslateIndex) << std::endl;
}
void setSnapTranslateDown(){
  currentTranslateIndex = (currentTranslateIndex - 1);
  if (currentTranslateIndex < 0){
    currentTranslateIndex = snapAngles.size() - 1;
  }
  std::cout << "Snap translate is now: " << snapTranslates.at(currentTranslateIndex) << std::endl;
}
glm::vec3 snapTranslateUp(glm::vec3 currentPos, Axis translationAxis){

}
glm::vec3 snapTranslateDown(glm::vec3 currentPos, Axis translationAxis){

}