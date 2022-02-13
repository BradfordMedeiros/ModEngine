#include "./state.h"

extern World world;

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
        loadSkybox(world, *skyboxTexture); 
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
      auto captureCursor = std::get_if<std::string>(&value);
      if (captureCursor != NULL){
        auto valid = maybeParseBool(*captureCursor, &state.captureCursor);
        assert(valid);
        std::cout << "captureCursor: " << (state.captureCursor ? "true" : "false") << std::endl;
      }     
    },
    .object = "mouse",
    .attribute = "capturecursor",
  },
};

void setState(engineState& state, ObjectValue& value, float now){
  for (auto &stateMap : mapping){
    std::cout << "comparing to: " << stateMap.object << " - " << stateMap.attribute << std::endl;
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
		.enableManipulator = false,
		.manipulatorMode = NONE,
		.manipulatorAxis = NOAXIS,
    .manipulatorLineId = 0,
		.firstMouse = true,
		.lastX = 0,
		.lastY = 0,
		.offsetX = 0,
		.offsetY = 0,
    .mouseIsDown = false,
    .captureCursor = false,
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
    .exposureStart = 0.f,
    .oldExposure = 1.f,
    .targetExposure = 1.f,
    .exposure = 1.f,
  	.takeScreenshot = false,
    .highlight = true,
    .multiselect = false,
    .editor = EditorContent{ .activeObj = 0 },
    .isRecording = false,
    .recordingIndex = -1,
    .renderMode = RENDER_FINAL,
    .snappingMode = SNAP_RELATIVE,
    .drawPoints = false,
    .moveUp = false,
    .cameraFast = true,
    .depthBufferLayer = -1,
    .printKeyStrokes = false,
    .cullEnabled = false,
    .groupSelection = true,
    .pauseWorldTiming = false,
    .activeCameraObj = NULL,
    .activeCameraData = NULL,
    .skybox = "",
    .skyboxcolor = glm::vec3(1.f, 1.f, 1.f),
    .enableDof = false,
    .swapInterval = 0,
    .fullscreen = false,
    .nativeViewport = true,
    .nativeResolution = true,
    .viewportSize = glm::ivec2(0, 0),
    .viewportoffset = glm::ivec2(0, 0),
    .resolution = glm::ivec2(0, 0),
    .borderTexture = "",
	};
	return state;
}

void setInitialState(engineState& state, std::string file, float now){
  auto tokens = parseFormat(loadFile(file));
  for (auto &token : tokens){
    ObjectValue objValue {
      .object = token.target,
      .attribute = token.attribute,
      .value = parseAttributeValue(token.payload),
    };
    setState(state, objValue, now); 
  }
}
