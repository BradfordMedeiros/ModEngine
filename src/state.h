#ifndef MOD_STATE
#define MOD_STATE

#include <string>
#include <functional>
#include "./common/util.h"
#include "./easyuse/editor.h"
#include "./easyuse/easy_use.h"
#include "./easyuse/manipulator.h"
#include "./scene/scene.h"

enum RENDER_MODE { RENDER_FINAL, RENDER_PORTAL, RENDER_PAINT, RENDER_DEPTH, RENDER_BLOOM, RENDER_GRAPHS, RENDER_SELECTION};
std::string renderModeAsStr(RENDER_MODE mode);

struct CamInterpolation {
  bool shouldInterpolate;
  float startingTime;
  float length;
  objid targetCam;
};

enum ANTIALIASING_TYPE { ANTIALIASING_NONE, ANTIALIASING_MSAA };
enum CURSOR_TYPE { CURSOR_NORMAL, CURSOR_CAPTURE, CURSOR_HIDDEN };

struct engineState {
  bool visualizeNormals;
  bool showDebug;
  int showDebugMask;
  bool isRotateSelection;
  bool selectionDisabled;
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
  glm::vec3 hoveredItemColor;

  unsigned int activeCamera;
  CamInterpolation cameraInterp;

  std::string additionalText;
 
  ManipulatorData manipulatorState;
  ManipulatorMode manipulatorMode;
  Axis manipulatorAxis;
  objid manipulatorLineId;
  SNAPPING_MODE manipulatorPositionMode;
  bool relativePositionMode;
  bool translateMirror;
  SNAPPING_MODE rotateMode;
  SCALING_GROUP scalingGroup;
  bool snapManipulatorScales;
  bool preserveRelativeScale;

  bool firstMouse;
  float lastX;
  float lastY;
  float offsetX;
  float offsetY;
  bool mouseIsDown;
  CURSOR_TYPE cursorBehavior;

  bool enableDiffuse;
  bool enableSpecular;
  bool enablePBR;
  bool enableAttenuation;
  bool enableShadows;
  float shadowIntensity;
  bool showBoneWeight;
  bool useBoneTransform;
  int textureIndex;
  bool shouldPaint;
  bool shouldTerrainPaint;
  float terrainPaintAmount;
  float terrainPaintRadius;
  bool terrainSmoothing;
  std::string terrainPaintBrush;

  bool enableBloom;
  float bloomThreshold;
  float bloomAmount;
  int bloomBlurAmount;
  bool enableFog;
  float fogMinCutoff;
  float fogMaxCutoff;
  glm::vec4 fogColor;
  glm::vec3 ambient;
  
  float exposureStart;
  float oldExposure;
  float targetExposure;
  float exposure;
  bool enableExposure;
  bool enableGammaCorrection;

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
  glm::ivec4 savedWindowsize;
  std::string borderTexture;
  ANTIALIASING_TYPE antialiasingMode;

  std::string crosshair;
  std::string windowname;
  std::string iconpath;

  int fontsize;

  bool showGrid;
  int gridSize;
  EasyUseInfo easyUse;

  bool worldpaused;

  glm::vec2 infoTextOffset;

  bool useYAxis;

  objid forceSelectIndex;

  float volume;
  bool muteSound;
  
  bool disableInput;
  bool escapeQuits;
};

engineState getDefaultState(unsigned int initialScreenWidth, unsigned int initialScreenHeight);
void setState(engineState& state, std::vector<ObjectValue>& values, float now);
void setInitialState(engineState& state, std::string file, float now, std::function<std::string(std::string)> readFile, bool disableInput);
std::vector<ObjectValue> getState(engineState& state);

#endif