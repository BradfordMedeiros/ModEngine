#include "./state.h"

engineState getDefaultState(unsigned int initialScreenWidth, unsigned int initialScreenHeight){
	engineState state = {
		.visualizeNormals = false,
		.showCameras = false,
		.isRotateSelection = false,
		.selectedName = "no object selected",
		.useDefaultCamera = false,
		.moveRelativeEnabled = false,
		.mode = 0,  // 0 = translate mode, 1 = scale mode, 2 = rotate
		.axis = 0,  // 0 = x, 1 = y, 2 = z
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
		.enableDiffuse = true,
		.enableSpecular = true,
		.showDepthBuffer = false,
		.fov = 45.f,
		.toggleFov = false,
		.showBoneWeight = false,
  	.useBoneTransform = true,
  	.discardAmount = 0.5,
  	.offsetTextureMode = false,
  	.portalTextureIndex = 0,
  	.textureDisplayMode = true,
  	.shouldPaint = false,
  	.enableBloom = false,
  	.bloomAmount = 1.f,
  	.takeScreenshot = false,
    .highlight = false,
    .shouldSelect = true,
    .editor = EditorContent{
      .selectedObj = EditorItem { 
        .id = -1,
        .name = "",
      },
    },
    .multiselectMode = false,
	};
	return state;
}

