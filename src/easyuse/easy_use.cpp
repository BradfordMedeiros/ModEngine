#include "./easy_use.h"

EasyUseInfo createEasyUse(){
  return EasyUseInfo {
    .currentAngleIndex = 0,
    .currentTranslateIndex = 0,
    .currentScaleIndex = 0,
  };
}

static std::vector<int> snapAngles = { 1, 5, 15, 30, 45, 90, 180 };
void setSnapAngleUp(EasyUseInfo& easyUse){
  easyUse.currentAngleIndex = (easyUse.currentAngleIndex + 1) % snapAngles.size();
  std::cout << "Snap angle is now: " << snapAngles.at(easyUse.currentAngleIndex) << std::endl;
}
void setSnapAngleDown(EasyUseInfo& easyUse){
  easyUse.currentAngleIndex = (easyUse.currentAngleIndex - 1);
  if (easyUse.currentAngleIndex < 0){
    easyUse.currentAngleIndex = snapAngles.size() - 1;
  }
  std::cout << "Snap angle is now: " << snapAngles.at(easyUse.currentAngleIndex) << std::endl;
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
  return 0.f;
}
glm::quat snapAngle(EasyUseInfo& easyUse, glm::quat angle, Axis rotationAxis, bool isUp, SNAPPING_MODE mode){
  glm::vec3 euler = glm::degrees(glm::eulerAngles(angle));
  auto deltaAngle = snapAngles.at(easyUse.currentAngleIndex);
  if (mode == SNAP_RELATIVE){
    auto multiplier = isUp ? 1.f : -1.f;
    if (rotationAxis == NOAXIS || rotationAxis == XAXIS){
      euler.x = euler.x + (multiplier * deltaAngle);
    }else if (rotationAxis == YAXIS){
      euler.y = euler.y + (multiplier * deltaAngle);
    }else if (rotationAxis == ZAXIS){
      euler.z = euler.z + (multiplier * deltaAngle);
    }else{
      assert(false);
    }    
  }else {
    if (rotationAxis == NOAXIS || rotationAxis == XAXIS){
      euler.x = getClosestAngle(euler.x, deltaAngle, isUp);
    }else if (rotationAxis == YAXIS){
      euler.y = getClosestAngle(euler.y, deltaAngle, isUp);
    }else if (rotationAxis == ZAXIS){
      euler.z = getClosestAngle(euler.z, deltaAngle, isUp);
    }else{
      assert(false);
    }
  }  
  glm::vec3 newAngle = glm::radians(glm::vec3(euler.x, euler.y, euler.z));
  glm::quat newRotation = glm::quat(newAngle);
  return newRotation;
}
glm::quat snapAngleUp(EasyUseInfo& easyUse, SNAPPING_MODE mode, glm::quat angle, Axis rotationAxis){
  return snapAngle(easyUse, angle, rotationAxis, true, mode);
}
glm::quat snapAngleDown(EasyUseInfo& easyUse, SNAPPING_MODE mode, glm::quat angle, Axis rotationAxis){
  return snapAngle(easyUse, angle, rotationAxis, false, mode);
}

float getClosestPosition(float position, float snapAmount, bool up){
  int multiple = position / snapAmount;
  if (up){
    return (multiple + 1) * snapAmount; 
  }
  return (multiple - 1) * snapAmount;
};
float getClosestPosition(float position, float snapAmount){
  int multiple = position / snapAmount;
  float amount = multiple * snapAmount;
  float amountAndOne = (multiple + 1) * snapAmount;
  float diffAmount = glm::abs(position - amount);
  float diffAmountAndOne = glm::abs(position - amountAndOne);
  if (diffAmount < diffAmountAndOne){
    return amount;
  }
  return amountAndOne;
};

glm::vec3 snapVector(glm::vec3 current, Axis translationAxis, bool isUp, float snapAmount, SNAPPING_MODE mode){
  if (mode == SNAP_RELATIVE){
    if (translationAxis == NOAXIS || translationAxis == XAXIS){
      return glm::vec3((isUp ? 1 : -1) * snapAmount + current.x, current.y, current.z);
    }else if (translationAxis == YAXIS){
      return glm::vec3(current.x, (isUp ? 1 : -1) * snapAmount + current.y, current.z);
    }else if (translationAxis == ZAXIS){
      return glm::vec3(current.x, current.y, (isUp ? 1 : -1) * snapAmount + current.z);
    }   
  }else {
    if (mode == SNAP_CONTINUOUS){
      snapAmount = 0.1f;
    }
    if (translationAxis == NOAXIS || translationAxis == XAXIS){
      float newX = getClosestPosition(current.x, snapAmount, isUp);    
      return glm::vec3(newX, current.y, current.z);
    }else if (translationAxis == YAXIS){
      float newY = getClosestPosition(current.y, snapAmount, isUp);
      return glm::vec3(current.x, newY, current.z);
    }else if (translationAxis == ZAXIS){
      float newZ = getClosestPosition(current.z, snapAmount, isUp);
      return glm::vec3(current.x, current.y, newZ);
    }
  }
  assert(false);
  return glm::vec3(0.f, 0.f, 0.f);
}

glm::vec3 snapVector(glm::vec3 position, float snapAmount){
  float newX = getClosestPosition(position.x, snapAmount);    
  float newY = getClosestPosition(position.y, snapAmount);    
  float newZ = getClosestPosition(position.z, snapAmount);    
  auto snappedPosition = glm::vec3(newX, newY, newZ);
  //std::cout << "Current snapAmount: " << snapAmount << std::endl;
  //std::cout << "rounding:" << print(position) << " to " << print(snappedPosition) << std::endl;
  return snappedPosition;
}

glm::quat snapRotate(EasyUseInfo& easyUse, glm::quat newRotation, Axis snapAxis, float extraRadians){
  auto deltaAngle = snapAngles.at(easyUse.currentAngleIndex);
  bool snapX = snapAxis == XAXIS || snapAxis == NOAXIS;
  bool snapY = snapAxis == YAXIS || snapAxis == NOAXIS;
  bool snapZ = snapAxis == ZAXIS || snapAxis == NOAXIS;
  //std::cout << "snap angle is: " << deltaAngle << std::endl;
  glm::vec3 eulerNew = glm::degrees(glm::eulerAngles(newRotation));

  auto snappedAngleX = snapX ? getClosestPosition(eulerNew.x, deltaAngle) : eulerNew.x;    
  auto snappedAngleY = snapY ? getClosestPosition(eulerNew.y, deltaAngle) : eulerNew.y;    
  auto snappedAngleZ = snapZ ? getClosestPosition(eulerNew.z, deltaAngle) : eulerNew.z;    

  glm::vec3 extraRadiansVec(snapX ? extraRadians : 0.f, snapY ? extraRadians : 0.f, snapZ ? extraRadians : 0.f);
  glm::vec3 newAngle = glm::radians(glm::vec3(snappedAngleX, snappedAngleY, snappedAngleZ)) + extraRadiansVec;
  glm::quat newRot = glm::quat(newAngle);
  return newRot;
}

static std::vector<float> snapTranslates = { 0.01, 0.1, 0.5, 1, 5, 10 };
void setSnapTranslateUp(EasyUseInfo& easyUse){
  easyUse.currentTranslateIndex = (easyUse.currentTranslateIndex + 1) % snapTranslates.size();
  std::cout << "Snap translate is now: " << snapTranslates.at(easyUse.currentTranslateIndex) << std::endl;
}
void setSnapTranslateDown(EasyUseInfo& easyUse){
  easyUse.currentTranslateIndex = (easyUse.currentTranslateIndex - 1);
  if (easyUse.currentTranslateIndex < 0){
    easyUse.currentTranslateIndex = snapTranslates.size() - 1;
  }
  std::cout << "Snap translate is now: " << snapTranslates.at(easyUse.currentTranslateIndex) << std::endl;
}
glm::vec3 snapTranslateUp(EasyUseInfo& easyUse, SNAPPING_MODE mode, glm::vec3 currentPos, Axis translationAxis){
  return snapVector(currentPos, translationAxis, true, snapTranslates.at(easyUse.currentTranslateIndex), mode);
}
glm::vec3 snapTranslateDown(EasyUseInfo& easyUse, SNAPPING_MODE mode, glm::vec3 currentPos, Axis translationAxis){
  return snapVector(currentPos, translationAxis, false, snapTranslates.at(easyUse.currentTranslateIndex), mode);
}

glm::vec3 snapTranslate(EasyUseInfo& easyUse, glm::vec3 position){
  auto snapAmount = snapTranslates.at(easyUse.currentTranslateIndex);
  return snapVector(position, snapAmount);
}

static std::vector<float> snapScales = { 0.01, 0.1, 0.5, 1, 5, 10 };
void setSnapScaleUp(EasyUseInfo& easyUse){
  easyUse.currentScaleIndex = (easyUse.currentScaleIndex + 1) % snapScales.size();
  std::cout << "Snap scale is now: " << snapScales.at(easyUse.currentScaleIndex) << std::endl;
}
void setSnapScaleDown(EasyUseInfo& easyUse){
  easyUse.currentScaleIndex = (easyUse.currentScaleIndex - 1);
  if (easyUse.currentScaleIndex < 0){
    easyUse.currentScaleIndex = snapScales.size() - 1;
  }
  std::cout << "Snap scale is now: " << snapScales.at(easyUse.currentScaleIndex) << std::endl;
}
glm::vec3 snapScaleUp(EasyUseInfo& easyUse, SNAPPING_MODE mode, glm::vec3 currentScale, Axis translationAxis){
  return snapVector(currentScale, translationAxis, true, snapScales.at(easyUse.currentScaleIndex), mode);
}
glm::vec3 snapScaleDown(EasyUseInfo& easyUse, SNAPPING_MODE mode, glm::vec3 currentScale, Axis translationAxis){
  return snapVector(currentScale, translationAxis, false, snapScales.at(easyUse.currentScaleIndex), mode);
}
glm::vec3 snapScale(EasyUseInfo& easyUse, glm::vec3 scale){
  auto snapAmount = snapScales.at(easyUse.currentScaleIndex);
  return snapVector(scale, snapAmount);
}

float getSnapTranslateSize(EasyUseInfo& easyUse){
  return snapTranslates.at(easyUse.currentTranslateIndex);
}

void setSnapEasyUseUp(EasyUseInfo& easyUse, ManipulatorMode manipulatorMode){
  if (manipulatorMode == NONE || manipulatorMode == TRANSLATE){
    setSnapTranslateUp(easyUse);
  }else if (manipulatorMode == ROTATE){
    setSnapAngleUp(easyUse);
  }else if (manipulatorMode == SCALE){
    setSnapScaleUp(easyUse);
  }
}

void setSnapEasyUseDown(EasyUseInfo& easyUse, ManipulatorMode manipulatorMode){
  if (manipulatorMode == NONE || manipulatorMode == TRANSLATE){
    setSnapTranslateDown(easyUse);
  }else if (manipulatorMode == ROTATE){
    setSnapAngleDown(easyUse);
  }else if (manipulatorMode == SCALE){
    setSnapScaleDown(easyUse);
  }
}


void snapCameraForward(std::function<void(glm::quat)> orientation){
  orientation(quatFromDirection(glm::vec3(0.f, 0.f, -1.f)));
}
void snapCameraBackward(std::function<void(glm::quat)> orientation){
  orientation(quatFromDirection(glm::vec3(0.f, 0.f, 1.f)));
}
void snapCameraUp(std::function<void(glm::quat)> orientation){
  orientation(quatFromDirection(glm::vec3(0.f, 1.f, 0.f)));
}
void snapCameraDown(std::function<void(glm::quat)> orientation){
  orientation(quatFromDirection(glm::vec3(0.f, -1.f, 0.f)));
}
void snapCameraLeft(std::function<void(glm::quat)> orientation){
  orientation(quatFromDirection(glm::vec3(-1.f, 0.f, 0.f)));
}
void snapCameraRight(std::function<void(glm::quat)> orientation){
  orientation(quatFromDirection(glm::vec3(1.f, 0.f, 0.f)));
}