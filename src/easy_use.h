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

void setSnapTranslateUp();
void setSnapTranslateDown();
glm::vec3 snapTranslateUp(glm::vec3 currentPos, Axis translationAxis);
glm::vec3 snapTranslateDown(glm::vec3 currentPos, Axis translationAxis);

void setSnapScaleUp();
void setSnapScaleDown();
glm::vec3 snapScaleUp(glm::vec3 currentScale, Axis translationAxis);
glm::vec3 snapScaleDown(glm::vec3 currentScale, Axis translationAxis);

float getSnapTranslateSize();

#endif