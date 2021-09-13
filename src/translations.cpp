#include "./translations.h"

glm::quat setFrontDelta(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta){ // Might be worth considering the ordering of this. 
  glm::quat pitch  = angleAxis(-1 * deltaPitch * delta, glm::vec3(1.0f, 0.0f, 0.0f)) ;
  glm::quat yaw = glm::angleAxis(-1 * deltaYaw * delta,  glm::vec3(0.0f, 1.0f, 0.0f));
  glm::quat roll = glm::angleAxis(-1 * deltaRoll * delta,  glm::vec3(0.0f, 0.0f, 1.0f));
  return roll * yaw * orientation * pitch;                                      
}

glm::vec3 calculateRelativeOffset(glm::quat orientation, glm::vec3 offset, bool xzPlaneOnly){
  glm::vec3 delta = orientation * offset;
  return (xzPlaneOnly ? glm::vec3(delta.x, 0, delta.z) : delta);
}

glm::vec3 moveRelative(glm::vec3 position, glm::quat orientation, glm::vec3 offset, bool xzPlaneOnly){
  return position + calculateRelativeOffset(orientation, offset, xzPlaneOnly);   // @TODO This loses magnitude if you do have upwards velocity.  This should really be just projecting offset onto xz plane
}

glm::mat4 renderView(glm::vec3 position, glm::quat orientation){
  glm::mat4 cameraModelMatrix =  glm::translate(glm::mat4(1.f), glm::vec3(position.x, position.y, position.z)) * glm::toMat4(orientation);
  return glm::inverse(cameraModelMatrix);
}

glm::vec3 getVecAxis(Axis axis){
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
glm::vec3 applyTranslation(glm::vec3 position, float offsetX, float offsetY, Axis manipulatorAxis){
  glm::vec3 axis = getVecAxis(manipulatorAxis);
  glm::vec3 translate = getVecTranslate(offsetX, offsetY);
  return position + axis * translate;
}
glm::vec3 applyScaling(glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, Axis manipulatorAxis){
  // The scaling operation needs to figure out if you should make it bigger by making it 
  // more positive, or by making it more negative.  Therefore, need to use the objects position 
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
glm::quat applyRotation(glm::quat currentOrientation, float offsetX, float offsetY, Axis manipulatorAxis){
  float deltaPitch = manipulatorAxis == XAXIS ? offsetY : 0;
  float deltaYaw = manipulatorAxis == YAXIS ? offsetX : 0;
  float deltaRoll = manipulatorAxis == ZAXIS ? offsetX : 0;
  return setFrontDelta(currentOrientation, deltaYaw, deltaPitch, deltaRoll, 0.1);
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

glm::quat quatFromDirection(glm::vec3 direction){ 
  return orientationFromPos(glm::vec3(0.f, 0.f, 0.f), direction);
}
glm::vec3 directionFromQuat(glm::quat direction){
  return direction * glm::vec3(0, 1, 0);
}

