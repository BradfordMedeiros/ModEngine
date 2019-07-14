#include <iostream>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

unsigned int currentScreenWidth = INITIAL_SCREEN_WIDTH;
unsigned int currentScreenHeight = INITIAL_SCREEN_HEIGHT;

glm::mat4 projection;

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

 
  GLFWwindow* window = glfwCreateWindow(currentScreenWidth, currentScreenHeight, "ModEngine", NULL, NULL);
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
  glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f)); 
  
  onFramebufferSizeChange(window, currentScreenWidth, currentScreenHeight); 

  glUseProgram(shaderProgram); 

  VAOPointer vaopointer = loadMesh();

  glfwSetCursorPosCallback(window, onMouseEvents); 
  while (!glfwWindowShouldClose(window)){
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),  1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));    

    handleInput(window);
    glfwPollEvents();
    glfwSwapBuffers(window);
    glClearColor(0.1, 0.1, 0.1, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    drawMesh(vaopointer);
   
    model = glm::rotate(model, glm::radians(1.0f), glm::vec3(0.0, 1.0, 0.0));
  }

  std::cout << "LIFECYCLE: program exiting" << std::endl;
  glfwTerminate(); 
  return 0;
}
