#include "./camera.h"

glm::mat4 createCamera(glm::vec3 initialPosition){
   return glm::translate(glm::mat4(1.0f), initialPosition); 
}

