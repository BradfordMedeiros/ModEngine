#ifndef MOD_STATE
#define MOD_STATE

#include <string>
#include <functional>
#include "./common/util.h"
#include "./easyuse/editor.h"
#include "./scene/scene.h"

enum RENDER_MODE { RENDER_FINAL, RENDER_PORTAL, RENDER_PAINT, RENDER_DEPTH, RENDER_BLOOM };

struct CamInterpolation {
  bool shouldInterpolate;
  float startingTime;
  float length;
  objid targetCam;
};

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

  bool firstMouse;
  float lastX;
  float lastY;
  float offsetX;
  float offsetY;
  bool mouseIsDown;
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
  
  float exposureStart;
  float oldExposure;
  float targetExposure;
  float exposure;


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
  bool pauseWorldTiming;
  GameObject* activeCameraObj;
  GameObjectCamera* activeCameraData;

  std::string skybox;
  glm::vec3 skyboxcolor;

  bool enableDof;
};

engineState getDefaultState(unsigned int initialScreenWidth, unsigned int initialScreenHeight);
void setState(engineState& state, std::vector<ObjectValue>& values, float now);
void setInitialState(engineState& state, std::string file, float now);

#endif