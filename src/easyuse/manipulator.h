#ifndef MOD_MANIPULATOR
#define MOD_MANIPULATOR

#include <functional>
#include <optional>
#include "../translations.h"
#include "./manipulator_drawing.h"

struct InitialDragRotation {
  objid id;
  glm::quat value;
};
struct ManipulatorUpdate {
  glm::vec3 manipulatorNew;
  glm::vec3 targetNew;
  bool shouldSet;
};

struct InitialTransformData {
  objid id;
  Transformation transform;
};

struct ManipulatorData {
  std::string state;
  bool mouseClickedLastFrame;
  bool mouseReleasedLastFrame;
  bool selectedItemLastFrame;

  objid manipulatorId;
  Axis manipulatorObject;

  std::optional<glm::vec3> initialDragPosition;
  std::vector<InitialTransformData> initialTransforms;
  float rotationAmount;
  std::optional<glm::vec3> meanPosition;
};


enum SCALING_GROUP { INDIVIDUAL_SCALING, GROUP_SCALING };
struct ManipulatorOptions {
  SNAPPING_MODE manipulatorPositionMode;
  SNAPPING_MODE rotateMode;
  SCALING_GROUP scalingGroup;
  bool snapManipulatorScales;
  bool preserveRelativeScale;
};
struct ManipulatorUpdateInfo {
  glm::mat4 projection;
  glm::mat4 cameraViewMatrix; 
  ManipulatorMode mode;
  Axis defaultAxis;
  float mouseX; 
  float mouseY;
  glm::vec2 cursorPos;
  glm::vec2 screensize;
  ManipulatorOptions options;
  ManipulatorSelection selectedObjs;
};

ManipulatorData createManipulatorData();

objid getManipulatorId(ManipulatorData& manipulatorState);
void onManipulatorSelectItem(ManipulatorData& manipulatorState, objid selectedItem, std::string selectedItemName);
void onManipulatorMouseDown(ManipulatorData& manipulatorState);
void onManipulatorMouseRelease(ManipulatorData& manipulatorState);

enum MANIPULATOR_EVENT { OBJECT_ORIENT_UP, OBJECT_ORIENT_DOWN, OBJECT_ORIENT_RIGHT, OBJECT_ORIENT_LEFT, OBJECT_ORIENT_FORWARD, OBJECT_ORIENT_BACK };
void onManipulatorEvent(ManipulatorData& manipulatorState, ManipulatorTools& tools, MANIPULATOR_EVENT event);

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