#ifndef MOD_TRANSLATIONS
#define MOD_TRANSLATIONS
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "./common/util.h"

glm::quat setFrontDelta(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta);
glm::vec3 calculateRelativeOffset(glm::quat orientation, glm::vec3 offset, bool xzPlaneOnly);
glm::vec3 moveRelative(glm::vec3 position, glm::quat orientation, glm::vec3 offset, bool xzPlaneOnly);
glm::mat4 renderView(glm::vec3 position, glm::quat orientation);

glm::vec3 applyTranslation(glm::vec3 position, float offsetX, float offsetY, Axis manipulatorAxis);
glm::vec3 applyScaling(glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, Axis manipulatorAxis);
glm::quat applyRotation(glm::quat currentOrientation, float offsetX, float offsetY, Axis manipulatorAxis);

float convertBase(float value, float fromBaseLow, float fromBaseHigh, float toBaseLow, float toBaseHigh);


RotationDirection getCursorInfoWorld(glm::mat4 projection, glm::mat4 view, float cursorLeft, float cursorBottom, float screenWidth, float screenHeight, float zDistance = 1.f);
RotationDirection getCursorInfoWorldNdi(glm::mat4 projection, glm::mat4 view, float screenXPosNdi, float screenYPosNdi, float zDistance);

glm::vec3 getCursorRayDirection(glm::mat4 projection, glm::mat4 view, float cursorLeft, float cursorBottom, float screenWidth, float screenHeight);
glm::vec3 projectCursorAtDepth(glm::mat4 projection, glm::mat4 view, float nearPlane, float farPlane, glm::vec2 cursorPos, glm::vec2 screensize, float depth);
bool calcLineIntersection(glm::vec3 ray1From, glm::vec3 ray1Dir, glm::vec3 ray2From, glm::vec3 ray2Dir, glm::vec3* _intersectPoint);

struct ProjectCursorDebugInfo {
  glm::vec3 positionFrom;
  glm::vec3 projectedTarget;
  glm::vec3 target;
  glm::vec3 intersectionPoint;
  glm::vec3 selectDir;
  glm::vec3 targetAxis;
};
glm::vec3 projectCursorPositionOntoAxis(glm::mat4 projection, glm::mat4 view, glm::vec2 cursorPos, glm::vec2 screensize, Axis manipulatorAxis, glm::vec3 lockvalues, ProjectCursorDebugInfo* _debugInfo);

glm::quat quatFromDirection(glm::vec3 direction);
glm::vec3 directionFromQuat(glm::quat direction);

const glm::quat MOD_ORIENTATION_UP = quatFromDirection(glm::vec3(0.f, 1.f, 0.f));
const glm::quat MOD_ORIENTATION_RIGHT = quatFromDirection(glm::vec3(1.f, 0.f, 0.f));
const glm::quat MOD_ORIENTATION_FORWARD = quatFromDirection(glm::vec3(0.f, 0.f, -1.f));
glm::quat axisToOrientation(Axis axis);

const float MODPI = 3.1416f;
float atanRadians360(float x, float y);

struct Transformation {
  glm::vec3 position;
  glm::vec3 scale;
  glm::quat rotation;
};

glm::mat4 matrixFromComponents(glm::mat4 initialModel, glm::vec3 position, glm::vec3 scale, glm::quat rotation);
glm::mat4 matrixFromComponents(Transformation transformation);
Transformation getTransformationFromMatrix(glm::mat4 matrix);

struct RotationPosition {
  glm::vec3 position;
  glm::quat rotation;
};

RotationPosition rotateOverAxis(RotationPosition object, RotationPosition axis, float rotationRadians, std::optional<std::function<glm::quat(glm::quat)>> snapRotate);
std::optional<glm::vec3> findPlaneIntersection(glm::vec3 anypointOnPlane, glm::vec3 planeNormal, glm::vec3 rayPosition, glm::vec3 rayDirection);

glm::vec3 calcOffsetFromRotation(glm::vec3 position, std::optional<glm::vec3> offset, glm::quat rotation);
glm::vec3 calcOffsetFromRotationReverse(glm::vec3 position, std::optional<glm::vec3> offset, glm::quat rotation);
std::string print(Transformation& transform);
bool testOnlyAboutEqual(Transformation& transform1, Transformation& transform2);

#endif 
