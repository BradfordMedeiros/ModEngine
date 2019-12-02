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
    if (!state.isSelectionMode || state.isRotateSelection){
      rotateCamera(xoffset, yoffset);
    }else{
      state.cursorLeft += (int)(xoffset * 15);
      state.cursorTop  -= (int)(yoffset * 15);
    }
}

void mouse_button_callback(bool disableInput, GLFWwindow* window, engineState& state, int button, int action, int mods,
  void (*handleSerialization) (void), void (*selectItem) (void)){
  if (disableInput){
    return;
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
    state.enableManipulator = true;
    handleSerialization();
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
    state.enableManipulator = false;
    handleSerialization();
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
      
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
    selectItem();
  }

}

void handleInput(bool disableInput, GLFWwindow *window, float deltaTime, 
  engineState& state, 
	void (*translate)(float, float, float), void (*scale)(float, float, float), void (*rotate)(float, float, float),
  void (*moveCamera)(glm::vec3), void (*nextCamera)(void),
  void (*playSound)(void),
  void (*setObjectDimensions)(short index, float width, float height, float depth),
  void sendMoveObjectMessage(),
  void (*makeObject)(std::string name, std::string meshName, float x, float y, float z)
){
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
    glfwSetWindowShouldClose(window, true);
  }
  if (disableInput){    // we return after escape, so escape still quits
    return;
  }

  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS){
     makeObject("testobject", "./res/models/cone/cone.obj", 0, 10, 0);
  }
  if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS){
    //sendMoveObjectMessage();
  }
  if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS){
    setObjectDimensions(1, 10, 5, 10);
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
    moveCamera(glm::vec3(0.0, 0.0, -1.0f));
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
    moveCamera(glm::vec3(-1.0, 0.0, 0.0));
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){ 
    moveCamera(glm::vec3(0.0, 0.0, 1.0f));
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){ 
    moveCamera(glm::vec3(1.0, 0.0, 0.0f));
  }
  if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
    nextCamera();
  }
   
  if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS){
    state.visualizeNormals = !state.visualizeNormals;
    std::cout << "visualizeNormals: " << state.visualizeNormals << std::endl;
  }
  if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS){
    state.showCameras = !state.showCameras;
  }
  if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS){
    state.useDefaultCamera = !state.useDefaultCamera;
    std::cout << "Camera option: " << (state.useDefaultCamera ? "default" : "new") << std::endl;
  }
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
    state.moveRelativeEnabled = !state.moveRelativeEnabled;
    std::cout << "Move relative: " << state.moveRelativeEnabled << std::endl;
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
  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
    state.manipulatorAxis = XAXIS;
  }
  if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS){
    state.manipulatorAxis = YAXIS;
  }
  if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS){
    state.manipulatorAxis = ZAXIS;
  }

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
    if (state.mode == 0){
      translate(0.1, 0, 0);
    }else if (state.mode == 1){
      scale(0.1, 0, 0);
    }else if (state.mode == 2){
      rotate(0.1, 0, 0);
    }
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
    if (state.mode == 0){
      translate(-0.1, 0, 0);
    }else if (state.mode == 1){
      scale(-0.1, 0, 0);
    }else if (state.mode == 2){
      rotate(-0.1, 0, 0);
    }    
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
    if (state.mode == 0){
      if (state.axis == 0){
        translate(0, 0, -0.1);
      }else{
        translate(0, -0.1, 0);
      }
    }else if (state.mode == 1){
      if (state.axis == 0){
        scale(0, 0, -0.1);
      }else{
        scale(0, -0.1, 0);
      }
    }else if (state.mode == 2){
      if (state.axis == 0){
        rotate(0, 0, -0.1);
      }else{
        rotate(0, -0.1, 0);
      }
    }    
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
    if (state.mode == 0){
      if (state.axis == 0){
        translate(0, 0, 0.1);
      }else{
        translate(0, 0.1, 0);
      }
    }else if (state.mode == 1){
      if (state.axis == 0){
      scale(0, 0, 0.1);
	  }else{
	    scale(0, 0.1, 0);
	  }   
	}else if (state.mode == 2){
	  if (state.axis == 0){
	    rotate(0, 0, 0.1);
	  }else{
	    rotate(0, 0.1, 0);
      }
    }
  }
}
