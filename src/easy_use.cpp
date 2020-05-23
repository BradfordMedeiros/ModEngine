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

float getClosestAngle(float angle, int snapAngle, bool isUp){
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
    euler.x = getClosestAngle(euler.x, snapAngles.at(currentAngleIndex), isUp);
  }else if (rotationAxis == YAXIS){
    euler.y = getClosestAngle(euler.y, snapAngles.at(currentAngleIndex), isUp);
  }else if (rotationAxis == ZAXIS){
    euler.z = getClosestAngle(euler.z, snapAngles.at(currentAngleIndex), isUp);
  }else{
    assert(false);
  }
  glm::vec3 newAngle = glm::radians(glm::vec3(euler.x, euler.y, euler.z));
  glm::quat newRotation = glm::quat(newAngle);
  return newRotation;
}

glm::quat snapAngleUp(glm::quat angle, Axis rotationAxis){
  return snapAngle(angle, rotationAxis, true);
}
glm::quat snapAngleDown(glm::quat angle, Axis rotationAxis){
  return snapAngle(angle, rotationAxis, false);
}


static std::vector<float> snapTranslates = { 0.01, 0.1, 0.5, 1, 5, 10 };
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
float getClosestPosition(float position, float snapTranslate, bool isUp){
  int multiple = position / snapTranslate;
  if (isUp){
    return (multiple + 1) * snapTranslate; 
  }
  return (multiple - 1) * snapTranslate;
};
glm::vec3 snapTranslate(glm::vec3 current, Axis translationAxis, bool isUp){
  if (translationAxis == NOAXIS || translationAxis == XAXIS){
    float newX = getClosestPosition(current.x, snapTranslates.at(currentTranslateIndex), isUp);    
    return glm::vec3(newX, current.y, current.z);
  }else if (translationAxis == YAXIS){
    float newY = getClosestPosition(current.y, snapTranslates.at(currentTranslateIndex), isUp);
    return glm::vec3(current.x, newY, current.z);
  }else if (translationAxis == ZAXIS){
    float newZ = getClosestPosition(current.z, snapTranslates.at(currentTranslateIndex), isUp);
    return glm::vec3(current.x, current.y, newZ);
  }else{
    assert(false);
  }
  return glm::vec3(0.f, 0.f, 0.f);
}
glm::vec3 snapTranslateUp(glm::vec3 currentPos, Axis translationAxis){
  return snapTranslate(currentPos, translationAxis, true);
}
glm::vec3 snapTranslateDown(glm::vec3 currentPos, Axis translationAxis){
  return snapTranslate(currentPos, translationAxis, false);
}