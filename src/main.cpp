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

float quadVertices[] = {
  -1.0f,  1.0f,  0.0f, 1.0f,
  -1.0f, -1.0f,  0.0f, 0.0f,
   1.0f, -1.0f,  1.0f, 0.0f,

  -1.0f,  1.0f,  0.0f, 1.0f,
   1.0f, -1.0f,  1.0f, 0.0f,
   1.0f,  1.0f,  1.0f, 1.0f
};

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

unsigned int framebufferTexture;
unsigned int rbo;

int main(int argc, char* argv[]){
  options opts = loadOptions(argc, argv);

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

  unsigned int fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);  

  glGenTextures(1, &framebufferTexture);
  glBindTexture(GL_TEXTURE_2D, framebufferTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, currentScreenWidth, currentScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
  
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, currentScreenWidth, currentScreenHeight);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);  
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 
 
  if (!glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE){
    std::cerr << "ERROR: framebuffer incomplete" << std::endl;
    return -1;
  }
  unsigned int quadVAO, quadVBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0); 
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  
  auto onFramebufferSizeChange = [](GLFWwindow* window, int width, int height) -> void {
     std::cout << "EVENT: framebuffer resized:  new size-  " << "width("<< width << ")" << " height(" << height << ")" << std::endl;
     currentScreenWidth = width;
     currentScreenHeight = height;
     glBindTexture(GL_TEXTURE_2D, framebufferTexture);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, currentScreenWidth, currentScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  
     glBindRenderbuffer(GL_RENDERBUFFER, rbo);
     glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, currentScreenWidth, currentScreenHeight);
     glViewport(0, 0, currentScreenWidth, currentScreenHeight);
     projection = glm::perspective(glm::radians(45.0f), (float)currentScreenWidth / currentScreenHeight, 0.1f, 100.0f); 
  }; 

  onFramebufferSizeChange(window, currentScreenWidth, currentScreenHeight);
  glfwSetFramebufferSizeCallback(window, onFramebufferSizeChange); 
  
  std::cout << "INFO: shader file path is " << opts.shaderFolderPath << std::endl;
  unsigned int shaderProgram = loadShader(opts.shaderFolderPath + "/vertex.glsl", opts.shaderFolderPath + "/fragment.glsl");
  
  std::cout << "INFO: framebuffer file path is " << opts.framebufferShaderPath << std::endl;
  unsigned int framebufferProgram = loadShader(opts.framebufferShaderPath + "/vertex.glsl", opts.framebufferShaderPath + "/fragment.glsl");

  glm::mat4 model = glm::mat4(1.0f);
  
  onFramebufferSizeChange(window, currentScreenWidth, currentScreenHeight); 
  Mesh mesh = loadMesh(opts.texturePath);

  glfwSetCursorPosCallback(window, onMouseEvents); 
  
  while (!glfwWindowShouldClose(window)){
    handleInput(window);
    glfwPollEvents();
    glfwSwapBuffers(window);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glUseProgram(shaderProgram);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 view = cam.renderView();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),  1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));    

    glClearColor(0.1, 0.1, 0.1, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (unsigned int i = 0; i < 10; i++){
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(model, glm::vec3(i * 1.5f, 0.0f, 0.0f))));
      drawMesh(mesh);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(framebufferProgram); 
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
  }

  std::cout << "LIFECYCLE: program exiting" << std::endl;
  glfwTerminate(); 
  return 0;
}
