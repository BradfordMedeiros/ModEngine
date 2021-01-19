#include "./input.h"

void onMouse(bool disableInput, GLFWwindow* window, engineState& state, double xpos, double ypos, void(*rotateCamera)(float, float)){
    if (disableInput){
      return;
    }
    if(state.firstMouse){
        state.lastX = xpos;
        state.lastY = ypos;
        state.firstMouse = false;
        return;
    }
  
    float xoffset = xpos - state.lastX;
    float yoffset = state.lastY - ypos; 
    state.lastX = xpos;
    state.lastY = ypos;
    state.offsetX = xoffset;
    state.offsetY = yoffset;

    float sensitivity = 0.05;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    if (state.isRotateSelection){
      rotateCamera(xoffset, -yoffset);   // -y offset because mouse move forward is negative, which is ok, but inverted
    }else{
      state.cursorLeft += (int)(xoffset * 15);
      state.cursorTop  -= (int)(yoffset * 15);
    }
}

void mouse_button_callback(bool disableInput, GLFWwindow* window, engineState& state, int button, int action, int mods, void (*handleSerialization) (void)){
  if (disableInput){
    return;
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
    state.enableManipulator = true;
    handleSerialization();
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
    state.enableManipulator = false;
  }

  if (button == GLFW_MOUSE_BUTTON_MIDDLE){
    if (action == GLFW_PRESS){
      state.isRotateSelection = true;
    }else if (action == GLFW_RELEASE){
      state.isRotateSelection = false;
    }
    state.cursorLeft = state.currentScreenWidth / 2;
    state.cursorTop = state.currentScreenHeight / 2;
  }
    
}

void scroll_callback(GLFWwindow *window, engineState& state, double xoffset, double yoffset){
  if (state.toggleFov){
    if ((state.fov <= 0 && yoffset < 0) || (state.fov >= 180 && yoffset > 0)){
      return;
    }
    state.fov = state.fov + yoffset;
  }
}


void printControllerDebug(const unsigned char* buttons, int buttonCount){
  std::cout << "== buttons == ( ";
  for (int i = 0; i < buttonCount; i++){
    if (buttons[i] == GLFW_PRESS){
      std::cout << "DOWN ";
    }else {
      std::cout << "UP ";
    }
  }
  std::cout << " )" << std::endl;
}
void printAxisDebug(const float* axises, int count){
  std::cout << "== axises == ( ";
  for (int i = 0; i < count; i++){
    std::cout << " " << axises[i];
  }
  std::cout << " )" << std::endl;
}

float calcAxisValue(const float* axises, int count, int index, float deadzonemin, float deadzonemax, bool invert){
  assert(index < count);
  float axisValue = axises[index];
  if (axisValue > deadzonemin && axisValue < deadzonemax){
    return 0.f;
  }
  if (invert){
    axisValue = axisValue * -1.f;
  }
  return axisValue;
}

void processControllerInput(KeyRemapper& remapper, void (*moveCamera)(glm::vec3), float deltaTime){
  if (!glfwJoystickPresent(GLFW_JOYSTICK_1)){
    //std::cout << "joystick 0 not present" << std::endl;
    return;
  }
  //std::cout << "joystick is present" << std::endl;
  int count;
  auto axises = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);  
  int buttonCount;
  auto buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);

  for (auto buttonMapping : remapper.buttonMappings){
    if (buttonMapping.sourceKey < buttonCount){
      if (buttons[buttonMapping.sourceKey] == GLFW_PRESS){
        std::cout << "should remap this to: " << buttonMapping.destinationKey << std::endl;
      }
    }
  }

  auto axis1Config = getAxisConfig(remapper, 0);
  auto axis2Config = getAxisConfig(remapper, 1);
  auto axis5Config = getAxisConfig(remapper, 5);

  auto axis1Value = calcAxisValue(axises, count, 0, axis1Config.deadzonemin, axis1Config.deadzonemax, axis1Config.invert); 
  auto axis2Value = calcAxisValue(axises, count, 1, axis2Config.deadzonemin, axis2Config.deadzonemax, axis2Config.invert);
  auto axis5Value = calcAxisValue(axises, count, 5, axis5Config.deadzonemin, axis5Config.deadzonemax, axis5Config.invert);

  if (axis5Config.shouldMapKey && axis5Value > axis5Config.amount){
    std::cout << "via mapping should trigger: " << axis5Config.destinationKey << std::endl;
  }

  moveCamera(
    glm::vec3(
      axis1Value * 40.0f  * deltaTime, 
      0.0, 
      axis2Value * 40.0f  * deltaTime
    )
  );
  //printControllerDebug(buttons, buttonCount);
  //printAxisDebug(axises, count);
}

void handleInput(
  KeyRemapper& remapper,
  bool disableInput, 
  GLFWwindow *window, 
  float deltaTime, 
  engineState& state, 
	void (*translate)(float, float, float), 
  void (*scale)(float, float, float), 
  void (*rotate)(float, float, float),
  void (*moveCamera)(glm::vec3), 
  void (*nextCamera)(void),
  void (*setObjectDimensions)(int32_t index, float width, float height, float depth),
  void (*onDebugKey)(),
  void (*onArrowKey)(int key),
  void (*onCameraSystemChange)(bool usingBuiltInCamera),
  void (*onDelete)()
){
  processControllerInput(remapper, moveCamera, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
    glfwSetWindowShouldClose(window, true);
  }
  if (disableInput){    // we return after escape, so escape still quits
    return;
  }

  if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS){
    state.enableDiffuse = !state.enableDiffuse;
  }
  if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS){
    state.enableSpecular = !state.enableSpecular;
  }

  if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS){
    setObjectDimensions(1, 10, 5, 10);
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
    moveCamera(glm::vec3(0.0, 0.0, -40.0f * deltaTime));
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
    moveCamera(glm::vec3(-40.0 * deltaTime, 0.0, 0.0));
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){ 
    moveCamera(glm::vec3(0.0, 0.0, 40.0f * deltaTime));
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){ 
    moveCamera(glm::vec3(40.0f * deltaTime, 0.0, 0.0f));
  }
  if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS){
    nextCamera();
  }
   
  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS){
    //state.visualizeNormals = !state.visualizeNormals;
    //std::cout << "visualizeNormals: " << state.visualizeNormals << std::endl;
  } 
  if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS){
    state.showCameras = !state.showCameras;
    std::cout << "show cameras: " << state.showCameras << std::endl;
  }
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
    //onDebugKey();
  }
  if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS){
    state.useDefaultCamera = !state.useDefaultCamera;
    std::cout << "Camera option: " << (state.useDefaultCamera ? "default" : "new") << std::endl;
    onCameraSystemChange(state.useDefaultCamera);
  }
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
    state.moveRelativeEnabled = !state.moveRelativeEnabled;
  }
  if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS){
    state.toggleFov = !state.toggleFov;
    std::cout << "ToggleFOV: " << state.toggleFov << std::endl;
  }
  if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS){
    if (state.axis == 0){
      state.axis = 1;
    }else{
      state.axis = 0;
    }
  }
  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS){
    state.mode = 0;
  }
  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS){
    state.mode = 1;
  } 
  if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS){
    state.mode = 2;
  }

  if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS){
    state.manipulatorMode = TRANSLATE;
  }
  if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS){
    state.manipulatorMode = SCALE;
  }
  if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS){
    state.manipulatorMode = ROTATE;
  }
  if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS){
    state.offsetTextureMode = !state.offsetTextureMode;
    std::cout << "offset texture mode: " << state.offsetTextureMode << std::endl;
  } 
  if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS){
    state.showCameras = false; // turn this off so light isn't trapped in the box 
  }
  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
    state.manipulatorAxis = XAXIS;
  }
  if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS){
    state.manipulatorAxis = YAXIS;
  }
  if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS){
    state.manipulatorAxis = ZAXIS;
  }

   if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
     onArrowKey(GLFW_KEY_RIGHT);
   }
   if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
     onArrowKey(GLFW_KEY_LEFT);
   }
   if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
     onArrowKey(GLFW_KEY_UP);
   }
   if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
     onArrowKey(GLFW_KEY_DOWN);
   }

   if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS){
     state.showBoneWeight = !state.showBoneWeight;
     std::cout << "state: show bone weight " << state.showBoneWeight << std::endl;
   }
   if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS){
     state.useBoneTransform = !state.useBoneTransform;
     std::cout << "state: use bone transform: " << state.useBoneTransform << std::endl;
   }

   if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS){
      state.discardAmount += 0.01;
   }
   if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS){
      state.discardAmount -= 0.01;
   }

   if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS){
      onDelete();
   }

   if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS){
      state.enableBloom = !state.enableBloom;
   }

   if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
      std::cout << dumpLogInfo() << std::endl;
   }
}
