#ifndef MOD_CAMERA
#define MOD_CAMERA
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

class Camera {
private:
  glm::vec3 front, up;
  float speed;
  float sensitivity;
  float pitch, yaw;
  void setFront(float yaw, float pitch);
public:
  glm::vec3 position;
  Camera(glm::vec3 position, glm::vec3 up, float speed, float sensitivity, float pitch, float yaw);
  void moveFront(float deltaTime);
  void moveBack(float deltaTime);
  void moveLeft(float deltaTime);
  void moveRight(float deltaTime);
  void setFrontDelta(float deltaYaw, float deltaPitch, float deltaTime);
  glm::mat4 renderView();
};

glm::quat setFromDeltaTo(glm::quat orientation, float deltaYaw, float deltaPitch);
glm::vec3 moveRelativeTo(glm::vec3 position, glm::quat orientation, glm::vec3 offset);
glm::vec3 moveTo(glm::vec3 position, glm::vec3 offset);
glm::mat4 renderView(glm::vec3 position, glm::quat orientation);


#endif 
