#ifndef MOD_STATE
#define MOD_STATE

#include <string>
#include "./scene/scene_sandbox.h"
#include "./common/util.h"
#include "./easyuse/editor.h"

enum RENDER_MODE { RENDER_FINAL, RENDER_PORTAL, RENDER_PAINT, RENDER_DEPTH };

struct engineState {
  bool visualizeNormals;
  bool showCameras;
  bool isRotateSelection;
  std::string selectedName;
  bool useDefaultCamera;
  bool moveRelativeEnabled;
  unsigned int currentScreenWidth;
  unsigned int currentScreenHeight;
  int cursorLeft;
  int cursorTop;
  int32_t currentHoverIndex;
  int32_t lastHoverIndex;
  bool hoveredIdInScene;
  bool lastHoveredIdInScene;
  unsigned int activeCamera;
  std::string additionalText;
 
  bool enableManipulator;
  ManipulatorMode manipulatorMode;
  Axis manipulatorAxis;

  bool firstMouse;
  float lastX;
  float lastY;
  float offsetX;
  float offsetY;
  bool mouseIsDown;
  bool enableDiffuse;
  bool enableSpecular;
  float fov;  // in degrees
  bool toggleFov;
  bool showBoneWeight;
  bool useBoneTransform;
  int textureIndex;
  bool shouldPaint;
  bool shouldTerrainPaint;
  bool terrainPaintDown;
  bool enableBloom;
  float bloomAmount;
  bool takeScreenshot;
  bool highlight;
  bool multiselect;
  EditorContent editor;
  bool isRecording;
  objid recordingIndex;
  RENDER_MODE renderMode;
  SNAPPING_MODE snappingMode;
  bool drawPoints;
  bool moveUp;
  bool cameraFast;

  int depthBufferLayer;
  bool printKeyStrokes;
  bool cullEnabled;

  bool groupSelection;
};

engineState getDefaultState(unsigned int initialScreenWidth, unsigned int initialScreenHeight);

#endif