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
#include "./scheme/scheme_bindings.h"
#include "./shaders.h"
#include "./camera.h"
#include "./sound.h"
#include "./common/util.h"
#include "./object_types.h"
#include "./colorselection.h"
#include "./state.h"
#include "./input.h"

GameObject* activeCameraObj;
GameObject defaultCamera = GameObject {
  .id = -1,
  .name = "defaultCamera",
  .position = glm::vec3(-8.0f, 4.0f, -8.0f),
  .scale = glm::vec3(1.0f, 1.0f, 1.0f),
  .rotation = glm::quat(0, 1, 0, 0.0f),
};
engineState state = getDefaultState(800, 600);
Scene scene;
std::map<std::string, Mesh> meshes;
std::map<unsigned int, Mesh> fontMeshes;
std::map<short, GameObjectObj> objectMapping = getObjectMapping();

glm::mat4 projection;
unsigned int framebufferTexture;
unsigned int rbo;
glm::mat4 orthoProj;
ALuint soundBuffer;

float quadVertices[] = {
  -1.0f,  1.0f,  0.0f, 1.0f,
  -1.0f, -1.0f,  0.0f, 0.0f,
   1.0f, -1.0f,  1.0f, 0.0f,

  -1.0f,  1.0f,  0.0f, 1.0f,
   1.0f, -1.0f,  1.0f, 0.0f,
   1.0f,  1.0f,  1.0f, 1.0f
};


void setActiveCamera(short cameraId){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(objectMapping);
  if (! (std::find(cameraIndexs.begin(), cameraIndexs.end(), cameraId) != cameraIndexs.end())){
    std::cout << "index: " << cameraId << " is not a valid index" << std::endl;
    return;
  }
  activeCameraObj = &scene.idToGameObjects[cameraId];
  state.selectedIndex = cameraId;
}
void nextCamera(){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(objectMapping);
  if (cameraIndexs.size() == 0){  // if we do not have a camera in the scene, we use default
    state.useDefaultCamera = true;    
    activeCameraObj = NULL;
  }

  state.activeCamera = (state.activeCamera + 1) % cameraIndexs.size();
  short activeCameraId = cameraIndexs[state.activeCamera];
  setActiveCamera(activeCameraId);
  std::cout << "active camera is: " << state.activeCamera << std::endl;
}

void moveCamera(glm::vec3 offset){
  defaultCamera.position = moveRelative(defaultCamera.position, defaultCamera.rotation, glm::vec3(offset));
}
void rotateCamera(float xoffset, float yoffset){
  defaultCamera.rotation = setFrontDelta(defaultCamera.rotation, xoffset, yoffset, 0, 1);
}

void playSound(){
  playSound(soundBuffer);
}
void handleSerialization(){
  playSound();
  std::cout << serializeScene(scene, [](short objectId)-> std::vector<std::pair<std::string, std::string>> {
    return getAdditionalFields(objectId, objectMapping);
  }) << std::endl; 
}
void selectItem(){
  Color pixelColor = getPixelColor(state.cursorLeft, state.cursorTop, state.currentScreenHeight);
  state.selectedIndex = getIdFromColor(pixelColor.r, pixelColor.g, pixelColor.b);
  state.selectedName = scene.idToGameObjects[state.selectedIndex].name;
  std::cout << "(" << state.cursorLeft << "," << state.cursorTop << ")" << std::endl;
  std::cout << "Info: Pixel color selection: (  " << pixelColor.r << " , " << pixelColor.g << " , " << pixelColor.b << "  )" << std::endl;
  std::cout << "selected object: " << state.selectedName << std::endl;
  state.additionalText = "     <" + std::to_string((int)(255 * pixelColor.r)) + ","  + std::to_string((int)(255 * pixelColor.g)) + " , " + std::to_string((int)(255 * pixelColor.b)) + ">  " + " --- " + state.selectedName;
}

void onMouseEvents(GLFWwindow* window, double xpos, double ypos){
  onMouse(window, state, xpos, ypos, rotateCamera);
}
void onMouseCallback(GLFWwindow* window, int button, int action, int mods){
  mouse_button_callback(window, state, button, action, mods, handleSerialization, selectItem);
}

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

void removeObjectById(short id){
  std::cout << "removing object by id: " << id << std::endl;
  removeObject(objectMapping, id);
  removeObjectFromScene(scene, id);
}
void makeObject(float x, float y, float z){

}

std::vector<short> getObjectsByType(std::string type){
  if (type == "mesh"){
    std::vector indexes = getGameObjectsIndex<GameObjectMesh>(objectMapping);
    return indexes;
  }else if (type == "camera"){
    std::vector indexes = getGameObjectsIndex<GameObjectCamera>(objectMapping);
    return indexes;
  }

  std::vector<short> defaultVec;
  return defaultVec;
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

int main(int argc, char* argv[]){
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
    
  createStaticSchemeBindings(moveCamera, rotateCamera, removeObjectById, makeObject, getObjectsByType, setActiveCamera);
  std::thread shellThread(startShell);

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

  font fontToRender = readFont(result["font"].as<std::string>());
  fontMeshes = loadFontMeshes(fontToRender);
  Mesh crosshairSprite = loadSpriteMesh(result["crosshair"].as<std::string>());

  scene = deserializeScene(loadFile("./res/scenes/example.rawscene"), [](short id, std::string type, std::string field, std::string payload) -> void {
    addObject(id, type, field, payload, objectMapping, meshes, "./res/models/box/box.obj", [](std::string meshName) -> void {
      meshes[meshName] = loadMesh(meshName);
    });
  }, fields);


  glfwSetCursorPosCallback(window, onMouseEvents); 
  glfwSetMouseButtonCallback(window, onMouseCallback);
  
  float deltaTime = 0.0f; // Time between current frame and last frame

  unsigned int frameCount = 0;
  float previous = glfwGetTime();
  float last60 = previous;

  unsigned int currentFramerate = 0;
  std::cout << "INFO: render loop starting" << std::endl;

  while (!glfwWindowShouldClose(window)){
    frameCount++;
    float now = glfwGetTime();
    deltaTime = now - previous;   
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
    renderScene(scene, selectionProgram, projection, view, glm::mat4(1.0f), true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(framebufferProgram); 
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
        
    handleInput(window, deltaTime, state, translate, scale, rotate, moveCamera, nextCamera, playSound);

    glfwPollEvents();
    
    // 2ND pass renders what we care about to the screen.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1, 0.1, 0.1, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderScene(scene, shaderProgram, projection, view, glm::mat4(1.0f), false);
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