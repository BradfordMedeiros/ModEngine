#include <iostream>
#include <vector>
#include <thread>

#include <cxxopts.hpp>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>


#include "./scene/scene.h"
#include "./scene/common/mesh.h"
#include "./scene/common/util/loadmodel.h"
#include "./scene/sprites/readfont.h"
#include "./scene/sprites/sprites.h"
#include "./shaders.h"
#include "./camera.h"
#include "./sound.h"
#include "./common/util.h"
#include "./guile.h"
#include "./object_types.h"
#include "./colorselection.h"
#include "./state.h"

void renderScene(Scene& scene, GLint shaderProgram, glm::mat4 projection, glm::mat4 view,  glm::mat4 model, bool useSelectionColor);
void renderUI(GLint uiShaderProgram, Mesh& crosshairSprite, unsigned int currentFramerate);


engineState state = getDefaultState(800, 600);

Scene scene;
std::map<std::string, Mesh> meshes;
std::map<unsigned int, Mesh> fontMeshes;
std::map<short, GameObjectObj> objectMapping = getObjectMapping();

GameObject* activeCameraObj;
GameObject defaultCamera = GameObject {
  .id = -1,
  .name = "defaultCamera",
  .position = glm::vec3(-8.0f, 4.0f, -8.0f),
  .scale = glm::vec3(1.0f, 1.0f, 1.0f),
  .rotation = glm::quat(0, 1, 0, 0.0f),
};
void nextCamera(){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(objectMapping);
  if (cameraIndexs.size() == 0){  // if we do not have a camera in the scene, we use default
    state.useDefaultCamera = true;    
    activeCameraObj = NULL;
  }

  state.activeCamera = (state.activeCamera + 1) % cameraIndexs.size();
  short activeCameraId = cameraIndexs[state.activeCamera];
  activeCameraObj = &scene.idToGameObjects[activeCameraId];
  state.selectedIndex = activeCameraId;
  std::cout << "active camera is: " << state.activeCamera << std::endl;
}

glm::mat4 projection;
unsigned int framebufferTexture;
unsigned int rbo;
glm::mat4 orthoProj;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

void translate(float x, float y, float z){
  auto offset = glm::vec3(x,y,z);
  if (state.moveRelativeEnabled){
    auto oldGameObject = scene.idToGameObjects[state.selectedIndex];
    scene.idToGameObjects[state.selectedIndex].position = moveRelative(oldGameObject.position, oldGameObject.rotation, offset);
  }else{
    scene.idToGameObjects[state.selectedIndex].position = move(scene.idToGameObjects[state.selectedIndex].position, offset);   
  }
}
void scale(float x, float y, float z){
  scene.idToGameObjects[state.selectedIndex].scale.x+= x;
  scene.idToGameObjects[state.selectedIndex].scale.y+= y;
  scene.idToGameObjects[state.selectedIndex].scale.z+=z;
}
void rotate(float x, float y, float z){
  scene.idToGameObjects[state.selectedIndex].rotation  = setFrontDelta(scene.idToGameObjects[state.selectedIndex].rotation, x, y, z, 5);
}

ALuint soundBuffer;
void handleInput(GLFWwindow *window){
   float currentFrame = glfwGetTime();
   deltaTime = currentFrame - lastFrame;
   lastFrame = currentFrame;

   if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
      glfwSetWindowShouldClose(window, true);
   }
   if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
      defaultCamera.position = moveRelative(defaultCamera.position, defaultCamera.rotation, glm::vec3(0.0, 0.0, 1.0f));
   }
   if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
      defaultCamera.position = moveRelative(defaultCamera.position, defaultCamera.rotation, glm::vec3(1.0, 0.0, 0.0f));
   }
   if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){ 
      defaultCamera.position = moveRelative(defaultCamera.position, defaultCamera.rotation, glm::vec3(0.0, 0.0, -1.0f));
   }
   if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){ 
      defaultCamera.position = moveRelative(defaultCamera.position, defaultCamera.rotation, glm::vec3(-1.0, 0.0, 0.0f));
   }
   if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS){
      playSound(soundBuffer);
   }
   if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
      nextCamera();
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
   if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS){
      state.visualizeNormals = !state.visualizeNormals;
      std::cout << "visualizeNormals: " << state.visualizeNormals << std::endl;
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
        playSound(soundBuffer);
        std::cout << serializeScene(scene, [](short objectId)-> std::vector<std::pair<std::string, std::string>> {
          return getAdditionalFields(objectId, objectMapping);
        }) << std::endl;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE){
      if (action == GLFW_PRESS){
        state.isRotateSelection = true;
      }else if (action == GLFW_RELEASE){
        state.isRotateSelection = false;
      }
      state.cursorLeft = state.currentScreenWidth / 2;
      state.cursorTop = state.currentScreenHeight / 2;
    }
        
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
      Color pixelColor = getPixelColor(state.cursorLeft, state.cursorTop, state.currentScreenHeight);
      state.selectedIndex = getIdFromColor(pixelColor.r, pixelColor.g, pixelColor.b);
      state.selectedName = scene.idToGameObjects[state.selectedIndex].name;
      std::cout << "(" << state.cursorLeft << "," << state.cursorTop << ")" << std::endl;
      std::cout << "Info: Pixel color selection: (  " << pixelColor.r << " , " << pixelColor.g << " , " << pixelColor.b << "  )" << std::endl;
      std::cout << "selected object: " << state.selectedName << std::endl;
      state.additionalText = "     <" + std::to_string((int)(255 * pixelColor.r)) + ","  + std::to_string((int)(255 * pixelColor.g)) + " , " + std::to_string((int)(255 * pixelColor.b)) + ">  " + " --- " + state.selectedName;
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
    if (!state.isSelectionMode || state.isRotateSelection){
      defaultCamera.rotation = setFrontDelta(defaultCamera.rotation, xoffset, yoffset, 0, 1);
    }else{
      state.cursorLeft += (int)(xoffset * 15);
      state.cursorTop  -= (int)(yoffset * 15);
    }
}


SCM moveCamera(SCM value){
  //cam.moveRight(scm_to_double(value));
  return SCM_UNSPECIFIED;
}


int main(int argc, char* argv[]){
  initGuile();
  std::thread shellThread(startShellForNewThread);
  registerFunction("movecamera", moveCamera);

  cxxopts::Options cxxoption("ModEngine", "ModEngine is a game engine for hardcore fps");
  cxxoption.add_options()
   ("s,shader", "Folder path of default shader", cxxopts::value<std::string>()->default_value("./res/shaders/default"))
   ("t,texture", "Image to use as default texture", cxxopts::value<std::string>()->default_value("./res/textures/wood.jpg"))
   ("f,framebuffer", "Folder path of framebuffer", cxxopts::value<std::string>()->default_value("./res/shaders/framebuffer"))
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

 
  GLFWwindow* window = glfwCreateWindow(state.currentScreenWidth, state.currentScreenHeight, "ModEngine", NULL, NULL);
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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, state.currentScreenWidth, state.currentScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
  
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, state.currentScreenWidth, state.currentScreenHeight);
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
     state.currentScreenWidth = width;
     state.currentScreenHeight = height;
     glBindTexture(GL_TEXTURE_2D, framebufferTexture);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, state.currentScreenWidth, state.currentScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  
     glBindRenderbuffer(GL_RENDERBUFFER, rbo);
     glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, state.currentScreenWidth, state.currentScreenHeight);
     glViewport(0, 0, state.currentScreenWidth, state.currentScreenHeight);
     projection = glm::perspective(glm::radians(45.0f), (float)state.currentScreenWidth / state.currentScreenHeight, 0.1f, 200.0f); 
     orthoProj = glm::ortho(0.0f, (float)state.currentScreenWidth, (float)state.currentScreenHeight, 0.0f, -1.0f, 1.0f);  
  }; 

  onFramebufferSizeChange(window, state.currentScreenWidth, state.currentScreenHeight);
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

  font fontToRender = readFont(result["font"].as<std::string>());
  fontMeshes = loadFontMeshes(fontToRender);
  Mesh crosshairSprite = loadSpriteMesh(result["crosshair"].as<std::string>());

  scene = deserializeScene(loadFile("./res/scenes/example.rawscene"), [](short id, std::string type, std::string field, std::string payload) -> void {
    addObject(id, type, field, payload, objectMapping, meshes, "./res/models/box/box.obj", [](std::string meshName) -> void {
      meshes[meshName] = loadMesh(meshName);
    });
  }, fields);

  glfwSetCursorPosCallback(window, onMouseEvents); 
  glfwSetMouseButtonCallback(window, mouse_button_callback);

  unsigned int frameCount = 0;
  float previous = glfwGetTime();
  float last60 = previous;

  unsigned int currentFramerate = 100;
  std::cout << "INFO: render loop starting" << std::endl;

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
 
    glm::mat4 view;
    if (state.useDefaultCamera){
      view = renderView(defaultCamera.position, defaultCamera.rotation);
    }else{
      view = renderView(activeCameraObj->position, activeCameraObj->rotation);
    }

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
    renderUI(uiShaderProgram, crosshairSprite, currentFramerate);

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


void drawGameobject(GameObjectH objectH, Scene& scene, GLint shaderProgram, glm::mat4 model, bool useSelectionColor){
  GameObject object = scene.idToGameObjects[objectH.id];

  glm::mat4 modelMatrix = glm::translate(model, object.position);
  modelMatrix = modelMatrix * glm::toMat4(object.rotation) ;

  glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(getColorFromGameobject(object, useSelectionColor, state.selectedIndex == object.id)));
  
  if (state.visualizeNormals){
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    drawMesh(meshes["./res/models/cone/cone.obj"]); 
  }

  modelMatrix = glm::scale(modelMatrix, object.scale);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
  
  renderObject(objectH.id, objectMapping, meshes["./res/models/box/box.obj"], state.showCameras);

  for (short id: objectH.children){
    drawGameobject(scene.idToGameObjectsH[id], scene, shaderProgram, modelMatrix, useSelectionColor);
  }
}

void renderScene(Scene& scene, GLint shaderProgram, glm::mat4 projection, glm::mat4 view,  glm::mat4 model, bool useSelectionColor){
  glUseProgram(shaderProgram);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));    
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),  1, GL_FALSE, glm::value_ptr(view));

  for (unsigned int i = 0; i < scene.rootGameObjectsH.size(); i++){
    drawGameobject(scene.idToGameObjectsH[scene.rootGameObjectsH[i]], scene, shaderProgram, model, useSelectionColor);
  }  
}

void renderUI(GLint uiShaderProgram, Mesh& crosshairSprite, unsigned int currentFramerate){
    glUseProgram(uiShaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(orthoProj)); 

    if (!state.isSelectionMode){
      drawSpriteAround(uiShaderProgram, crosshairSprite, state.currentScreenWidth/2, state.currentScreenHeight/2, 40, 40);
    }else if (!state.isRotateSelection){
      drawSpriteAround(uiShaderProgram, crosshairSprite, state.cursorLeft, state.cursorTop, 20, 20);
    }

    drawWords(uiShaderProgram, fontMeshes, std::to_string(currentFramerate) + state.additionalText, 10, 20, 4);

    std::string modeText = state.mode == 0 ? "translate" : (state.mode == 1 ? "scale" : "rotate"); 
    std::string axisText = state.axis == 0 ? "xz" : "xy";
    drawWords(uiShaderProgram, fontMeshes, "Mode: " + modeText + " Axis: " + axisText, 10, 40, 3);
}