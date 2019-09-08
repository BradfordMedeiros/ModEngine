#include "./input.h"

void handleInput(GLFWwindow *window, float deltaTime, 
  engineState& state, 
	void (*translate)(float, float, float), void (*scale)(float, float, float), void (*rotate)(float, float, float),
  void (*moveCamera)(glm::vec3), void (*nextCamera)(void),
  void (*playSound)(void)
){
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS){
    playSound();
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
    moveCamera(glm::vec3(0.0, 0.0, 1.0f));
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
    moveCamera(glm::vec3(1.0, 0.0, 0.0));
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){ 
    moveCamera(glm::vec3(0.0, 0.0, -1.0f));
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){ 
    moveCamera(glm::vec3(-1.0, 0.0, 0.0f));
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
  if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS){
    state.isSelectionMode = !state.isSelectionMode;
    state.isRotateSelection = false;
  }

  if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS){
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
