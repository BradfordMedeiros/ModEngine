#include <iostream>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "./shaders.h"
#include "./options.h"
#include "./mesh.h"

#define INITIAL_SCREEN_WIDTH 800
#define INITIAL_SCREEN_HEIGHT 600

void handleInput(GLFWwindow *window){
   if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
     	glfwSetWindowShouldClose(window, true);
   }
}
void onMouseEvents(GLFWwindow* window, double xPos, double yPos){
   std::cout << "CONTROL: mouse: xpos(" << xPos << ") yPos(" << yPos << ")" << std::endl; 
}
void onFramebufferSizeChange(GLFWwindow* window, int width, int height){
  std::cout << "EVENT: framebuffer resized:  new size-  " << "width("<< width << ")" << " height(" << height << ")" << std::endl; 
  glViewport(0, 0, width, height);
}

int main(int argc, char* argv[]){
  if (argc < 2){
    std::cerr << "please provide texture file location" << std::endl;
    return -1;
  }
  options opts = loadOptions(argv[1]);

 
  std::cout << "LIFECYCLE: program starting" << std::endl;
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

 
  GLFWwindow* window = glfwCreateWindow(INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT, "ModEngine", NULL, NULL);
  if (window == NULL){
    std::cerr << "ERROR: failed to create window" << std::endl;
    glfwTerminate();
    return -1;
  }
  
  glfwMakeContextCurrent(window); 
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
     std::cerr << "ERROR: failed to load opengl functions" << std::endl;
     glfwTerminate();
     return -1;
  }
 
  glfwSetFramebufferSizeCallback(window, onFramebufferSizeChange); 
  glViewport(0, 0, INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT); 
 
  std:: cout << "shader file path is " << opts.shaderFolderPath << std::endl;
  unsigned int shaderProgram = loadShader(opts.shaderFolderPath + "/vertex.glsl", opts.shaderFolderPath + "/fragment.glsl");

  glUseProgram(shaderProgram); 
  VAOPointer vaopointer = loadMesh();

  glfwSetCursorPosCallback(window, onMouseEvents); 
  while (!glfwWindowShouldClose(window)){
    handleInput(window);
    glfwPollEvents();
    glfwSwapBuffers(window);
    glClearColor(0.1, 0.1, 0.1, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    drawMesh(vaopointer);
  }

  std::cout << "LIFECYCLE: program exiting" << std::endl;
  glfwTerminate(); 
  return 0;
}
