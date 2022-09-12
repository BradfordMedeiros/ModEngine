#ifndef MOD_MANIPULATOR_DRAWING
#define MOD_MANIPULATOR_DRAWING

#include "../common/util.h"
#include "../scene/serialization.h"  

struct ManipulatorTools {
  std::function<glm::vec3(objid)> getPosition;
  std::function<void(objid, glm::vec3)> setPosition;
  std::function<glm::vec3(objid)> getScale;
  std::function<void(objid, glm::vec3)> setScale;
  std::function<glm::quat(objid)> getRotation;
  std::function<void(objid, glm::quat)> setRotation;
  std::function<glm::vec3(glm::vec3)> snapPosition;
  std::function<glm::vec3(glm::vec3)> snapScale;
  std::function<glm::quat(glm::quat, Axis)> snapRotate;
  std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine;
  std::function<void()> clearLines;
};
struct IdVec3Pair {
  objid id;
  glm::vec3 value;
};

void drawDirectionalLine(std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, glm::vec3 fromPos, glm::vec3 direction, LineColor color);
void drawHitMarker(std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, glm::vec3 position);
void drawRotation(std::vector<IdVec3Pair> positions, glm::vec3 meanPosition, glm::quat rotationOrientation, glm::vec3 cameraPosition, glm::vec3 selectDir, glm::vec3 intersection, float angle, std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine);

#endif