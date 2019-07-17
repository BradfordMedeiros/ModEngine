#ifndef MOD_CAMERA
#define MOD_CAMERA
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
private:
  glm::vec3 position, front, up;
  float speed;
  float pitch, yaw;
  void setFront(float yaw, float pitch);
public:
  Camera(glm::vec3 position, glm::vec3 up, float speed, float pitch, float yaw);
  glm::mat4 getView();
  void moveFront();
  void moveBack();
  void moveLeft();
  void moveRight();
  void setFrontDelta(float deltaYaw, float deltaPitch);
  glm::mat4 renderView();
};

#endif 
