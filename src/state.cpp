#include "./state.h"

extern btIDebugDraw* debuggerDrawer;

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
  if (mode == RENDER_SELECTION){
    return "selection";
  }
  modassert(false, "invalid render mode");
  return "";
}

struct ObjectStateMapping {
  std::function<void(engineState& state, AttributeValue, float)> attr;
  std::function<AttributeValue(engineState& state)> getAttr;
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

template <typename T>
std::function<AttributeValue(engineState& state)> getGetAttr(size_t offset){
  return [offset](engineState& state) -> AttributeValue {
    T* value = (T*)(((char*)&state) + offset);
    return *value; 
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
    .getAttr = [structOffset](engineState& state) -> AttributeValue {
      glm::ivec2* vecValue = (glm::ivec2*)(((char*)&state) + structOffset);
      return glm::vec2(vecValue -> x, vecValue -> y);
    },
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}
ObjectStateMapping simpleVec2Serializer(std::string object, std::string attribute, size_t structOffset, std::optional<size_t> usingDefaultOffset){
  ObjectStateMapping mapping {
    .attr = [structOffset, usingDefaultOffset](engineState& state, AttributeValue value, float now) -> void { 
      auto viewportSizeStr = std::get_if<std::string>(&value);
      if (viewportSizeStr != NULL){
        glm::vec2* vecValue = (glm::vec2*)(((char*)&state) + structOffset);
        *vecValue = parseVec2(*viewportSizeStr);
        if (usingDefaultOffset.has_value()){
          bool* usingDefault = (bool*)(((char*)&state) + usingDefaultOffset.value());
          *usingDefault = false;
        }
      }     
    },
    .getAttr = [structOffset](engineState& state) -> AttributeValue {
      glm::vec2* vecValue = (glm::vec2*)(((char*)&state) + structOffset);
      return serializeVec(*vecValue);
    },
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}
ObjectStateMapping simpleVec3Serializer(std::string object, std::string attribute, size_t offset){
  ObjectStateMapping mapping {
    .attr = getSetAttr<glm::vec3>(offset),
    .getAttr = getGetAttr<glm::vec3>(offset),
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}
ObjectStateMapping simpleFloatSerializer(std::string object, std::string attribute, size_t offset){
  ObjectStateMapping mapping {
    .attr = getSetAttr<float>(offset),
    .getAttr = getGetAttr<float>(offset),
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}
ObjectStateMapping simpleStringSerializer(std::string object, std::string attribute, size_t offset){
  ObjectStateMapping mapping {
    .attr = getSetAttr<std::string>(offset),
    .getAttr = getGetAttr<std::string>(offset),
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}
ObjectStateMapping simpleIntSerializer(std::string object, std::string attribute, size_t offset){
  ObjectStateMapping mapping {
    .attr = getSetAttr<int, float>(offset),
    .getAttr = [offset](engineState& state) -> AttributeValue {
      int* value = (int*)(((char*)&state) + offset);
      float floatValue = static_cast<float>(*value);
      return floatValue; 
    },
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
    .getAttr = [enums, enumStrings, offset](engineState& state) -> AttributeValue {
      int* enumValue = (int*)(((char*)&state) + offset);
      for (int i = 0; i < enums.size(); i++){
        if (enums.at(i) == *enumValue){
          return enumStrings.at(i);
        }
      }
      modassert(false, "invalid enum value in enum serializer");
      return "";
    },
    .object = object,
    .attribute = attribute,
  };
  return mapping;
}

std::vector<ObjectStateMapping> mapping = {
  simpleFloatSerializer("game", "speed", offsetof(engineState, engineSpeed)),
  simpleBoolSerializer("physics", "enabled", offsetof(engineState, enablePhysics)),
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void {
      auto debugEnabled = std::get_if<std::string>(&value);
      if (debugEnabled != NULL){
        if (*debugEnabled == "true"){
          debuggerDrawer -> setDebugMode(1);
          state.enablePhysicsDebug = true;
        }else if (*debugEnabled == "false") {
          debuggerDrawer -> setDebugMode(0);
          state.enablePhysicsDebug = false;
        }
      }
    },
    .getAttr = [](engineState& state) -> AttributeValue {
      if (state.enablePhysicsDebug){
        return "enabled";
      }
      return "disabled";
    },
    .object = "physics",
    .attribute = "debug",
  },

  simpleBoolSerializer("editor", "debug", offsetof(engineState, showDebug)),
  simpleBoolSerializer("editor", "showbones", offsetof(engineState, showBones)),
  simpleIntSerializer("editor", "debugmask", offsetof(engineState, showDebugMask)),
  simpleBoolSerializer("diffuse", "enabled", offsetof(engineState, enableDiffuse)),
  simpleBoolSerializer("specular", "enabled", offsetof(engineState, enableSpecular)),
  simpleBoolSerializer("attenuation", "enabled", offsetof(engineState, enableAttenuation)),
  simpleBoolSerializer("voxellighting", "enabled", offsetof(engineState, enableVoxelLighting)),
  simpleBoolSerializer("voxellighting", "visualize", offsetof(engineState, visualizeVoxelLightingCells)),
  simpleBoolSerializer("shadows", "enabled", offsetof(engineState, enableShadows)),
  simpleFloatSerializer("shadows", "intensity", offsetof(engineState, shadowIntensity)),
  simpleVec3Serializer("light", "amount", offsetof(engineState, ambient)),
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto color = std::get_if<glm::vec4>(&value);
      if (color != NULL){
        auto fogColor = *color;
        state.fogColor = glm::vec4(fogColor.x, fogColor.y, fogColor.z, fogColor.w); 
      }
    },
    .getAttr = [](engineState& state) -> AttributeValue { return glm::vec4(state.fogColor.x, state.fogColor.y, state.fogColor.z, state.fogColor.w); },
    .object = "fog",
    .attribute = "color",
  },
  simpleBoolSerializer("fog", "enabled", offsetof(engineState, enableFog)),
  simpleFloatSerializer("fog", "mincutoff", offsetof(engineState, fogMinCutoff)),
  simpleFloatSerializer("fog", "maxcutoff", offsetof(engineState, fogMaxCutoff)),
  simpleBoolSerializer("bloom", "enabled", offsetof(engineState, enableBloom)),
  simpleFloatSerializer("bloom", "threshold", offsetof(engineState, bloomThreshold)),
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
    .getAttr = [](engineState& state) -> AttributeValue { return state.targetExposure; },
    .object = "exposure",
    .attribute = "amount",
  },
  simpleBoolSerializer("exposure", "enabled", offsetof(engineState, enableExposure)),
  simpleBoolSerializer("gamma", "enabled", offsetof(engineState, enableGammaCorrection)),
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
  simpleBoolSerializer("rendering", "frustumcull", "enabled", "disabled", offsetof(engineState, enableFrustumCulling)),

  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto strValue = std::get_if<std::string>(&value);
      modassert(strValue, "mouse cursor invalid type");
      if (*strValue == "normal"){
        state.cursorBehavior = CURSOR_NORMAL;
        state.shouldToggleCursor = true;
        return;
      }else if (*strValue == "hidden"){
        state.cursorBehavior = CURSOR_HIDDEN;
        state.shouldToggleCursor = true;
        return;
      }else if (*strValue == "capture"){
        state.cursorBehavior = CURSOR_CAPTURE;
        state.shouldToggleCursor = true;
        return;
      }
      modassert(false, "invalid value to mouse cursor");
    },
    .getAttr = [](engineState& state) -> AttributeValue { 
      if (state.cursorBehavior == CURSOR_NORMAL){
        return "normal";
      }else if (state.cursorBehavior == CURSOR_HIDDEN){
        return "hidden";
      }else if (state.cursorBehavior == CURSOR_CAPTURE){
        return "capture";
      }
      modassert(false, "invalid value set state");
      return 0.f;
    },
    .object = "mouse",
    .attribute = "cursor",
  },
  simpleBoolSerializer("mouse", "show-cursor", offsetof(engineState, showCursor)),
  simpleEnumSerializer("tools", "manipulator-mode", { TRANSLATE, SCALE, ROTATE }, { "translate", "scale", "rotate" }, offsetof(engineState, manipulatorMode)),
  simpleEnumSerializer("tools", "manipulator-axis", { XAXIS, YAXIS, ZAXIS, NOAXIS }, { "x", "y", "z", "none" }, offsetof(engineState, manipulatorAxis)),
  simpleBoolSerializer("tools", "preserve-scale", offsetof(engineState, preserveRelativeScale)),
  simpleEnumSerializer("tools", "scale-group", { INDIVIDUAL_SCALING, GROUP_SCALING }, { "individual", "group" }, offsetof(engineState, scalingGroup)),
  simpleEnumSerializer("tools", "snap-position", { SNAP_CONTINUOUS, SNAP_RELATIVE, SNAP_ABSOLUTE }, { "none", "relative", "absolute" }, offsetof(engineState, manipulatorPositionMode)),
  simpleBoolSerializer("tools", "position-relative", offsetof(engineState, relativePositionMode)),  
  simpleBoolSerializer("tools", "position-mirror", offsetof(engineState, translateMirror)),
  simpleBoolSerializer("tools", "snap-scale", offsetof(engineState, snapManipulatorScales)),
  simpleEnumSerializer("tools", "snap-rotate", { SNAP_CONTINUOUS, SNAP_RELATIVE, SNAP_ABSOLUTE }, { "none", "relative", "absolute" }, offsetof(engineState, rotateMode)),
  
  simpleStringSerializer("window", "name", offsetof(engineState, windowname)),
  simpleStringSerializer("window", "icon", offsetof(engineState, iconpath)),
  simpleIntSerializer("rendering", "fontsize", offsetof(engineState, fontsize)),
  simpleBoolSerializer("editor", "showgrid", offsetof(engineState, showGrid)),
  simpleIntSerializer("editor", "gridsize", offsetof(engineState, gridSize)),
  simpleFloatSerializer("editor", "snapangle", offsetof(engineState, easyUse.currentAngle)),
  simpleFloatSerializer("editor", "snaptranslate", offsetof(engineState, easyUse.currentTranslate)),
  simpleFloatSerializer("editor", "snapscale", offsetof(engineState, easyUse.currentScale)),
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto vec4Rotation = std::get_if<glm::vec4>(&value);
      modassert(vec4Rotation, "vec4 rotation is null");
      setSnapCoordinateSystem(state.easyUse, parseQuat(*vec4Rotation));
    },
    .getAttr = [](engineState& state) -> AttributeValue { 
      if (state.easyUse.orientation.has_value()){
        return serializeQuat(state.easyUse.orientation.value()); 
      }
      return glm::vec4(0.f, 0.f, -1.f, 0.f);
    },
    .object = "editor",
    .attribute = "grid",
  },
  ObjectStateMapping {
    .attr = [](engineState& state, AttributeValue value, float now) -> void { 
      auto selectedIndexStr = std::get_if<std::string>(&value);
      if (selectedIndexStr != NULL){
        int index =  std::atoi(selectedIndexStr -> c_str());
        std::cout << "selected index: " << index << std::endl;
        state.forceSelectIndex = index;
      }
    },
    .getAttr = [](engineState& state) -> AttributeValue { return serializeFloat(state.forceSelectIndex); },
    .object = "editor",
    .attribute = "selected-index",
  },
  simpleBoolSerializer("world", "paused", offsetof(engineState, worldpaused)),
  simpleVec2Serializer("debug", "textoffset", offsetof(engineState, infoTextOffset), std::nullopt),
  simpleBoolSerializer("editor", "groupselection", offsetof(engineState, groupSelection)),
  simpleFloatSerializer("sound", "volume", offsetof(engineState, volume)),
  simpleBoolSerializer("sound", "mute", offsetof(engineState, muteSound)),
  simpleEnumSerializer("editor", "disableinput", { DISABLED, ENABLED, CAMERA_ONLY }, { "true", "false", "camera" }, offsetof(engineState, inputMode)),
  simpleBoolSerializer("octree", "rebuild", offsetof(engineState, rebuildOctreePhysicsOnEdit)),
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
    .engineSpeed = 1.f,
    .enablePhysics = true,
    .enablePhysicsDebug = false,
    .showDebug = false,
    .showBones = false,
    .showDebugMask = 0,
		.isRotateSelection = false,
    .selectionDisabled = false,
		.useDefaultCamera = true,
		.moveRelativeEnabled = false,
	  .currentScreenWidth = initialScreenWidth,
		.currentScreenHeight = initialScreenHeight,
		.cursorLeft = initialScreenWidth / 2,
		.cursorTop  = initialScreenHeight / 2,
    .currentHoverIndex = -1,
    .lastHoverIndex = -1,
    .hoveredItemColor = glm::vec3(0.f, 0.f, 0.f),
    .hoveredColor = std::nullopt,
    .currentCursorDepth = std::nullopt,
		.activeCamera = 0,
		.additionalText = "",
    .manipulatorState = createManipulatorData(),
		.manipulatorMode = NONE,
		.manipulatorAxis = NOAXIS,
    .manipulatorLineId = 0,
    .manipulatorPositionMode = SNAP_CONTINUOUS,
    .relativePositionMode = false,
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
    .shouldToggleCursor = false,
    .showCursor = true,
		.enableDiffuse = true,
		.enableSpecular = true,
    .enableAttenuation = true,
    .enableVoxelLighting = true,
    .visualizeVoxelLightingCells = false,
		.showBoneWeight = false,
  	.useBoneTransform = true,
  	.textureIndex = 0,
  	.enableBloom = false, 
    .bloomThreshold = 2.7f,
  	.bloomAmount = 1.f,   
    .bloomBlurAmount = 5,
    .enableFog = true,  
    .fogMinCutoff = 0.5f,
    .fogMaxCutoff = 0.9999f,
    .fogColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
    .ambient = glm::vec3(0.4, 0.4, 0.4),
    .exposureStart = 0.f,
    .oldExposure = 1.f,
    .targetExposure = 1.f,
    .exposure = 1.f,
    .enableExposure = false,
    .enableGammaCorrection = false,
  	.takeScreenshot = false,
    .screenshotPath = "",
    .multiselect = false,
    .editor = EditorContent{},
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
    .activeCameraObj = std::nullopt,
    .activeCameraData = NULL,
    .skybox = "",
    .updateSkybox = true,
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
    .windowname = "ModEngine",
    .iconpath = "./docs/modengine.png",
    .fontsize = 3,
    .showGrid = false,
    .gridSize = 10,
    .easyUse = createEasyUse(),
    .worldpaused = false,
    .infoTextOffset = glm::ivec2(0, 0),
    .forceSelectIndex = 0,
    .volume = 1.f,
    .muteSound = false,
    .inputMode = ENABLED,
    .escapeQuits = false,
    .navmeshTextureId = std::nullopt,
    .rampDirection = RAMP_LEFT,
    .activeTextureIndex = 0,
    .adjustedCoords = glm::ivec2(0, 0),
    .enableFrustumCulling = false,
    .visualizeFrustum = true,
    .cullingObject = std::nullopt,
    .rebuildOctreePhysicsOnEdit = true,
	};
	return state;
}

void setInitialState(engineState& state, std::string file, float now, std::function<std::string(std::string)> readFile, bool disableInput){
  state.inputMode = disableInput ? DISABLED : ENABLED;

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
    values.push_back(
      ObjectValue {
        .object = stateMap.object,
        .attribute = stateMap.attribute,
        .value = value,
      }
    );
  }
  return values;
}