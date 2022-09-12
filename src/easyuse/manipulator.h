#ifndef MOD_MANIPULATOR
#define MOD_MANIPULATOR

#include <functional>
#include <optional>
#include "../translations.h"
#include "./manipulator_drawing.h"

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
  glm::mat4 projection,
  glm::mat4 cameraViewMatrix, 
  ManipulatorMode mode,
  Axis defaultAxis,
  float mouseX, 
  float mouseY,
  glm::vec2 cursorPos,
  glm::vec2 screensize,
  ManipulatorOptions options,
  ManipulatorTools tools
);

#endif