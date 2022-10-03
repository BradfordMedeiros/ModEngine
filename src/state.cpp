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

struct StateBoolSerializer {
  size_t structOffset;
  std::string enabledValue;
  std::string disabledValue;
};
std::function<void(engineState& state, AttributeValue value, float now)> getSetAttr(StateBoolSerializer serializer){
  return [serializer](engineState& state, AttributeValue value, float now) -> void { 
    bool* boolValue = (bool*)(((char*)&state) + serializer.structOffset);
    auto enabledStr = std::get_if<std::string>(&value);
    if (enabledStr != NULL){
      if (*enabledStr == serializer.enabledValue){
        *boolValue = true;
      }else if (*enabledStr == serializer.disabledValue){
        *boolValue = false;
      }else{
        modassert(false, std::string("invalid string: ") + *enabledStr);
      }
      return;
    }
    modassert(false, "invalid type");
  };
}
ObjectStateMapping simpleBoolSerializer(std::string object, std::string attribute, std::string enabledValue, std::string disabledValue, size_t offset){
  ObjectStateMapping mapping {
    .attr = getSetAttr(
      StateBoolSerializer{
        .structOffset = offset,      
        .enabledValue = enabledValue,
        .disabledValue = disabledValue,
      }
    ),
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}
ObjectStateMapping simpleBoolSerializer(std::string object, std::string attribute, size_t offset){
  return simpleBoolSerializer(object, attribute, "true", "false", offset);
}

template <typename T> 
std::function<void(engineState& state, AttributeValue value, float now)> getSetAttr(size_t structOffset){
  return [structOffset](engineState& state, AttributeValue value, float now) -> void { 
    T* tValue = (T*)(((char*)&state) + structOffset);
    auto attrValue = std::get_if<T>(&value);
    if (attrValue != NULL){
      *tValue = *attrValue;
    }
  };
}

template <typename T, typename F> 
std::function<void(engineState& state, AttributeValue value, float now)> getSetAttr(size_t structOffset){
  return [structOffset](engineState& state, AttributeValue value, float now) -> void { 
    T* tValue = (T*)(((char*)&state) + structOffset);
    auto attrValue = std::get_if<F>(&value);
    if (attrValue != NULL){
      *tValue = static_cast<T>(*attrValue);
    }
  };
}

ObjectStateMapping simpleVec3Serializer(std::string object, std::string attribute, size_t offset){
  ObjectStateMapping mapping {
    .attr = getSetAttr<glm::vec3>(offset),
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}
ObjectStateMapping simpleFloatSerializer(std::string object, std::string attribute, size_t offset){
  ObjectStateMapping mapping {
    .attr = getSetAttr<float>(offset),
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}
ObjectStateMapping simpleStringSerializer(std::string object, std::string attribute, size_t offset){
  ObjectStateMapping mapping {
    .attr = getSetAttr<std::string>(offset),
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}

ObjectStateMapping simpleIntSerializer(std::string object, std::string attribute, size_t offset){
  ObjectStateMapping mapping {
    .attr = getSetAttr<int, float>(offset),
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}

std::vector<ObjectStateMapping> mapping = {
  simpleBoolSerializer("diffuse", "enabled", offsetof(engineState, enableDiffuse)),
  simpleBoolSerializer("specular", "enabled", offsetof(engineState, enableSpecular)),
  simpleBoolSerializer("pbr", "enabled", offsetof(engineState, enablePBR)),
  simpleVec3Serializer("light", "amount", offsetof(engineState, ambient)),
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
  simpleBoolSerializer("fog", "enabled", offsetof(engineState, enableFog)),
  simpleBoolSerializer("bloom", "enabled", offsetof(engineState, enableBloom)),
  simpleFloatSerializer("bloom", "amount", offsetof(engineState, bloomAmount)),
  simpleIntSerializer("bloomblur", "amount", offsetof(engineState, bloomBlurAmount)),
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
  simpleStringSerializer("skybox", "texture", offsetof(engineState, skybox)),
  simpleVec3Serializer("skybox", "color", offsetof(engineState, skyboxcolor)),
  simpleBoolSerializer("skybox", "enable", offsetof(engineState, showSkybox)),
  simpleBoolSerializer("dof", "state", "enabled", "disabled", offsetof(engineState, enableDof)),
  simpleIntSerializer("rendering", "swapinterval", offsetof(engineState, swapInterval)),
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
  simpleStringSerializer("rendering", "border", offsetof(engineState, borderTexture)),
  simpleBoolSerializer("rendering", "fullscreen", offsetof(engineState, fullscreen)),
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
  simpleBoolSerializer("rendering", "cull", "enabled", "disabled", offsetof(engineState, cullEnabled)),
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
  simpleStringSerializer("mouse", "crosshair", offsetof(engineState, crosshair)),
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
  simpleBoolSerializer("tools", "preserve-scale", offsetof(engineState, preserveRelativeScale)),
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
  simpleBoolSerializer("tools", "position-mirror", offsetof(engineState, translateMirror)),
  simpleBoolSerializer("tools", "snap-scale", offsetof(engineState, snapManipulatorScales)),
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
  simpleStringSerializer("window", "name", offsetof(engineState, windowname)),
  simpleStringSerializer("window", "icon", offsetof(engineState, iconpath)),
  simpleIntSerializer("rendering", "fontsize", offsetof(engineState, fontsize)),
  simpleBoolSerializer("editor", "showgrid", offsetof(engineState, showGrid)),
  simpleIntSerializer("editor", "gridsize", offsetof(engineState, gridSize)),
  simpleIntSerializer("editor", "snapangle-index", offsetof(engineState, easyUse.currentAngleIndex)),
  simpleIntSerializer("editor", "snaptranslate-index", offsetof(engineState, easyUse.currentTranslateIndex)),
  simpleIntSerializer("editor", "snapscale-index", offsetof(engineState, easyUse.currentScaleIndex)),


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
  simpleBoolSerializer("world", "paused", offsetof(engineState, worldpaused)),
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


std::vector<ObjectValue> getState(engineState& state){
  return {
    ObjectValue {
      .object = "placeholder_object",
      .attribute = "placeholder_attribute",
      .value = "placeholder_value",
    }
  };
}