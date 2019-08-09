#include <iostream>
#include <vector>

#include <cxxopts.hpp>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "./shaders.h"
#include "./mesh.h"
#include "./camera.h"
#include "./sound.h"
#include "./loadmodel.h"
#include "./readfont.h"

#define INITIAL_SCREEN_WIDTH 800
#define INITIAL_SCREEN_HEIGHT 600

unsigned int currentScreenWidth = INITIAL_SCREEN_WIDTH;
unsigned int currentScreenHeight = INITIAL_SCREEN_HEIGHT;

glm::mat4 projection;

Camera cam(glm::vec3(-8.0f, 4.0f, -8.0f), glm::vec3(0.0, 1.0f, 0.0f), 0.2f, -20.0f, 30.0f);

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

ALuint soundBuffer;
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
   if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS){
      playSound(soundBuffer);
   }
}   

std::map<unsigned int, Mesh> fontMeshes;
Mesh currentCharacter;
void keycallback(GLFWwindow* window, unsigned int codepoint){
      currentCharacter = fontMeshes[codepoint];

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

glm::mat4 orthoProj;

void drawSprite(GLint shaderProgram, Mesh mesh, float left, float top, float width, float height){
  glm::mat4 modelMatrix = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(left, top, 0)), glm::vec3(width * 1.0f, height * 1.0f, 1.0f)); 
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
  drawMesh(mesh);
}
void drawSpriteAround(GLint shaderProgram, Mesh mesh, float centerX, float centerY, float width, float height){
  drawSprite(shaderProgram, mesh, centerX - width/2, centerY - height/2, width, height);
}

int main(int argc, char* argv[]){
  cxxopts::Options cxxoption("ModEngine", "ModEngine is a game engine for hardcore fps");
  cxxoption.add_options()
   ("s,shader", "Folder path of default shader", cxxopts::value<std::string>()->default_value("./res/shaders/default"))
   ("t,texture", "Image to use as default texture", cxxopts::value<std::string>()->default_value("./res/textures/wood.jpg"))
   ("f,framebuffer", "Folder path of framebuffer", cxxopts::value<std::string>()->default_value("./res/shaders/framebuffer"))
   ("m,model", "Model to load", cxxopts::value<std::string>()->default_value("./res/models/column_seat/column_seat.obj"))
   ("b,modelbox", "Second model to load", cxxopts::value<std::string>()->default_value("./res/models/box/box.obj"))
   ("d,twodee", "Image to use for texture for 2d mesh", cxxopts::value<std::string>()->default_value("./res/textures/grass.png"))
   ("e,twoshader", "Shader to use for 2d mesh", cxxopts::value<std::string>()->default_value("./res/shaders/2d"))
   ("u,uishader", "Shader to use for ui", cxxopts::value<std::string>()->default_value("./res/shaders/ui"))
   ("c,crosshair", "icon to use for crosshair", cxxopts::value<std::string>()->default_value("./res/textures/crosshairs/crosshair029.png"))
   ("o,font", "font to use", cxxopts::value<std::string>()->default_value("./res/textures/fonts/gamefont"))
   ("h,help", "Print help")
  ;   

  const auto result = cxxoption.parse(argc, argv);
  if (result["help"].as<bool>()){
    std::cout << cxxoption.help() << std::endl;
    return 0;
  }

  const std::string shaderFolderPath = result["shader"].as<std::string>();
  const std::string texturePath = result["texture"].as<std::string>();
  const std::string framebufferTexturePath = result["framebuffer"].as<std::string>();
  const std::string twodeeShaderPath = result["twoshader"].as<std::string>();
  const std::string uiShaderPath = result["uishader"].as<std::string>();
  
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

  startSoundSystem();
  soundBuffer = loadSound("./res/sounds/sample.wav");

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
     orthoProj = glm::ortho(0.0f, (float)currentScreenWidth, (float)currentScreenHeight, 0.0f, -1.0f, 1.0f);  
  }; 

  onFramebufferSizeChange(window, currentScreenWidth, currentScreenHeight);
  glfwSetFramebufferSizeCallback(window, onFramebufferSizeChange); 
  
  std::cout << "INFO: shader file path is " << shaderFolderPath << std::endl;
  unsigned int shaderProgram = loadShader(shaderFolderPath + "/vertex.glsl", shaderFolderPath + "/fragment.glsl");

  std::cout << "INFO: 2D shader file path is " << twodeeShaderPath << std::endl;
  unsigned int twodeeShaderProgram = loadShader(twodeeShaderPath + "/vertex.glsl", twodeeShaderPath + "/fragment.glsl");
  
  std::cout << "INFO: framebuffer file path is " << framebufferTexturePath << std::endl;
  unsigned int framebufferProgram = loadShader(framebufferTexturePath + "/vertex.glsl", framebufferTexturePath + "/fragment.glsl");

  std::cout << "INFO: ui shader file path is " << uiShaderPath << std::endl;
  unsigned int uiShaderProgram = loadShader(uiShaderPath + "/vertex.glsl",  uiShaderPath + "/fragment.glsl");

  glm::mat4 model = glm::mat4(1.0f);

  onFramebufferSizeChange(window, currentScreenWidth, currentScreenHeight); 

  font fontToRender = readFont(result["font"].as<std::string>());

  Mesh columnSeatMesh = loadMesh(result["model"].as<std::string>());
  Mesh boxMesh = loadMesh(result["modelbox"].as<std::string>());
  Mesh grassMesh = load2DMesh(result["twodee"].as<std::string>());
  Mesh crosshairSprite = loadSpriteMesh(result["crosshair"].as<std::string>());

  for ( const auto &[ascii, font]: fontToRender.chars ) {
    fontMeshes[ascii] = load2DMeshTexCoords(fontToRender.image, font.x, font.y, font.width, font.height);
  }
  currentCharacter = fontMeshes[70];

  glfwSetCursorPosCallback(window, onMouseEvents); 
  
  

  glfwSetCharCallback(window, keycallback);
  while (!glfwWindowShouldClose(window)){
    handleInput(window);

    glfwPollEvents();
    glfwSwapBuffers(window);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glUseProgram(shaderProgram);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1, 0.1, 0.1, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = cam.renderView();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),  1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));    

    for (unsigned int i = 0; i < 5; i++){
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(model, glm::vec3(6.0f + (i * 3.0f), 0.0f, 0.0f))));
      drawMesh(boxMesh);
    }

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f))));
    drawMesh(columnSeatMesh);
    
    glUseProgram(twodeeShaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),  1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection)); 
    
    glm::mat4 grassModel = glm::inverse(glm::lookAt(glm::vec3(5.0f, 1.7f, 1.05f), cam.position, glm::vec3(0.0, 1.0f, 0.0f)));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(grassModel));
    drawMesh(grassMesh);

    glUseProgram(uiShaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(orthoProj)); 
    drawSpriteAround(uiShaderProgram, crosshairSprite, currentScreenWidth/2, currentScreenHeight/2, 40, 40);

    drawSpriteAround(uiShaderProgram, currentCharacter, currentScreenWidth /4 + 200, currentScreenHeight /4 + 200, 100, 100);

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
  stopSoundSystem();
  glfwTerminate(); 
  return 0;
}


