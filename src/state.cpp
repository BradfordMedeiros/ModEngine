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
  std::function<std::optional<AttributeValue>(engineState& state)> getAttr;
  std::string object;
  std::string attribute;
};

struct StateBoolSerializer {
  size_t structOffset;
  std::string enabledValue;
  std::string disabledValue;
};

std::optional<AttributeValue> getStateAttrNotImplemented(engineState& state){
  return std::nullopt;
}

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
    .getAttr = [offset, enabledValue, disabledValue](engineState& state) -> AttributeValue {
      bool* value = (bool*)(((char*)&state) + offset);
      return *value ? enabledValue : disabledValue;
    },
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

ObjectStateMapping simpleIVec2Serializer(std::string object, std::string attribute, size_t structOffset, std::optional<size_t> usingDefaultOffset){
  ObjectStateMapping mapping {
    .attr = [structOffset, usingDefaultOffset](engineState& state, AttributeValue value, float now) -> void { 
      auto viewportSizeStr = std::get_if<std::string>(&value);
      if (viewportSizeStr != NULL){
        glm::ivec2* vecValue = (glm::ivec2*)(((char*)&state) + structOffset);
        *vecValue = parseVec2(*viewportSizeStr);
        if (usingDefaultOffset.has_value()){
          bool* usingDefault = (bool*)(((char*)&state) + usingDefaultOffset.value());
          *usingDefault = false;
        }
      }     
    },
    .getAttr = getStateAttrNotImplemented,
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}

ObjectStateMapping simpleVec3Serializer(std::string object, std::string attribute, size_t offset){
  ObjectStateMapping mapping {
    .attr = getSetAttr<glm::vec3>(offset),
    .getAttr = getStateAttrNotImplemented,
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}
ObjectStateMapping simpleFloatSerializer(std::string object, std::string attribute, size_t offset){
  ObjectStateMapping mapping {
    .attr = getSetAttr<float>(offset),
    .getAttr = getStateAttrNotImplemented,
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}
ObjectStateMapping simpleStringSerializer(std::string object, std::string attribute, size_t offset){
  ObjectStateMapping mapping {
    .attr = getSetAttr<std::string>(offset),
    .getAttr = getStateAttrNotImplemented,
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}

ObjectStateMapping simpleIntSerializer(std::string object, std::string attribute, size_t offset){
  ObjectStateMapping mapping {
    .attr = getSetAttr<int, float>(offset),
    .getAttr = getStateAttrNotImplemented,
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}

ObjectStateMapping simpleEnumSerializer(std::string object, std::string attribute, std::vector<int> enums, std::vector<std::string> enumStrings, size_t offset){
  modassert(enums.size() == enumStrings.size(), "enum serializer enums and strings invalid size");
  ObjectStateMapping mapping {
    .attr = [enums, enumStrings, offset](engineState& state, AttributeValue value, float now) -> void { 
      auto strValue = std::get_if<std::string>(&value);
      if (strValue != NULL){
        for (int i = 0; i < enums.size(); i++){
          auto enumString = enumStrings.at(i);
          if (enumString == *strValue){
            auto enumValue = enums.at(i);
            int* value = (int*)(((char*)&state) + offset);
            *value = enumValue;
            return;
          }
        }
        modassert(false, std::string("invalid enum type: ") + *strValue);
      }
    },
    .getAttr = getStateAttrNotImplemented,
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
    .getAttr = getStateAttrNotImplemented,
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
    .getAttr = getStateAttrNotImplemented,
    .object = "exposure",
    .attribute = "amount",
  },
  simpleStringSerializer("skybox", "texture", offsetof(engineState, skybox)),
  simpleVec3Serializer("skybox", "color", offsetof(engineState, skyboxcolor)),
  simpleBoolSerializer("skybox", "enable", offsetof(engineState, showSkybox)),
  simpleBoolSerializer("dof", "state", "enabled", "disabled", offsetof(engineState, enableDof)),
  simpleIntSerializer("rendering", "swapinterval", offsetof(engineState, swapInterval)),
  simpleIVec2Serializer("rendering", "viewport", offsetof(engineState, viewportSize), offsetof(engineState, nativeViewport)),
  simpleIVec2Serializer("rendering", "viewportoffset", offsetof(engineState, viewportoffset), std::nullopt),
  simpleIVec2Serializer("rendering", "resolution", offsetof(engineState, resolution), offsetof(engineState, nativeResolution)),
  simpleStringSerializer("rendering", "border", offsetof(engineState, borderTexture)),
  simpleBoolSerializer("rendering", "fullscreen", offsetof(engineState, fullscreen)),
  simpleEnumSerializer("rendering", "antialiasing", { ANTIALIASING_NONE, ANTIALIASING_MSAA }, { "none", "msaa" }, offsetof(engineState, antialiasingMode)),
  simpleBoolSerializer("rendering", "cull", "enabled", "disabled", offsetof(engineState, cullEnabled)),
  simpleEnumSerializer("mouse", "cursor", { CURSOR_NORMAL, CURSOR_HIDDEN, CURSOR_CAPTURE }, { "normal", "hidden", "capture" }, offsetof(engineState, cursorBehavior)),
  simpleStringSerializer("mouse", "crosshair", offsetof(engineState, crosshair)),
  simpleEnumSerializer("tools", "manipulator-mode", { TRANSLATE, SCALE, ROTATE }, { "translate", "scale", "rotate" }, offsetof(engineState, manipulatorMode)),
  simpleEnumSerializer("tools", "manipulator-axis", { XAXIS, YAXIS, ZAXIS, NOAXIS }, { "x", "y", "z", "none" }, offsetof(engineState, manipulatorAxis)),
  simpleBoolSerializer("tools", "preserve-scale", offsetof(engineState, preserveRelativeScale)),
  simpleEnumSerializer("tools", "scale-group", { INDIVIDUAL_SCALING, GROUP_SCALING }, { "individual", "group" }, offsetof(engineState, scalingGroup)),
  simpleEnumSerializer("tools", "snap-position", { SNAP_CONTINUOUS, SNAP_RELATIVE, SNAP_ABSOLUTE }, { "none", "relative", "absolute" }, offsetof(engineState, manipulatorPositionMode)),
  simpleBoolSerializer("tools", "position-mirror", offsetof(engineState, translateMirror)),
  simpleBoolSerializer("tools", "snap-scale", offsetof(engineState, snapManipulatorScales)),
  simpleEnumSerializer("tools", "snap-rotate", { SNAP_CONTINUOUS, SNAP_RELATIVE, SNAP_ABSOLUTE }, { "none", "relative", "absolute" }, offsetof(engineState, rotateMode)),
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
    .getAttr = getStateAttrNotImplemented,
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
  std::vector<ObjectValue> values;
  for (auto &stateMap : mapping){
    auto value = stateMap.getAttr(state);
    //modassert(value.has_value(), std::string("not yet implemented: ") + stateMap.object + " - " + stateMap.attribute);
    if (value.has_value()){
      values.push_back(
        ObjectValue {
          .object = stateMap.object,
          .attribute = stateMap.attribute,
          .value = value.value(),
        }
      );
    }
  }
  return values;
}