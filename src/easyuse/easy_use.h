#ifndef MOD_EASYUSE
#define MOD_EASYUSE

#include <iostream>
#include <vector>
#include <functional>
#include "../common/util.h"
#include "../translations.h"

glm::quat snapAngleUp(SNAPPING_MODE mode, glm::quat currentAngle, Axis rotationAxis); 
glm::quat snapAngleDown(SNAPPING_MODE mode, glm::quat currentAngle, Axis rotationAxis);

glm::vec3 snapTranslateUp(SNAPPING_MODE mode, glm::vec3 currentPos, Axis translationAxis);
glm::vec3 snapTranslateDown(SNAPPING_MODE mode, glm::vec3 currentPos, Axis translationAxis);

glm::vec3 snapScaleUp(SNAPPING_MODE mode, glm::vec3 currentScale, Axis translationAxis);
glm::vec3 snapScaleDown(SNAPPING_MODE mode, glm::vec3 currentScale, Axis translationAxis);

float getSnapTranslateSize();

void setSnapEasyUseUp(ManipulatorMode manipulatorMode);
void setSnapEasyUseDown(ManipulatorMode manipulatorMode);

void snapCameraForward(std::function<void(glm::quat)> orientation);
void snapCameraBackward(std::function<void(glm::quat)> orientation);
void snapCameraUp(std::function<void(glm::quat)> orientation);
void snapCameraDown(std::function<void(glm::quat)> orientation);
void snapCameraLeft(std::function<void(glm::quat)> orientation);
void snapCameraRight(std::function<void(glm::quat)> orientation);

#endif