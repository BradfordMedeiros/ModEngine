#include "./camera.h"

glm::mat4 createCamera(glm::vec3 position){
   return glm::translate(glm::mat4(1.0f), position); 
}

