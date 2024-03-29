#ifndef MOD_EASYUSE
#define MOD_EASYUSE

#include <iostream>
#include <vector>
#include <functional>
#include "../common/util.h"
#include "../translations.h"

struct EasyUseInfo {
	float currentAngle;
	float currentTranslate;
	float currentScale;

	std::optional<glm::quat> orientation;
};

EasyUseInfo createEasyUse();

glm::quat snapAngleUp(EasyUseInfo& easyUse, SNAPPING_MODE mode, glm::quat currentAngle, Axis rotationAxis); 
glm::quat snapAngleDown(EasyUseInfo& easyUse, SNAPPING_MODE mode, glm::quat currentAngle, Axis rotationAxis);
glm::quat snapRotate(EasyUseInfo& easyUse, glm::quat newRotation, Axis snapAxis, float extraRadians);

glm::vec3 snapTranslateUp(EasyUseInfo& easyUse, SNAPPING_MODE mode, glm::vec3 currentPos, Axis translationAxis);
glm::vec3 snapTranslateDown(EasyUseInfo& easyUse, SNAPPING_MODE mode, glm::vec3 currentPos, Axis translationAxis);
glm::vec3 snapTranslate(EasyUseInfo& easyUse, glm::vec3 position);

glm::vec3 snapScaleUp(EasyUseInfo& easyUse, SNAPPING_MODE mode, glm::vec3 currentScale, Axis translationAxis);
glm::vec3 snapScaleDown(EasyUseInfo& easyUse, SNAPPING_MODE mode, glm::vec3 currentScale, Axis translationAxis);
glm::vec3 snapScale(EasyUseInfo& easyUse, glm::vec3 scale);

struct SnapCoordSystem {
	std::optional<glm::quat> orientation;
	float size;
};

SnapCoordSystem getSnapTranslateSize(EasyUseInfo& easyUse);

void setSnapEasyUseUp(EasyUseInfo& easyUse, ManipulatorMode manipulatorMode);
void setSnapEasyUseDown(EasyUseInfo& easyUse, ManipulatorMode manipulatorMode);

void setSnapCoordinateSystem(EasyUseInfo& easyUse, std::optional<glm::quat> orientation);

void snapCameraForward(std::function<void(glm::quat)> orientation);
void snapCameraBackward(std::function<void(glm::quat)> orientation);
void snapCameraUp(std::function<void(glm::quat)> orientation);
void snapCameraDown(std::function<void(glm::quat)> orientation);
void snapCameraLeft(std::function<void(glm::quat)> orientation);
void snapCameraRight(std::function<void(glm::quat)> orientation);

#endif