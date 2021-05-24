#ifndef MOD_EASYUSE
#define MOD_EASYUSE

#include <iostream>
#include <vector>
#include <functional>
#include "../common/util.h"
#include "../translations.h"

void setSnapAngleUp();
void setSnapAngleDown();
glm::quat snapAngleUp(SNAPPING_MODE mode, glm::quat currentAngle, Axis rotationAxis); 
glm::quat snapAngleDown(SNAPPING_MODE mode, glm::quat currentAngle, Axis rotationAxis);

void setSnapTranslateUp();
void setSnapTranslateDown();
glm::vec3 snapTranslateUp(SNAPPING_MODE mode, glm::vec3 currentPos, Axis translationAxis);
glm::vec3 snapTranslateDown(SNAPPING_MODE mode, glm::vec3 currentPos, Axis translationAxis);

void setSnapScaleUp();
void setSnapScaleDown();
glm::vec3 snapScaleUp(SNAPPING_MODE mode, glm::vec3 currentScale, Axis translationAxis);
glm::vec3 snapScaleDown(SNAPPING_MODE mode, glm::vec3 currentScale, Axis translationAxis);

float getSnapTranslateSize();

void snapCameraForward(std::function<void(glm::quat)> orientation);
void snapCameraBackward(std::function<void(glm::quat)> orientation);
void snapCameraUp(std::function<void(glm::quat)> orientation);
void snapCameraDown(std::function<void(glm::quat)> orientation);
void snapCameraLeft(std::function<void(glm::quat)> orientation);
void snapCameraRight(std::function<void(glm::quat)> orientation);

#endif