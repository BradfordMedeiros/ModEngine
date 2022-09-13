#ifndef MOD_MANIPULATOR_DRAWING
#define MOD_MANIPULATOR_DRAWING

#include "../common/util.h"
#include "../scene/serialization.h"  

struct ManipulatorSelection {
  std::optional<objid> mainObj;
  std::vector<objid> selectedIds;
};  

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
  std::function<void(objid)> removeObjectById; 
  std::function<objid(void)> makeManipulator;
  std::function<ManipulatorSelection()> getSelectedIds;
};

struct IdVec3Pair {
  objid id;
  glm::vec3 value;
};

void drawDirectionalLine(std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, glm::vec3 fromPos, glm::vec3 direction, LineColor color);
void drawHitMarker(std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, glm::vec3 position);
void drawRotation(std::vector<IdVec3Pair> positions, glm::vec3 meanPosition, glm::quat rotationOrientation, glm::vec3 cameraPosition, glm::vec3 selectDir, glm::vec3 intersection, float angle, std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine);
void drawProjectionVisualization(std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, ProjectCursorDebugInfo& projectCursorInfo);

glm::vec3 calcMeanPosition(std::vector<IdVec3Pair> positions);
std::vector<IdVec3Pair> idToPositions(std::vector<objid>& targets, std::function<glm::vec3(objid)> getValue);

//  2 - 2 = 0 units, so 1x original scale
//  3 - 2 = 1 units, so 2x original scale
//  4 - 2 = 2 units, so 3x original scale 
// for negative
// (-2) - (-2) = 0 units, so 1x original scale
// (-3) - (-2) = -1 units, so 2x original scale
glm::vec3 calcPositionDiff(glm::vec3 effectInitialPos, glm::vec3 projectedPosition, glm::vec3 manipulatorPosition, bool reverseOnMiddle);

template <typename T, typename V>
T findDragValue(std::vector<V> values, objid id){
  for (auto &value : values){
    if (value.id == id){
      return value.value;
    }
  }
  modassert(false, std::string("could not find manipulatorTarget = " + std::to_string(id)));
  T defaultValue;
  return defaultValue;
}

#endif