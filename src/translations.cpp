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
  return glm::translate(glm::toMat4(orientation), position);
}

glm::vec3 getVecAxis(){
  return glm::vec3(1.0f, 0.0f, 1.0f);
}
glm::vec3 getVecTranslate(float offsetX, float offsetY){
  return glm::vec3(-offsetX, offsetY, offsetY);
}
glm::vec3 applyTranslation(glm::vec3 position, float offsetX, float offsetY){
  glm::vec3 axis = getVecAxis();
  glm::vec3 translate = getVecTranslate(offsetX, offsetY);
  return position + axis * translate;
}
glm::vec3 applyScaling(glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY){
  float distanceOld = glm::distance(position, glm::vec3(lastX, lastY, 0));
  float distanceNew = glm::distance(position, glm::vec3(lastX + offsetX, lastY + offsetY, 0));

  if (distanceNew < distanceOld){
    return initialScale - glm::vec3(0.1f, 0.1f, 0.1f);
  }else if (distanceNew > distanceOld){
    return initialScale + glm::vec3(0.1f, 0.1f, 0.1f);
  }else{ 
    return initialScale;
  }
}
glm::quat applyRotation(glm::quat currentOrientation, float offsetX, float offsetY){
  return setFrontDelta(currentOrientation, offsetX, offsetY, 0, 1);
}