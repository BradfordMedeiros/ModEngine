#ifndef MOD_MANIPULATOR
#define MOD_MANIPULATOR

#include <functional>
#include <optional>
#include "../common/util.h"
#include "../scene/serialization.h"  
#include "../translations.h"

struct ManipulatorOptions {
  bool snapManipulatorPositions;
  bool snapManipulatorScales;
  bool snapManipulatorAngles;
  bool rotateSnapRelative;
  bool preserveRelativeScale;
};
struct ManipulatorSelection {
  std::optional<objid> mainObj;
  std::vector<objid> selectedIds;
};  

objid getManipulatorId();
void onManipulatorSelectItem(objid selectedItem, std::string selectedItemName);
void onManipulatorMouseRelease();

void onManipulatorUpdate(
  std::function<ManipulatorSelection()> getSelectedIds,
  std::function<objid(void)> makeManipulator,
  std::function<void(objid)> removeObjectById,
  std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine,
  std::function<void()> clearLines,
  std::function<glm::vec3(objid)> getPosition, 
  std::function<void(objid, glm::vec3)> setPosition, 
  std::function<glm::vec3(objid)> getScale,
  std::function<void(objid, glm::vec3)> setScale,
  std::function<glm::quat(objid)> getRotation,
  std::function<void(objid, glm::quat)> setRotation,
  glm::mat4 projection,
  glm::mat4 cameraViewMatrix, 
  ManipulatorMode mode,
  float mouseX, 
  float mouseY,
  glm::vec2 cursorPos,
  glm::vec2 screensize,
  std::function<glm::vec3(glm::vec3)> snapPosition,
  std::function<glm::vec3(glm::vec3)> snapScale,
  std::function<glm::quat(glm::quat, Axis)> snapRotate,
  ManipulatorOptions options
);

#endif