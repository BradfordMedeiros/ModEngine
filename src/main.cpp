#include <iostream>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

void handleInput(GLFWwindow *window){
   if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
     	glfwSetWindowShouldClose(window, true);
   }
}

int main(){
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(800, 600, "ModEngine", NULL, NULL);
  if (window == NULL){
    std::cerr << "Error: failed to create window" << std::endl;
    glfwTerminate();
    return -1;
  } 
  glfwMakeContextCurrent(window); 
  
  while (!glfwWindowShouldClose(window)){
    handleInput(window);
    glfwPollEvents();
  } 
  return 0;
}
