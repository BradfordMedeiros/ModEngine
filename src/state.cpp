#include "./state.h"

engineState getDefaultState(unsigned int initialScreenWidth, unsigned int initialScreenHeight){
	engineState state = {
		.visualizeNormals = false,
		.showCameras = false,
		.isRotateSelection = false,
		.selectedName = "no object selected",
		.useDefaultCamera = false,
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
		.additionalText = "",
		.enableManipulator = false,
		.manipulatorMode = NONE,
		.manipulatorAxis = NOAXIS,
		.firstMouse = true,
		.lastX = 0,
		.lastY = 0,
		.offsetX = 0,
		.offsetY = 0,
    .mouseIsDown = false,
		.enableDiffuse = true,
		.enableSpecular = true,
		.showBoneWeight = false,
  	.useBoneTransform = true,
  	.textureIndex = 0,
  	.shouldPaint = false,
    .shouldTerrainPaint = false,
  	.enableBloom = false, 
  	.bloomAmount = 1.f,   
    .enableFog = true,  
    .fogColor = glm::vec3(0.f, 0.f, 0.f), 
  	.takeScreenshot = false,
    .highlight = true,
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
    .printKeyStrokes = true,
    .cullEnabled = false,
    .groupSelection = true,
    .pauseWorldTiming = false,
    .activeCameraObj = NULL,
	};
	return state;
}

struct ObjectStateMapping {
  std::function<void(engineState& state, AttributeValue)> attr;
  std::string object;
  std::string attribute;
};

std::vector<ObjectStateMapping> mapping = {
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value) -> void { 
      auto enabled = std::get_if<std::string>(&value);
      if (enabled != NULL){
        state.enableDiffuse = *enabled == "true";
      }
    },
    .object = "diffuse",
    .attribute = "enabled",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value) -> void { 
      auto enabled = std::get_if<std::string>(&value);
      if (enabled != NULL){
        state.enableSpecular = *enabled == "true";
      }
    },
    .object = "specular",
    .attribute = "enabled",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value) -> void { 
      auto color = std::get_if<glm::vec3>(&value);
      if (color != NULL){
        state.fogColor = *color; 
      }
    },
    .object = "fog",
    .attribute = "color",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value) -> void { 
      auto enabled = std::get_if<std::string>(&value);
      if (enabled != NULL){
        state.enableFog = *enabled == "true";
      }
    },
    .object = "fog",
    .attribute = "enabled",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value) -> void { 
      auto enabled = std::get_if<std::string>(&value);
      if (enabled != NULL){
        state.enableBloom = *enabled == "true";
      }
    },
    .object = "bloom",
    .attribute = "enabled",
  },
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value) -> void { 
      auto amount = std::get_if<float>(&value);
      if (amount != NULL){
        state.bloomAmount = *amount;
      }
    },
    .object = "bloom",
    .attribute = "amount",
  },
};

void setState(engineState& state, ObjectValue& value){
  for (auto &stateMap : mapping){
    if (value.object == stateMap.object && value.attribute == stateMap.attribute){
      stateMap.attr(state, value.value);
      return;
    }
  }
  std::cout << value.object << " not supported" << std::endl;
  assert(false);
}

void setState(engineState& state, std::vector<ObjectValue>& values){
  for (auto &value : values){
    setState(state, value);
  }
}