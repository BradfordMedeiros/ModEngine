#ifndef MOD_WORLDTASKS
#define MOD_WORLDTASKS

#include <glm/glm.hpp>
#include "./main_api.h"  // change this for the scene layer instead


struct RequestMovingObject {
  glm::vec3 initialPos;
  glm::vec3 finalPos;
  float initialTime;
  float duration;
};

void moveCameraTo(objid cameraId, glm::vec3 position, std::optional<float> duration);
void handleMovingObjects(float currTime);


#endif