#ifndef MOD_TRANSLATIONS
#define MOD_TRANSLATIONS
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "./common/util.h"

glm::quat setFrontDelta(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta);
glm::vec3 calculateRelativeOffset(glm::quat orientation, glm::vec3 offset, bool xzPlaneOnly);
glm::vec3 moveRelative(glm::vec3 position, glm::quat orientation, glm::vec3 offset, bool xzPlaneOnly);
glm::mat4 renderView(glm::vec3 position, glm::quat orientation);

glm::vec3 applyTranslation(glm::vec3 position, float offsetX, float offsetY, Axis manipulatorAxis);
glm::vec3 applyScaling(glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, Axis manipulatorAxis);
glm::quat applyRotation(glm::quat currentOrientation, float offsetX, float offsetY, Axis manipulatorAxis);

float convertBase(float value, float fromBaseLow, float fromBaseHigh, float toBaseLow, float toBaseHigh);

glm::vec3 getCursorRayDirection(glm::mat4 projection, glm::mat4 view, float cursorLeft, float cursorTop, float screenWidth, float screenHeight);
glm::vec3 projectCursorAtDepth(glm::mat4 projection, glm::mat4 view, float nearPlane, float farPlane, glm::vec2 cursorPos, glm::vec2 screensize, float depth);
bool calcLineIntersection(glm::vec3 ray1From, glm::vec3 ray1Dir, glm::vec3 ray2From, glm::vec3 ray2Dir, glm::vec3* _intersectPoint);
glm::vec3 projectCursorPositionOntoAxis(glm::mat4 projection, glm::mat4 view, glm::vec2 cursorPos, glm::vec2 screensize, Axis manipulatorAxis, glm::vec3 lockvalues);

glm::quat quatFromDirection(glm::vec3 direction);
glm::vec3 directionFromQuat(glm::quat direction);


#endif 
