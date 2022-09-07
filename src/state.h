#ifndef MOD_STATE
#define MOD_STATE

#include <string>
#include <functional>
#include "./common/util.h"
#include "./easyuse/editor.h"
#include "./easyuse/easy_use.h"
#include "./scene/scene.h"

enum RENDER_MODE { RENDER_FINAL, RENDER_PORTAL, RENDER_PAINT, RENDER_DEPTH, RENDER_BLOOM, RENDER_GRAPHS };
std::string renderModeAsStr(RENDER_MODE mode);

struct CamInterpolation {
  bool shouldInterpolate;
  float startingTime;
  float length;
  objid targetCam;
};

enum ANTIALIASING_TYPE { ANTIALIASING_NONE, ANTIALIASING_MSAA };

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
  CamInterpolation cameraInterp;

  std::string additionalText;
 
  bool enableManipulator;
  ManipulatorMode manipulatorMode;
  Axis manipulatorAxis;
  objid manipulatorLineId;
  bool snapManipulatorPositions;
  bool snapManipulatorScales;
  bool snapManipulatorAngles;
  bool rotateSnapRelative;
  bool preserveRelativeScale;

  bool firstMouse;
  float lastX;
  float lastY;
  float offsetX;
  float offsetY;
  bool mouseIsDown;
  bool captureCursor;

  bool enableDiffuse;
  bool enableSpecular;
  bool enablePBR;
  bool showBoneWeight;
  bool useBoneTransform;
  int textureIndex;
  bool shouldPaint;
  bool shouldTerrainPaint;
  bool terrainPaintDown;
  
  bool enableBloom;
  float bloomAmount;
  int bloomBlurAmount;
  bool enableFog;
  glm::vec4 fogColor;
  glm::vec3 ambient;
  
  float exposureStart;
  float oldExposure;
  float targetExposure;
  float exposure;


  bool takeScreenshot;
  std::string screenshotPath;
  
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
  bool pauseWorldTiming;
  GameObject* activeCameraObj;
  GameObjectCamera* activeCameraData;

  std::string skybox;
  glm::vec3 skyboxcolor;
  bool showSkybox;

  bool enableDof;

  // Rendering options 
  int swapInterval;
  bool fullscreen;
  bool nativeViewport;
  bool nativeResolution;
  glm::ivec2 viewportSize;
  glm::ivec2 viewportoffset;
  glm::ivec2 resolution;
  std::string borderTexture;
  ANTIALIASING_TYPE antialiasingMode;

  std::string crosshair;
  std::string windowname;
  std::string iconpath;

  int fontsize;

  bool showGrid;
  int gridSize;
  EasyUseInfo easyUse;
};

engineState getDefaultState(unsigned int initialScreenWidth, unsigned int initialScreenHeight);
void setState(engineState& state, std::vector<ObjectValue>& values, float now);
void setInitialState(engineState& state, std::string file, float now, std::function<std::string(std::string)> readFile);

#endif