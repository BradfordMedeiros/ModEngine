#include <iostream>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

int main(){
  std::cout << "hello world" << std::endl; 

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

  return 0;
}
