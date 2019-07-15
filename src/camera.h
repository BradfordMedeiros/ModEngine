#ifndef MOD_CAMERA
#define MOD_CAMERA
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
 
struct Camera{
   glm::vec3 position;
   glm::vec3 up;
   float yaw; 
   float pitch;
};

glm::mat4 createModelViewMatrix(Camera camera);
glm::vec3 moveRelativeToCamera(Camera camera, glm::vec3 direction);

#endif 
