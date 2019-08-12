#include <iostream>
#include <vector>
#include <math.h>     

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
#include "./sprites.h"
#include "./scene.h"

#define INITIAL_SCREEN_WIDTH 800
#define INITIAL_SCREEN_HEIGHT 600

unsigned int currentScreenWidth = INITIAL_SCREEN_WIDTH;
unsigned int currentScreenHeight = INITIAL_SCREEN_HEIGHT;

unsigned int cursorLeft = currentScreenWidth / 4;
unsigned int cursorTop  = currentScreenHeight / 4;
bool isSelectionMode = false;
bool isRotateSelection = false;

Scene scene;
short selectedIndex = -1;

glm::mat4 projection;

Camera cam(glm::vec3(-8.0f, 4.0f, -8.0f), glm::vec3(0.0, 1.0f, 0.0f), 25.0f, 150.0f, -20.0f, 30.0f);

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
      cam.moveFront(deltaTime);
   }
   if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
      cam.moveLeft(deltaTime);
   }
   if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){ 
      cam.moveBack(deltaTime);
   }
   if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){ 
      cam.moveRight(deltaTime);
   }
   if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS){
      playSound(soundBuffer);
   }
   if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS){
      isSelectionMode = !isSelectionMode;
      isRotateSelection = false;
   }

   if (glfwGetKey(window, GLFW_KEY_UP ) == GLFW_PRESS){
      scene.gameObjects[selectedIndex].position.x+=0.1;
   }
   if (glfwGetKey(window, GLFW_KEY_DOWN ) == GLFW_PRESS){
      scene.gameObjects[selectedIndex].position.x-=0.1;
   }
   if (glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS){
      scene.gameObjects[selectedIndex].position.z+=0.1;
   }
   if (glfwGetKey(window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
      scene.gameObjects[selectedIndex].position.z-=0.1;
   }
}   

std::map<unsigned int, Mesh> fontMeshes;
void keycallback(GLFWwindow* window, unsigned int codepoint){
  // this can get the raw text input into the keyboard
}

struct Color {
  GLfloat r;
  GLfloat g;
  GLfloat b;
};
struct GLPosition {
  GLint x;
  GLint y;
};
Color getPixelColor(GLint x, GLint y) {
    Color color;
    glReadPixels(x, currentScreenHeight - y, 1, 1, GL_RGB, GL_FLOAT, &color);
    return color;
}

glm::vec3 getColorFromGameobject(GameObject object, bool useSelectionColor, bool isSelected){
  if (isSelected){
    return glm::vec3(1.0f, 0.0f, 0.0f);
  }
  if (!useSelectionColor){
    return glm::vec3(1.0f, 1.0f, 1.0f);
  }
  float blueChannel = object.id * 0.01;
  return glm::vec3(0.0f, 0.0f, blueChannel);
}
unsigned int getIdFromColor(float r, float g, float b){
  short objectId = round(b / 0.01);
  return objectId;
}


std::string additionalText = "";
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
        playSound(soundBuffer);
        selectedIndex = (selectedIndex + 1) % scene.gameObjects.size();
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE){
      if (action == GLFW_PRESS){
        isRotateSelection = true;
      }else if (action == GLFW_RELEASE){
        isRotateSelection = false;
      }
      cursorLeft = currentScreenWidth / 2;
      cursorTop = currentScreenHeight / 2;
    }
        
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
      Color pixelColor = getPixelColor(cursorLeft, cursorTop);
      selectedIndex = getIdFromColor(pixelColor.r, pixelColor.g, pixelColor.b);
      std::cout << "(" << cursorLeft << "," << cursorTop << ")" << std::endl;
      std::cout << "Info: Pixel color selection: (  " << pixelColor.r << " , " << pixelColor.g << " , " << pixelColor.b << "  )" << std::endl;
      additionalText = "     <" + std::to_string((int)(255 * pixelColor.r)) + ","  + std::to_string((int)(255 * pixelColor.g)) + " , " + std::to_string((int)(255 * pixelColor.b)) + ">  ";
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
    if (!isSelectionMode || isRotateSelection){
      cam.setFrontDelta(xoffset, yoffset, deltaTime);
    }else{
      cursorLeft += (int)(xoffset * 15);
      cursorTop  -= (int)(yoffset * 15);
    }
}
void renderScene(Scene& scene, GLint shaderProgram, glm::mat4 projection, glm::mat4 view,  glm::mat4 model, bool useSelectionColor){
  glUseProgram(shaderProgram);

  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));    
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),  1, GL_FALSE, glm::value_ptr(view));

  for (unsigned int i = 0; i < scene.gameObjects.size(); i++){
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(model, scene.gameObjects[i].position)));
    glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(getColorFromGameobject(scene.gameObjects[i], useSelectionColor, selectedIndex == i)));
    drawMesh(scene.gameObjects[i].mesh);
  }  
  for (unsigned int i = 0; i < scene.rotatingGameObjects.size(); i++){
    glm::mat4 modelMatrix = glm::inverse(glm::lookAt(glm::vec3(5.0f, 1.7f, 1.05f), cam.position, scene.rotatingGameObjects[i].position));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    drawMesh(scene.rotatingGameObjects[i].mesh);
  }  
}

unsigned int framebufferTexture;
unsigned int rbo;

glm::mat4 orthoProj;

unsigned int currentFramerate = 100;

void renderUI(GLint uiShaderProgram, Mesh& crosshairSprite){
    glUseProgram(uiShaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(orthoProj)); 

    if (!isSelectionMode){
      drawSpriteAround(uiShaderProgram, crosshairSprite, currentScreenWidth/2, currentScreenHeight/2, 40, 40);
    }else if (!isRotateSelection){
      drawSpriteAround(uiShaderProgram, crosshairSprite, cursorLeft, cursorTop, 20, 20);
    }

    drawWords(uiShaderProgram, fontMeshes, std::to_string(currentFramerate) + additionalText, 10, 20, 4);
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
  
  std::cout << "INFO: framebuffer file path is " << framebufferTexturePath << std::endl;
  unsigned int framebufferProgram = loadShader(framebufferTexturePath + "/vertex.glsl", framebufferTexturePath + "/fragment.glsl");

  std::cout << "INFO: ui shader file path is " << uiShaderPath << std::endl;
  unsigned int uiShaderProgram = loadShader(uiShaderPath + "/vertex.glsl",  uiShaderPath + "/fragment.glsl");

  std::string selectionShaderPath = "./res/shaders/selection";
  std::cout << "INFO: selection shader path is " << selectionShaderPath << std::endl;
  unsigned int selectionProgram = loadShader(selectionShaderPath + "/vertex.glsl", selectionShaderPath + "/fragment.glsl");

  glm::mat4 model = glm::mat4(1.0f);

  onFramebufferSizeChange(window, currentScreenWidth, currentScreenHeight); 

  font fontToRender = readFont(result["font"].as<std::string>());
  fontMeshes = loadFontMeshes(fontToRender);

  Mesh columnSeatMesh = loadMesh(result["model"].as<std::string>());
  Mesh boxMesh = loadMesh(result["modelbox"].as<std::string>());
  Mesh grassMesh = load2DMesh(result["twodee"].as<std::string>());
  Mesh crosshairSprite = loadSpriteMesh(result["crosshair"].as<std::string>());

  scene = loadScene(columnSeatMesh, boxMesh, grassMesh);

  glfwSetCursorPosCallback(window, onMouseEvents); 
  glfwSetCharCallback(window, keycallback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);

  unsigned int frameCount = 0;
  float previous = glfwGetTime();
  float last60 = previous;

  while (!glfwWindowShouldClose(window)){
    frameCount++;
    float now = glfwGetTime();
    deltaTime = now - previous;     // this should be used 
    previous = now;

    if (frameCount == 60){
      frameCount = 0;
      float timedelta = now - last60;
      last60 = now;
      currentFramerate = (int)60/(timedelta);
    }
 
    glm::mat4 view = cam.renderView();

    glfwSwapBuffers(window);
    
    // 1ST pass draws selection program shader to be able to handle selection 
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1, 0.1, 0.1, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderScene(scene, selectionProgram, projection, view, model, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(framebufferProgram); 
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    handleInput(window);    // input needs to be called in between so read pixels can read old pixel value
    glfwPollEvents();
    
    // 2ND pass renders what we care about to the screen.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1, 0.1, 0.1, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderScene(scene, shaderProgram, projection, view, model, false);
    renderUI(uiShaderProgram, crosshairSprite);

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


