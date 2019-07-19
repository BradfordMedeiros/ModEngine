#include <iostream>
#include <vector>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "./shaders.h"
#include "./options.h"
#include "./mesh.h"
#include "./camera.h"

#define INITIAL_SCREEN_WIDTH 800
#define INITIAL_SCREEN_HEIGHT 600

unsigned int currentScreenWidth = INITIAL_SCREEN_WIDTH;
unsigned int currentScreenHeight = INITIAL_SCREEN_HEIGHT;

glm::mat4 projection;

Camera cam(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0, 1.0f, 0.0f), 0.2f, 0.0f, 0.0f);

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

void handleInput(GLFWwindow *window){
   float currentFrame = glfwGetTime();
   deltaTime = currentFrame - lastFrame;
   lastFrame = currentFrame;  

   if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
      glfwSetWindowShouldClose(window, true);
   }
   if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
      cam.moveFront();
   }
   if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
      cam.moveLeft();
   }
   if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){ 
      cam.moveBack();
   }
   if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){ 
      cam.moveRight();
   }
}   


bool firstMouse = true;
float lastX, lastY;
void onMouseEvents(GLFWwindow* window, double xpos, double ypos){
    if(firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }
  
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    cam.setFrontDelta(xoffset, yoffset);
}

int main(int argc, char* argv[]){
  if (argc < 3){
    std::cerr << "please provide shader + texture file location" << std::endl;
    return -1;
  }
  options opts = loadOptions(argv);

  std::cout << "LIFECYCLE: program starting" << std::endl;
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

 
  GLFWwindow* window = glfwCreateWindow(currentScreenWidth, currentScreenHeight, "ModEngine", NULL, NULL);
  if (window == NULL){
    std::cerr << "ERROR: failed to create window" << std::endl;
    glfwTerminate();
    return -1;
  }
  
  glfwMakeContextCurrent(window);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
 
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
     std::cerr << "ERROR: failed to load opengl functions" << std::endl;
     glfwTerminate();
     return -1;
  }
 
  auto onFramebufferSizeChange = [](GLFWwindow* window, int width, int height) -> void {
     std::cout << "EVENT: framebuffer resized:  new size-  " << "width("<< width << ")" << " height(" << height << ")" << std::endl;
     currentScreenWidth = width;
     currentScreenHeight = height;
     glViewport(0, 0, currentScreenWidth, currentScreenHeight);
     projection = glm::perspective(glm::radians(45.0f), (float)currentScreenWidth / currentScreenHeight, 0.1f, 100.0f); 
  }; 

  onFramebufferSizeChange(window, currentScreenWidth, currentScreenHeight);
  glfwSetFramebufferSizeCallback(window, onFramebufferSizeChange); 
  
  std:: cout << "shader file path is " << opts.shaderFolderPath << std::endl;
  unsigned int shaderProgram = loadShader(opts.shaderFolderPath + "/vertex.glsl", opts.shaderFolderPath + "/fragment.glsl");

  glm::mat4 model = glm::mat4(1.0f);
  
  onFramebufferSizeChange(window, currentScreenWidth, currentScreenHeight); 

  glUseProgram(shaderProgram); 
  glEnable(GL_DEPTH_TEST);
  VAOPointer vaopointer = loadMesh(opts.texturePath);

  glfwSetCursorPosCallback(window, onMouseEvents); 
  
  while (!glfwWindowShouldClose(window)){
    glm::mat4 view = cam.renderView();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),  1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));    


    handleInput(window);
    glfwPollEvents();
    glfwSwapBuffers(window);
    glClearColor(0.1, 0.1, 0.1, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (unsigned int i = 0; i < 10; i++){
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(model, glm::vec3(i * 1.5f, 0.0f, 0.0f))));
      drawMesh(vaopointer);
    }
  }

  std::cout << "LIFECYCLE: program exiting" << std::endl;
  glfwTerminate(); 
  return 0;
}
