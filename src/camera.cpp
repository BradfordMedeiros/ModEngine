#include "./camera.h"

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
