#include "./state.h"

extern World world;

std::string renderModeAsStr(RENDER_MODE mode){
  if (mode == RENDER_FINAL){
    return "final";
  }
  if (mode == RENDER_PORTAL){
    return "portal";
  }
  if (mode == RENDER_PAINT){
    return "paint";
  }
  if (mode == RENDER_DEPTH){
    return "depth";
  }
  if (mode == RENDER_BLOOM){
    return "bloom";
  }
  if (mode == RENDER_GRAPHS){
    return "graphs";
  }
  modassert(false, "invalid render mode");
  return "";
}

struct ObjectStateMapping {
  std::function<void(engineState& state, AttributeValue, float)> attr;
  std::string object;
  std::string attribute;
};

std::vector<ObjectStateMapping> mapping = {
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto enabled = std::get_if<std::string>(&value);
      if (enabled != NULL){
        state.enableDiffuse = *enabled == "true";
      }
    },
    .object = "diffuse",
    .attribute = "enabled",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto enabled = std::get_if<std::string>(&value);
      if (enabled != NULL){
        state.enableSpecular = *enabled == "true";
      }
    },
    .object = "specular",
    .attribute = "enabled",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto enabled = std::get_if<std::string>(&value);
      if (enabled != NULL){
        state.enablePBR = *enabled == "true";
      }
    },
    .object = "pbr",
    .attribute = "enabled",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto color = std::get_if<glm::vec3>(&value);
      if (color != NULL){
        state.ambient = *color; 
      }
    },
    .object = "light",
    .attribute = "amount",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto color = std::get_if<glm::vec3>(&value);
      if (color != NULL){
        auto fogColor = *color;
        state.fogColor = glm::vec4(fogColor.x, fogColor.y, fogColor.z, 1.0f); 
      }
    },
    .object = "fog",
    .attribute = "color",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto enabled = std::get_if<std::string>(&value);
      if (enabled != NULL){
        state.enableFog = *enabled == "true";
      }
    },
    .object = "fog",
    .attribute = "enabled",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto enabled = std::get_if<std::string>(&value);
      if (enabled != NULL){
        state.enableBloom = *enabled == "true";
      }
    },
    .object = "bloom",
    .attribute = "enabled",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto amount = std::get_if<float>(&value);
      if (amount != NULL){
        state.bloomAmount = *amount;
      }
    },
    .object = "bloom",
    .attribute = "amount",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto amount = std::get_if<float>(&value);
      if (amount != NULL){
        state.bloomBlurAmount = static_cast<int>(*amount);
      }
    },
    .object = "bloomblur",
    .attribute = "amount",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto amount = std::get_if<float>(&value);
      if (amount != NULL){
        state.exposureStart = now;
        state.targetExposure = *amount;
        state.oldExposure = state.exposure;
        std::cout << "target exposure: " << state.targetExposure << " but the old exposure: " << state.oldExposure << std::endl;
      }
    },
    .object = "exposure",
    .attribute = "amount",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto skyboxTexture = std::get_if<std::string>(&value);
      if (skyboxTexture != NULL){
        std::cout << "state: update skybox: " << *skyboxTexture << std::endl;
        state.skybox = *skyboxTexture;
      }     
    },
    .object = "skybox",
    .attribute = "texture",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto skyboxColor = std::get_if<glm::vec3>(&value);
      if (skyboxColor != NULL){
        std::cout << "state: update skybox color: " << print(*skyboxColor) << std::endl;
        state.skyboxcolor = *skyboxColor; 
      }
    },
    .object = "skybox",
    .attribute = "color",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto showSkybox = std::get_if<std::string>(&value);
      if (showSkybox != NULL){
        std::cout << "state: update showSkybox: " << state.showSkybox << std::endl;
        state.showSkybox = *showSkybox == "true"; 
      }
    },
    .object = "skybox",
    .attribute = "enable",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto dovEnabled = std::get_if<std::string>(&value);
      if (dovEnabled != NULL){
        std::cout << "state: update dof: " << *dovEnabled << std::endl;
        state.enableDof = *dovEnabled == "enabled";
      }     
    },
    .object = "dof",
    .attribute = "state",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto swapInterval = std::get_if<float>(&value);
      if (swapInterval != NULL){
        int value = static_cast<int>(*swapInterval);
        std::cout << "state: swap interval: " << value << std::endl;
        state.swapInterval = value;
      }     
    },
    .object = "rendering",
    .attribute = "swapinterval",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto viewportSizeStr = std::get_if<std::string>(&value);
      if (viewportSizeStr != NULL){
        state.nativeViewport = false;
        state.viewportSize = parseVec2(*viewportSizeStr);
        std::cout << "viewport size: " << print(state.viewportSize) << std::endl;
      }     
    },
    .object = "rendering",
    .attribute = "viewport",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto viewportSizeStr = std::get_if<std::string>(&value);
      if (viewportSizeStr != NULL){
        state.viewportoffset = parseVec2(*viewportSizeStr);
        std::cout << "viewport offsetset: " << print(state.viewportoffset) << std::endl;
      }     
    },
    .object = "rendering",
    .attribute = "viewportoffset",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto resolutionStr = std::get_if<std::string>(&value);
      if (resolutionStr != NULL){
        state.nativeResolution = false;
        state.resolution = parseVec2(*resolutionStr);
        std::cout << "resolution: " << print(state.resolution) << std::endl;
      }     
    },
    .object = "rendering",
    .attribute = "resolution",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto borderTexture = std::get_if<std::string>(&value);
      if (borderTexture != NULL){
        state.borderTexture = *borderTexture;
        std::cout << "border texture: " << state.borderTexture << std::endl;
      }     
    },
    .object = "rendering",
    .attribute = "border",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto fullscreen = std::get_if<std::string>(&value);
      if (fullscreen != NULL){
        auto valid = maybeParseBool(*fullscreen, &state.fullscreen);
        assert(valid);
        std::cout << "fullscreen: " << (state.fullscreen ? "true" : "false") << std::endl;
      }     
    },
    .object = "rendering",
    .attribute = "fullscreen",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto antialiasing = std::get_if<std::string>(&value);
      if (antialiasing != NULL){
        auto isNone = *antialiasing == "none";
        auto isMsaa = *antialiasing == "msaa";
        if (!isNone && !isMsaa){
          std::cout << "invalid anti-aliasing mode: " << *antialiasing << std::endl;
          assert(false);
        }
        if (isNone){
          state.antialiasingMode = ANTIALIASING_NONE;
        }else if (isMsaa){
          state.antialiasingMode = ANTIALIASING_MSAA;
        }
      }     
    },
    .object = "rendering",
    .attribute = "antialiasing",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto cullEnabled = std::get_if<std::string>(&value);
      if (cullEnabled != NULL){
        state.cullEnabled = *cullEnabled != "disabled";
      }
      modassert(*cullEnabled == "disabled" || *cullEnabled == "enabled", "invalid cullEnable string");
    },
    .object = "rendering",
    .attribute = "cull",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto cursorBehavior = std::get_if<std::string>(&value);
      if (cursorBehavior != NULL){
        if (*cursorBehavior == "normal"){
          state.cursorBehavior = CURSOR_NORMAL;
          return;
        }else if (*cursorBehavior == "hidden"){
          state.cursorBehavior = CURSOR_HIDDEN;
          return;
        }else if (*cursorBehavior == "capture"){
          state.cursorBehavior = CURSOR_CAPTURE;
          return;
        }
        modassert(false, "invalid cursor type");
      }
    },
    .object = "mouse",
    .attribute = "cursor",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto crosshair = std::get_if<std::string>(&value);
      if (crosshair != NULL){
        state.crosshair = *crosshair;
        return;
      }
      assert(false);
    },
    .object = "mouse",
    .attribute = "crosshair",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto mode = std::get_if<std::string>(&value);
      if (mode == NULL){
        modassert(false, "invalid manipulator mode type");
        return;
      }
      if (*mode == "translate"){
        state.manipulatorMode = TRANSLATE;
      }else if (*mode == "scale"){
        state.manipulatorMode = SCALE;
      }else if (*mode == "rotate"){
        state.manipulatorMode = ROTATE;
      }else{
        modassert(false, "invalid manipulator mode option: " + *mode);
      }
    },
    .object = "tools",
    .attribute = "manipulator-mode",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto axis = std::get_if<std::string>(&value);
      if (axis == NULL){
        modassert(false, "invalid manipulator axis type");
        return;
      }
      if (*axis == "x"){
        state.manipulatorAxis = XAXIS;
      }else if (*axis == "y"){
        state.manipulatorAxis = YAXIS;
      }else if (*axis == "z"){
        state.manipulatorAxis = ZAXIS;
      }else if (*axis == "none"){
        state.manipulatorAxis = NOAXIS;
      }else{
        modassert(false, "invalid manipulator axis option: " + *axis);
      }
    },
    .object = "tools",
    .attribute = "manipulator-axis",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto preserveRelativeScale = std::get_if<std::string>(&value);
      if (preserveRelativeScale != NULL){
        state.preserveRelativeScale = *preserveRelativeScale == "true";
        return;
      }
      modassert(false, "invalid tools preserve-scale option: " + *preserveRelativeScale);
    },
    .object = "tools",
    .attribute = "preserve-scale",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto scaleGroup = std::get_if<std::string>(&value);
      if (scaleGroup != NULL){
        if (*scaleGroup == "individual"){
          state.scalingGroup = INDIVIDUAL_SCALING;
          return;
        }else if (*scaleGroup == "group"){
          state.scalingGroup = GROUP_SCALING;
          return;
        }
        modassert(false, "invalid tools scale-group option: " + *scaleGroup);
        return;
      }
    },
    .object = "tools",
    .attribute = "scale-group",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto snapManipulatorPositions = std::get_if<std::string>(&value);
      if (snapManipulatorPositions != NULL){
        if (*snapManipulatorPositions == "none"){
          state.manipulatorPositionMode = SNAP_CONTINUOUS;
        }else if (*snapManipulatorPositions == "relative"){
          state.manipulatorPositionMode = SNAP_RELATIVE;
        }else if (*snapManipulatorPositions == "absolute"){
          state.manipulatorPositionMode = SNAP_ABSOLUTE;
        }
        return;
      }
      modassert(false, "invalid tools snap-position option: " + *snapManipulatorPositions);
    },
    .object = "tools",
    .attribute = "snap-position",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto positionMirror = std::get_if<std::string>(&value);
      if (*positionMirror == "true"){
        state.translateMirror = true;
      }else if (*positionMirror == "false"){
        state.translateMirror = false;
      }
      modassert(*positionMirror == "true" || *positionMirror == "false", "invalid tools position-mirror: " + *positionMirror);
    },
    .object = "tools",
    .attribute = "position-mirror",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto snapManipulatorScales = std::get_if<std::string>(&value);
      if (snapManipulatorScales != NULL){
        state.snapManipulatorScales = *snapManipulatorScales == "true";
        return;
      }
      modassert(false, "invalid tools snap-scale option: " + *snapManipulatorScales);
    },
    .object = "tools",
    .attribute = "snap-scale",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto snapManipulatorAngles = std::get_if<std::string>(&value);
      if (*snapManipulatorAngles == "none"){
        state.rotateMode = SNAP_CONTINUOUS;
      }else if (*snapManipulatorAngles == "relative"){
        state.rotateMode = SNAP_RELATIVE;
      }else if (*snapManipulatorAngles == "absolute"){
        state.rotateMode = SNAP_ABSOLUTE;
      }
      return;
      modassert(false, "invalid tools snap-angles option: " + *snapManipulatorAngles);
    },
    .object = "tools",
    .attribute = "snap-rotate",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto windowname = std::get_if<std::string>(&value);
      if (windowname != NULL){
        state.windowname = *windowname;
        return;
      }
      assert(false);
    },
    .object = "window",
    .attribute = "name",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto iconpath = std::get_if<std::string>(&value);
      if (iconpath != NULL){
        state.iconpath = *iconpath;
        return;
      }
      assert(false);
    },
    .object = "window",
    .attribute = "icon",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto fontsize = std::get_if<float>(&value);
      if (fontsize != NULL){
        state.fontsize = *fontsize;
        return;
      }
      assert(false);
    },
    .object = "rendering",
    .attribute = "fontsize",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto showGrid = std::get_if<std::string>(&value);
      if (showGrid != NULL){
        state.showGrid = *showGrid == "true";
        return;
      }
      assert(false);
    },
    .object = "editor",
    .attribute = "showgrid",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto gridsize = std::get_if<float>(&value);
      if (gridsize != NULL){
        state.gridSize = *gridsize;
        return;
      }
      assert(false);
    },
    .object = "editor",
    .attribute = "gridsize",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto index = std::get_if<float>(&value);
      if (index != NULL){
        state.easyUse.currentAngleIndex = *index;
        return;
      }
      assert(false);
    },
    .object = "editor",
    .attribute = "snapangle-index",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto index = std::get_if<float>(&value);
      if (index != NULL){
        state.easyUse.currentTranslateIndex = *index;
        return;
      }
      assert(false);
    },
    .object = "editor",
    .attribute = "snaptranslate-index",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto index = std::get_if<float>(&value);
      if (index != NULL){
        state.easyUse.currentScaleIndex = *index;
        return;
      }
      assert(false);
    },
    .object = "editor",
    .attribute = "snapscale-index",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto selectedIndexStr = std::get_if<std::string>(&value);
      if (selectedIndexStr != NULL){
        int index =  std::atoi(selectedIndexStr -> c_str());
        std::cout << "selected index: " << index << std::endl;
        state.editor.forceSelectIndex = index;
      }

    },
    .object = "editor",
    .attribute = "selected-index",
  },

  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto worldpaused = std::get_if<std::string>(&value);
      if (worldpaused != NULL){
        state.worldpaused = *worldpaused == "true";
      }
    },
    .object = "world",
    .attribute = "paused",
  },
};  

void setState(engineState& state, ObjectValue& value, float now){
  for (auto &stateMap : mapping){
    //std::cout << "comparing to: " << stateMap.object << " - " << stateMap.attribute << std::endl;
    if (value.object == stateMap.object && value.attribute == stateMap.attribute){
      stateMap.attr(state, value.value, now);
      return;
    }
  }
  std::cout << "(" << value.object << ", " << value.attribute  << ") - " << " not supported" << std::endl;
  assert(false);
}

void setState(engineState& state, std::vector<ObjectValue>& values, float now){
  for (auto &value : values){
    setState(state, value, now);
  }
}

engineState getDefaultState(unsigned int initialScreenWidth, unsigned int initialScreenHeight){
	engineState state = {
		.visualizeNormals = false,
		.showCameras = false,
		.isRotateSelection = false,
		.selectedName = "no object selected",
		.useDefaultCamera = true,
		.moveRelativeEnabled = false,
	  .currentScreenWidth = initialScreenWidth,
		.currentScreenHeight = initialScreenHeight,
		.cursorLeft = initialScreenWidth / 2,
		.cursorTop  = initialScreenHeight / 2,
    .currentHoverIndex = -1,
    .lastHoverIndex = -1,
    .hoveredIdInScene = false,
    .lastHoveredIdInScene = false,
		.activeCamera = 0,
    .cameraInterp = CamInterpolation { .shouldInterpolate = false },
		.additionalText = "",
    .manipulatorState = createManipulatorData(),
		.enableManipulator = false,
		.manipulatorMode = NONE,
		.manipulatorAxis = NOAXIS,
    .manipulatorLineId = 0,
    .manipulatorPositionMode = SNAP_CONTINUOUS,
    .translateMirror = false,
    .rotateMode = SNAP_CONTINUOUS,
    .scalingGroup = INDIVIDUAL_SCALING,
    .snapManipulatorScales = true,
    .preserveRelativeScale = false,
		.firstMouse = true,
		.lastX = 0,
		.lastY = 0,
		.offsetX = 0,
		.offsetY = 0,
    .mouseIsDown = false,
    .cursorBehavior = CURSOR_NORMAL,
		.enableDiffuse = true,
		.enableSpecular = true,
    .enablePBR = false,
		.showBoneWeight = false,
  	.useBoneTransform = true,
  	.textureIndex = 0,
  	.shouldPaint = false,
    .shouldTerrainPaint = false,
  	.enableBloom = false, 
  	.bloomAmount = 1.f,   
    .bloomBlurAmount = 5,
    .enableFog = true,  
    .fogColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
    .ambient = glm::vec3(0.4, 0.4, 0.4),
    .exposureStart = 0.f,
    .oldExposure = 1.f,
    .targetExposure = 1.f,
    .exposure = 1.f,
  	.takeScreenshot = false,
    .screenshotPath = "",
    .highlight = true,
    .multiselect = false,
    .editor = EditorContent{ .forceSelectIndex = 0, .activeObj = 0 },
    .isRecording = false,
    .recordingIndex = -1,
    .renderMode = RENDER_FINAL,
    .snappingMode = SNAP_RELATIVE,
    .drawPoints = false,
    .moveUp = false,
    .cameraFast = true,
    .depthBufferLayer = -1,
    .printKeyStrokes = false,
    .cullEnabled = true,
    .groupSelection = true,
    .activeCameraObj = NULL,
    .activeCameraData = NULL,
    .skybox = "",
    .skyboxcolor = glm::vec3(1.f, 1.f, 1.f),
    .showSkybox = true,
    .enableDof = false,
    .swapInterval = 0,
    .fullscreen = false,
    .nativeViewport = true,
    .nativeResolution = true,
    .viewportSize = glm::ivec2(0, 0),
    .viewportoffset = glm::ivec2(0, 0),
    .resolution = glm::ivec2(0, 0),
    .savedWindowsize = glm::ivec4(0, 0, 600, 400),
    .borderTexture = "",
    .antialiasingMode = ANTIALIASING_NONE,
    .crosshair = "",
    .windowname = "ModEngine",
    .iconpath = "./misc/modengine.png",
    .fontsize = 3,
    .showGrid = false,
    .gridSize = 10,
    .easyUse = createEasyUse(),
    .worldpaused = false,
	};
	return state;
}

void setInitialState(engineState& state, std::string file, float now, std::function<std::string(std::string)> readFile){
  auto tokens = parseFormat(readFile(file));
  for (auto &token : tokens){
    ObjectValue objValue {
      .object = token.target,
      .attribute = token.attribute,
      .value = parseAttributeValue(token.payload),
    };
    setState(state, objValue, now); 
  }
}
