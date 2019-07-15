#include "./camera.h"

glm::mat4 createModelViewMatrix(Camera camera){ 
  glm::vec3 direction = glm::normalize(glm::vec3(
     cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch)),
     sin(glm::radians(camera.pitch)),
     sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch))
  ));
  return glm::lookAt(camera.position, camera.position + direction, camera.up);
}

glm::vec3 moveRelativeToCamera(Camera camera, glm::vec3 direction){
   return camera.position + direction;
}
