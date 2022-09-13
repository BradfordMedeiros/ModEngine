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

struct InitialDragRotation {
  objid id;
  glm::quat value;
};
struct ManipulatorUpdate {
  glm::vec3 manipulatorNew;
  glm::vec3 targetNew;
  bool shouldSet;
};


struct ManipulatorData {
  std::string state;

  objid manipulatorId;
  Axis manipulatorObject;
  std::optional<glm::vec3> initialDragPosition;
  std::vector<IdVec3Pair> initialDragPositions;
  std::vector<IdVec3Pair> initialDragScales;
  std::vector<InitialDragRotation> initialDragRotations;
};
ManipulatorData createManipulatorData();

objid getManipulatorId(ManipulatorData& manipulatorState);
void onManipulatorSelectItem(ManipulatorData& manipulatorState, objid selectedItem, std::string selectedItemName);
void onManipulatorMouseRelease(ManipulatorData& manipulatorState);

void onManipulatorUpdate(
  ManipulatorData& manipulatorState,
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