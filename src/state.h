#ifndef MOD_STATE
#define MOD_STATE

#include <string>
#include "./scene/scene.h"
#include "./common/util.h"

enum ManipulatorAxis { NOAXIS, XAXIS, YAXIS, ZAXIS };

struct engineState {
  bool visualizeNormals;
  bool showCameras;
  bool isSelectionMode;
  bool isRotateSelection;
  std::string selectedName;
  bool useDefaultCamera;
  bool moveRelativeEnabled;
  unsigned int mode;
  unsigned int axis; 
  unsigned int currentScreenWidth;
  unsigned int currentScreenHeight;
  unsigned int cursorLeft;
  unsigned int cursorTop;
  short selectedIndex;
  unsigned int activeCamera;
  std::string additionalText;

  bool enableManipulator;
  ManipulatorMode manipulatorMode;
  ManipulatorAxis manipulatorAxis;
  bool firstMouse;
  float lastX;
  float lastY;
  float offsetX;
  float offsetY;

};

engineState getDefaultState(unsigned int initialScreenWidth, unsigned int initialScreenHeight);

#endif