#include "./state.h"

extern World world;

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
        //state.fogColor = *color; 
        state.fogColor = glm::vec4(0, 1.0f, 0, 1.f);
        return;
      }
      //assert(false);
      state.fogColor = glm::vec4(1.f, 0, 0, 1.f);
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
  ObjectStateMapping{
    .attr = [](engineState& state, AttributeValue value) -> void { 
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
    .attr = [](engineState& state, AttributeValue value) -> void { 
      auto skyboxColor = std::get_if<glm::vec3>(&value);
      if (skyboxColor != NULL){
        std::cout << "state: update skybox color: " << print(*skyboxColor) << std::endl;
        state.skyboxcolor = *skyboxColor; 
      }

      auto skyboxStr = std::get_if<std::string>(&value);
      std::cout << "not valid value!: " << (skyboxColor == NULL) << " - " <<  (skyboxStr == NULL) << std::endl;
      std::cout << "value is: [" << *skyboxStr << "]" << std::endl;
      assert(false);
    },
    .object = "skybox",
    .attribute = "color",
  },
};

void setState(engineState& state, ObjectValue& value){
  for (auto &stateMap : mapping){
    std::cout << "comparing to: " << stateMap.object << " - " << stateMap.attribute << std::endl;
    if (value.object == stateMap.object && value.attribute == stateMap.attribute){
      stateMap.attr(state, value.value);
      return;
    }
  }
  std::cout << "(" << value.object << ", " << value.attribute  << ") - " << " not supported" << std::endl;
  assert(false);
}

void setState(engineState& state, std::vector<ObjectValue>& values){
  for (auto &value : values){
    setState(state, value);
  }
}

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
    .fogColor = glm::vec4(0.f, 0.f, 0.f, 1.f), 
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
    .skybox = "",
    .skyboxcolor = glm::vec3(1.f, 1.f, 1.f),
	};
	return state;
}

void setInitialState(engineState& state, std::string file){
  auto tokens = parseFormat(loadFile(file));
  for (auto &token : tokens){
    ObjectValue objValue {
      .object = token.target,
      .attribute = token.attribute,
      .value = token.payload,
    };
    setState(state, objValue); 
  }
}
