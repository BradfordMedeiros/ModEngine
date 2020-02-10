#include "./translations.h"

glm::quat setFrontDelta(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta){
  glm::quat yawRotation = glm::rotate(orientation, glm::radians(deltaYaw * delta), glm::vec3(0.0f, -1.0f, 0.0f));
  glm::quat rotatedYaw = glm::rotate(yawRotation, glm::radians(deltaPitch * delta), glm::vec3(-1.0f, 0.0f, 0.0f) * yawRotation);
  glm::quat rotated = glm::rotate(rotatedYaw, glm::radians(deltaRoll * delta), glm::vec3(0.0f, 0.0f, -1.0f) * rotatedYaw);
  return rotated;
}

glm::vec3 moveRelative(glm::vec3 position, glm::quat orientation, glm::vec3 offset){
  return position + (offset * orientation);
}
glm::vec3 move(glm::vec3 position, glm::vec3 offset){
  return glm::vec3(position.x + offset.x, position.y + offset.y, position.z + offset.z);
}
glm::mat4 renderView(glm::vec3 position, glm::quat orientation){
  return glm::translate(glm::toMat4(orientation), glm::vec3(position.x * -1, position.y * - 1, position.z * -1));
}

glm::vec3 getVecAxis(ManipulatorAxis axis){
  if (axis == XAXIS){
    return glm::vec3(1.0f, 0.f, 0.f);
  }else if (axis == YAXIS){
    return glm::vec3(0.f, 1.0f, 0.f);
  }else if (axis == ZAXIS){
    return glm::vec3(0.f, 0.f, 1.0f);
  }
  return glm::vec3(1.0f, 0.0f, 1.0f);
}
glm::vec3 getVecTranslate(float offsetX, float offsetY){
  return glm::vec3(-offsetX, offsetY, offsetY);
}
glm::vec3 applyTranslation(glm::vec3 position, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis){
  glm::vec3 axis = getVecAxis(manipulatorAxis);
  glm::vec3 translate = getVecTranslate(offsetX, offsetY);
  return position + axis * translate;
}
glm::vec3 applyScaling(glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis){
  float distanceOld = glm::distance(position, glm::vec3(lastX, lastY, 0));
  float distanceNew = glm::distance(position, glm::vec3(lastX + offsetX, lastY + offsetY, 0));

  if (distanceNew < distanceOld){
    return initialScale - getVecAxis(manipulatorAxis) * 0.1f;
  }else if (distanceNew > distanceOld){
    return initialScale + getVecAxis(manipulatorAxis) * 0.1f;
  }else{ 
    return initialScale;
  }
}
glm::quat applyRotation(glm::quat currentOrientation, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis){
  float deltaYaw = manipulatorAxis == XAXIS ? offsetY : 0;
  float deltaPitch = manipulatorAxis == YAXIS ? offsetX : 0;
  float deltaRoll = manipulatorAxis == ZAXIS ? offsetX : 0;
  return setFrontDelta(currentOrientation, deltaYaw, deltaPitch, deltaRoll, 1);
}

float convertBase(float value, float fromBaseLow, float fromBaseHigh, float toBaseLow, float toBaseHigh){
  return ((value - fromBaseLow) * ((toBaseHigh - toBaseLow) / (fromBaseHigh - fromBaseLow))) + toBaseLow;
}
glm::vec3 getCursorRayDirection(glm::mat4 projection, glm::mat4 view, float cursorLeft, float cursorTop, float screenWidth, float screenHeight){
  glm::mat4 inversionMatrix = glm::inverse(projection * view);
  float screenXPosNdi = convertBase(cursorLeft, 0.f, screenWidth, -1.f, 1.f);
  float screenYPosNdi = convertBase(cursorTop, 0.f, screenHeight, -1.f, 1.f);
  glm::vec4 direction = inversionMatrix * glm::vec4(screenXPosNdi, -screenYPosNdi, 1.0f, 1.0f);
  return glm::normalize(glm::vec3(direction.x, direction.y, direction.z));
}