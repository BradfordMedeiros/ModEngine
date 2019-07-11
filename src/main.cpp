#include <iostream>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define INITIAL_SCREEN_WIDTH 800
#define INITIAL_SCREEN_HEIGHT 600

void handleInput(GLFWwindow *window){
   if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
     	glfwSetWindowShouldClose(window, true);
   }
}

void onFramebufferSizeChange(GLFWwindow* window, int width, int height){
  std::cout << "EVENT: framebuffer resized.  new size:  " << "width("<< width << ")" << " height(" << height << ")" << std::endl; 
}

int main(){
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
  glfwSetFramebufferSizeCallback(window, onFramebufferSizeChange); 
  while (!glfwWindowShouldClose(window)){
    handleInput(window);
    glfwPollEvents();
  } 
  return 0;
}
