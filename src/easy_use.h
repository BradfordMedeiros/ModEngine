#ifndef MOD_EASYUSE
#define MOD_EASYUSE

#include <iostream>
#include <vector>
#include <functional>
#include "./common/util.h"

void setSnapAngleUp();
void setSnapAngleDown();
glm::quat snapAngleUp(glm::quat currentAngle, Axis rotationAxis); 
glm::quat snapAngleDown(glm::quat currentAngle, Axis rotationAxis);

#endif