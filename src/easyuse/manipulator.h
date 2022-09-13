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
struct InitialDragRotation {
  objid id;
  glm::quat value;
};
struct ManipulatorUpdate {
  glm::vec3 manipulatorNew;
  glm::vec3 targetNew;
  bool shouldSet;
};


struct ManipulatorState {
  objid manipulatorId;
  Axis manipulatorObject;
  std::optional<glm::vec3> initialDragPosition;
  std::vector<IdVec3Pair> initialDragPositions;
  std::vector<IdVec3Pair> initialDragScales;
  std::vector<InitialDragRotation> initialDragRotations;
};
ManipulatorState createManipulatorState();

objid getManipulatorId(ManipulatorState& manipulatorState);
void onManipulatorSelectItem(ManipulatorState& manipulatorState, objid selectedItem, std::string selectedItemName);
void onManipulatorMouseRelease(ManipulatorState& manipulatorState);

void onManipulatorUpdate(
  ManipulatorState& manipulatorState,
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