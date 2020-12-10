#ifndef MOD_STATE
#define MOD_STATE

#include <string>
#include "./scene/scenegraph.h"
#include "./common/util.h"
#include "./editor.h"

struct engineState {
  bool visualizeNormals;
  bool showCameras;
  bool isRotateSelection;
  std::string selectedName;
  bool useDefaultCamera;
  bool moveRelativeEnabled;
  unsigned int mode;
  unsigned int axis; 
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
  bool showDepthBuffer;
  float fov;  // in degrees
  bool toggleFov;
  bool showBoneWeight;
  bool useBoneTransform;
  float discardAmount;
  bool offsetTextureMode;
  int portalTextureIndex;
  bool textureDisplayMode;
  bool shouldPaint;
  bool shouldTerrainPaint;
  bool terrainPaintDown;
  bool enableBloom;
  float bloomAmount;
  bool takeScreenshot;
  bool highlight;
  bool shouldSelect;
  EditorContent editor;
  bool multiselectMode;
  bool isRecording;
};

engineState getDefaultState(unsigned int initialScreenWidth, unsigned int initialScreenHeight);

#endif