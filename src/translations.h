#ifndef MOD_CAMERA
#define MOD_CAMERA
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "./common/util.h"

glm::quat setFrontDelta(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta);
glm::vec3 moveRelative(glm::vec3 position, glm::quat orientation, glm::vec3 offset);
glm::vec3 move(glm::vec3 position, glm::vec3 offset);
glm::mat4 renderView(glm::vec3 position, glm::quat orientation);

glm::vec3 applyTranslation(glm::vec3 position, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis);
glm::vec3 applyScaling(glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis);
glm::quat applyRotation(glm::quat currentOrientation, float offsetX, float offsetY);

#endif 
