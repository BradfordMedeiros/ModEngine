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
#include "./scene/physics.h"
#include "./scene/scenegraph.h"
#include "./scene/object_types.h"
#include "./scene/common/mesh.h"
#include "./scene/common/util/loadmodel.h"
#include "./scene/common/util/boundinfo.h"
#include "./scene/sprites/readfont.h"
#include "./scene/sprites/sprites.h"
#include "./scheme_bindings.h"
#include "./shaders.h"
#include "./translations.h"
#include "./sound.h"
#include "./common/util.h"
#include "./colorselection.h"
#include "./state.h"
#include "./input.h"
#include "./network.h"

const bool SHELL_ENABLED = false;

GameObject* activeCameraObj;
GameObject defaultCamera = GameObject {
  .id = -1,
  .name = "defaultCamera",
  .position = glm::vec3(-8.0f, 4.0f, -8.0f),
  .scale = glm::vec3(1.0f, 1.0f, 1.0f),
  .rotation = glm::quat(0, 1, 0, 0.0f),
};

bool showDebugInfo = false;
engineState state = getDefaultState(1920, 1080);
FullScene fullscene;
std::map<unsigned int, Mesh> fontMeshes;
std::vector<btRigidBody*> rigidbodies;

glm::mat4 projection;
unsigned int framebufferTexture;
unsigned int rbo;
glm::mat4 orthoProj;
ALuint soundBuffer;
unsigned int uiShaderProgram;

SchemeBindingCallbacks schemeBindings;

float quadVertices[] = {
  -1.0f,  1.0f,  0.0f, 1.0f,
  -1.0f, -1.0f,  0.0f, 0.0f,
   1.0f, -1.0f,  1.0f, 0.0f,

  -1.0f,  1.0f,  0.0f, 1.0f,
   1.0f, -1.0f,  1.0f, 0.0f,
   1.0f,  1.0f,  1.0f, 1.0f
};

void setActiveCamera(short cameraId){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(fullscene.objectMapping);
  if (! (std::find(cameraIndexs.begin(), cameraIndexs.end(), cameraId) != cameraIndexs.end())){
    std::cout << "index: " << cameraId << " is not a valid index" << std::endl;
    return;
  }
  activeCameraObj = &fullscene.scene.idToGameObjects[cameraId];
  state.selectedIndex = cameraId;
}
void nextCamera(){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(fullscene.objectMapping);
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
void drawText(std::string word, float left, float top, unsigned int fontSize){
  drawWords(uiShaderProgram, fontMeshes, word, left, top, fontSize);
}
void playSound(){
  playSound(soundBuffer);
}
void handleSerialization(){
  playSound();
  std::cout << serializeFullScene(fullscene.scene, fullscene.objectMapping) << std::endl;
}
void selectItem(){
  Color pixelColor = getPixelColor(state.cursorLeft, state.cursorTop, state.currentScreenHeight);
  state.selectedIndex = getIdFromColor(pixelColor.r, pixelColor.g, pixelColor.b);
  state.selectedName = fullscene.scene.idToGameObjects[state.selectedIndex].name;
  state.additionalText = "     <" + std::to_string((int)(255 * pixelColor.r)) + ","  + std::to_string((int)(255 * pixelColor.g)) + " , " + std::to_string((int)(255 * pixelColor.b)) + ">  " + " --- " + state.selectedName;
  schemeBindings.onObjectSelected(state.selectedIndex);
}
void processManipulator(){
  if (state.enableManipulator){
    auto selectObject = fullscene.scene.idToGameObjects[state.selectedIndex];
    if (state.manipulatorMode == TRANSLATE){
      fullscene.scene.idToGameObjects[state.selectedIndex].position = applyTranslation(selectObject.position, state.offsetX, state.offsetY, state.manipulatorAxis);
    }else if (state.manipulatorMode == SCALE){
      fullscene.scene.idToGameObjects[state.selectedIndex].scale = applyScaling(selectObject.position, selectObject.scale, state.lastX, state.lastY, state.offsetX, state.offsetY, state.manipulatorAxis);
    }else if (state.manipulatorMode == ROTATE){
      fullscene.scene.idToGameObjects[state.selectedIndex].rotation = applyRotation(selectObject.rotation, state.offsetX, state.offsetY, state.manipulatorAxis);
    }
  }
}

void onMouseEvents(GLFWwindow* window, double xpos, double ypos){
  onMouse(window, state, xpos, ypos, rotateCamera);
  processManipulator();
}
void onMouseCallback(GLFWwindow* window, int button, int action, int mods){
  mouse_button_callback(window, state, button, action, mods, handleSerialization, selectItem);
  schemeBindings.onMouseCallback(button, action, mods);
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
  schemeBindings.onKeyCallback(key, scancode, action, mods);
}
void translate(float x, float y, float z){
  physicsTranslate(fullscene, rigidbodies[state.selectedIndex], x, y, z, state.moveRelativeEnabled, state.selectedIndex);
}
void scale(float x, float y, float z){
  fullscene.scene.idToGameObjects[state.selectedIndex].scale.x+= x;
  fullscene.scene.idToGameObjects[state.selectedIndex].scale.y+= y;
  fullscene.scene.idToGameObjects[state.selectedIndex].scale.z+=z;
}
void rotate(float x, float y, float z){
  physicsRotate(fullscene, rigidbodies[state.selectedIndex], x, y, z, state.selectedIndex);
}

void setObjectDimensions(short index, float width, float height, float depth){
  auto gameObjV = fullscene.objectMapping[state.selectedIndex];  // todo this is bs, need a wrapper around objectmappping + scene
  auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); 
  if (meshObj != NULL){
    auto newScale = getScaleEquivalent(meshObj->mesh.boundInfo, width, height, depth);
    std::cout << "new scale: (" << newScale.x << ", " << newScale.y << ", " << newScale.z << ")" << std::endl;
    fullscene.scene.idToGameObjects[state.selectedIndex].scale = newScale;
  } 
}
void drawGameobject(GameObjectH objectH, Scene& scene, GLint shaderProgram, glm::mat4 model, bool useSelectionColor){
  GameObject object = fullscene.scene.idToGameObjects[objectH.id];
  glm::mat4 modelMatrix = glm::translate(model, object.position);
  modelMatrix = modelMatrix * glm::toMat4(object.rotation) ;

  bool objectSelected = state.selectedIndex == object.id;
  glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(getColorFromGameobject(object, useSelectionColor, objectSelected)));
  
  if (state.visualizeNormals){
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    drawMesh(fullscene.meshes["./res/models/cone/cone.obj"]); 
  }

  modelMatrix = glm::scale(modelMatrix, object.scale);
  
  // bounding code //////////////////////
  auto gameObjV = fullscene.objectMapping[objectH.id];
  auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); // doing this here is absolute bullshit.  fucked up abstraction level render should handle 
  if (meshObj != NULL){
    auto bounding = getBoundRatio(fullscene.meshes["./res/models/boundingbox/boundingbox.obj"].boundInfo, meshObj->mesh.boundInfo);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(getMatrixForBoundRatio(bounding, modelMatrix)));

    if (objectSelected){
      drawMesh(fullscene.meshes["./res/models/boundingbox/boundingbox.obj"]);
    }
  }
  /////////////////////////////// end bounding code

  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
  renderObject(objectH.id, fullscene.objectMapping, fullscene.meshes["./res/models/box/box.obj"], objectSelected, fullscene.meshes["./res/models/boundingbox/boundingbox.obj"], state.showCameras);

  for (short id: objectH.children){
    drawGameobject(fullscene.scene.idToGameObjectsH[id], scene, shaderProgram, modelMatrix, useSelectionColor);
  }
}

void removeObjectById(short id){
  std::cout << "removing object by id: " << id << std::endl;
  removeObject(fullscene.objectMapping, id);
  removeObjectFromScene(fullscene.scene, id);
}
void makeObject(std::string name, std::string meshName, float x, float y, float z){
  addObjectToFullScene(fullscene, name, meshName, glm::vec3(x,y,z));
}

std::vector<short> getObjectsByType(std::string type){
  if (type == "mesh"){
    std::vector indexes = getGameObjectsIndex<GameObjectMesh>(fullscene.objectMapping);
    return indexes;
  }else if (type == "camera"){
    std::vector indexes = getGameObjectsIndex<GameObjectCamera>(fullscene.objectMapping);
    return indexes;
  }
  return getGameObjectsIndex(fullscene.objectMapping);
}

std::string getGameObjectName(short index){
  return fullscene.scene.idToGameObjects[index].name;
}
glm::vec3 getGameObjectPosition(short index){
  return fullscene.scene.idToGameObjects[index].position;
}
void setGameObjectPosition(short index, glm::vec3 pos){
  fullscene.scene.idToGameObjects[index].position = pos;
}
short getGameObjectByName(std::string name){
  for (auto [id, gameObj]: fullscene.scene.idToGameObjects){
    if (gameObj.name == name){
      return id;
    }
  }
  return -1;
}
void setSelectionMode(bool enabled){
  state.isSelectionMode = enabled;
}

void renderScene(Scene& scene, GLint shaderProgram, glm::mat4 projection, glm::mat4 view,  glm::mat4 model, bool useSelectionColor){
  glUseProgram(shaderProgram);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));    
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),  1, GL_FALSE, glm::value_ptr(view));

  for (unsigned int i = 0; i < fullscene.scene.rootGameObjectsH.size(); i++){
    drawGameobject(fullscene.scene.idToGameObjectsH[fullscene.scene.rootGameObjectsH[i]], scene, shaderProgram, model, useSelectionColor);
  }  
}
void renderUI(Mesh& crosshairSprite, unsigned int currentFramerate){
  glUseProgram(uiShaderProgram);
  glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(orthoProj)); 

  if (!state.isSelectionMode){
    drawSpriteAround(uiShaderProgram, crosshairSprite, state.currentScreenWidth/2, state.currentScreenHeight/2, 40, 40);
  }else if (!state.isRotateSelection){
    drawSpriteAround(uiShaderProgram, crosshairSprite, state.cursorLeft, state.cursorTop, 20, 20);
  }

  if (showDebugInfo){
    drawText(std::to_string(currentFramerate) + state.additionalText, 10, 20, 4);
    std::string modeText = state.mode == 0 ? "translate" : (state.mode == 1 ? "scale" : "rotate"); 
    std::string axisText = state.axis == 0 ? "xz" : "xy";
    drawText("Mode: " + modeText + " Axis: " + axisText, 10, 40, 3);      

    std::string manipulatorAxisString;
    if (state.manipulatorAxis == XAXIS){
      manipulatorAxisString = "xaxis";
    }else if (state.manipulatorAxis == YAXIS){
      manipulatorAxisString = "yaxis";
    }else if (state.manipulatorAxis == ZAXIS){
      manipulatorAxisString = "zaxis";
    }else{
      manipulatorAxisString = "noaxis";
    }
    drawText("manipulator axis: " + manipulatorAxisString, 10, 50, 3);
  }
}

void onData(std::string data){
  std::cout << "got data: " << data << std::endl;
}
void sendMoveObjectMessage(){
  sendMessage((char*)"hello world");
}
void printVec3(std::string prefix, glm::vec3 vec){
  std::cout << prefix << vec.x << "," << vec.y << "," << vec.z << std::endl;
}
void printPhysicsInfo(PhysicsInfo physicsInfo){
  BoundInfo info = physicsInfo.boundInfo;
  std::cout << "x: [ " << info.xMin << ", " << info.xMax << "]" << std::endl;
  std::cout << "y: [ " << info.yMin << ", " << info.yMax << "]" << std::endl;
  std::cout << "z: [ " << info.zMin << ", " << info.zMax << "]" << std::endl;
  std::cout << "pos: (" << physicsInfo.gameobject.position.x << ", " << physicsInfo.gameobject.position.y << ", " << physicsInfo.gameobject.position.z << ")" << std::endl;
  std::cout << "box: (" << physicsInfo.collisionInfo.x << ", " << physicsInfo.collisionInfo.y << ", " << physicsInfo.collisionInfo.z << ")" << std::endl;
}
void dumpPhysicsInfo(){
  for (unsigned int i = 0; i < rigidbodies.size(); i++){
    printVec3("PHYSICS:" + std::to_string(i) + ":", getPosition(rigidbodies[i]));
  }
}
void updatePhysicsPositions(std::vector<btRigidBody*>& rigidbodies, FullScene& fullscene){
  for (unsigned int i = 0; i < rigidbodies.size(); i++){
    fullscene.scene.idToGameObjects[i].rotation = getRotation(rigidbodies[i]);
    fullscene.scene.idToGameObjects[i].position = getPosition(rigidbodies[i]);
  }
}
void addPhysicsBodies(physicsEnv physicsEnv, FullScene& fullscene){
  for (auto const& [id, _] : fullscene.scene.idToGameObjects){
    auto physicsInfo = getPhysicsInfoForGameObject(fullscene, id);
    printPhysicsInfo(physicsInfo);

    auto rigidPtr = addRigidBody(
      physicsEnv, 
      physicsInfo.gameobject.position,
      physicsInfo.collisionInfo.x, physicsInfo.collisionInfo.y, physicsInfo.collisionInfo.z,
      physicsInfo.gameobject.rotation,
      id == 1
    );

    rigidbodies.push_back(rigidPtr);
    std::cout << "ADDING PTR: " << rigidPtr << std::endl;
  }
}


int main(int argc, char* argv[]){
  cxxopts::Options cxxoption("ModEngine", "ModEngine is a game engine for hardcore fps");
  cxxoption.add_options()
   ("s,shader", "Folder path of default shader", cxxopts::value<std::string>()->default_value("./res/shaders/default"))
   ("t,texture", "Image to use as default texture", cxxopts::value<std::string>()->default_value("./res/textures/wood.jpg"))
   ("x,scriptpath", "Script file to use", cxxopts::value<std::string>()->default_value("./res/scripts/game.scm"))
   ("f,framebuffer", "Folder path of framebuffer", cxxopts::value<std::string>()->default_value("./res/shaders/framebuffer"))
   ("u,uishader", "Shader to use for ui", cxxopts::value<std::string>()->default_value("./res/shaders/ui"))
   ("c,crosshair", "Icon to use for crosshair", cxxopts::value<std::string>()->default_value("./res/textures/crosshairs/crosshair029.png"))
   ("o,font", "Font to use", cxxopts::value<std::string>()->default_value("./res/textures/fonts/gamefont"))
   ("z,fullscreen", "Enable fullscreen mode", cxxopts::value<bool>()->default_value("false"))
   ("i,info", "Show debug info", cxxopts::value<bool>()->default_value("false"))
   ("l,listen", "Start server instance (listen)", cxxopts::value<bool>()->default_value("false"))
   ("k,skiploop", "Skip main game loop", cxxopts::value<bool>()->default_value("false"))
   ("d,dumpphysics", "Dump physics info to file for external processing", cxxopts::value<bool>()->default_value("false"))
   ("p,physics", "Enable physics", cxxopts::value<bool>()->default_value("false"))
   ("h,help", "Print help")
  ;   

  const auto result = cxxoption.parse(argc, argv);
  bool dumpPhysics = result["dumpphysics"].as<bool>();
  if (result["help"].as<bool>()){
    std::cout << cxxoption.help() << std::endl;
    return 0;
  }
  bool enablePhysics = result["physics"].as<bool>();

  const std::string shaderFolderPath = result["shader"].as<std::string>();
  const std::string texturePath = result["texture"].as<std::string>();
  const std::string framebufferTexturePath = result["framebuffer"].as<std::string>();
  const std::string uiShaderPath = result["uishader"].as<std::string>();
  showDebugInfo = result["info"].as<bool>();
  
  std::cout << "LIFECYCLE: program starting" << std::endl;

  bool isServer = result["listen"].as<bool>();
  modsocket serverInstance;
  if (isServer){
    serverInstance = createServer();;
  }

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 
  GLFWmonitor* monitor = result["fullscreen"].as<bool>() ? glfwGetPrimaryMonitor() : NULL;
  GLFWwindow* window = glfwCreateWindow(state.currentScreenWidth, state.currentScreenHeight, "ModEngine", monitor, NULL);
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
     projection = glm::perspective(glm::radians(45.0f), (float)state.currentScreenWidth / state.currentScreenHeight, 0.1f, 1000.0f); 
     orthoProj = glm::ortho(0.0f, (float)state.currentScreenWidth, (float)state.currentScreenHeight, 0.0f, -1.0f, 1.0f);  
  }; 

  onFramebufferSizeChange(window, state.currentScreenWidth, state.currentScreenHeight);
  glfwSetFramebufferSizeCallback(window, onFramebufferSizeChange); 
  
  std::cout << "INFO: shader file path is " << shaderFolderPath << std::endl;
  unsigned int shaderProgram = loadShader(shaderFolderPath + "/vertex.glsl", shaderFolderPath + "/fragment.glsl");
  
  std::cout << "INFO: framebuffer file path is " << framebufferTexturePath << std::endl;
  unsigned int framebufferProgram = loadShader(framebufferTexturePath + "/vertex.glsl", framebufferTexturePath + "/fragment.glsl");

  std::cout << "INFO: ui shader file path is " << uiShaderPath << std::endl;
  uiShaderProgram = loadShader(uiShaderPath + "/vertex.glsl",  uiShaderPath + "/fragment.glsl");

  std::string selectionShaderPath = "./res/shaders/selection";
  std::cout << "INFO: selection shader path is " << selectionShaderPath << std::endl;
  unsigned int selectionProgram = loadShader(selectionShaderPath + "/vertex.glsl", selectionShaderPath + "/fragment.glsl");

  font fontToRender = readFont(result["font"].as<std::string>());
  fontMeshes = loadFontMeshes(fontToRender);
  Mesh crosshairSprite = loadSpriteMesh(result["crosshair"].as<std::string>());

  fullscene = deserializeFullScene(loadFile("./res/scenes/example.rawscene"));
  
  schemeBindings  = createStaticSchemeBindings(
    result["scriptpath"].as<std::string>(), 
    moveCamera, 
    rotateCamera, 
    removeObjectById, 
    makeObject, 
    getObjectsByType, 
    setActiveCamera,
    drawText,
    getGameObjectName,
    getGameObjectPosition,
    setGameObjectPosition,
    getGameObjectByName,
    setSelectionMode
  );

  if (SHELL_ENABLED){
    std::thread shellThread(startShell);
  }

  glfwSetCursorPosCallback(window, onMouseEvents); 
  glfwSetMouseButtonCallback(window, onMouseCallback);
  glfwSetKeyCallback(window, keyCallback);

  float deltaTime = 0.0f; // Time between current frame and last frame

  unsigned int frameCount = 0;
  float previous = glfwGetTime();
  float last60 = previous;

  unsigned int currentFramerate = 0;
  std::cout << "INFO: render loop starting" << std::endl;

  auto physicsEnv = initPhysics();
  addPhysicsBodies(physicsEnv, fullscene);

  if (result["skiploop"].as<bool>()){
    goto cleanup;
  }

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
    
    if (isServer){
      getDataFromSocket(serverInstance, onData);
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
    renderScene(fullscene.scene, selectionProgram, projection, view, glm::mat4(1.0f), true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(framebufferProgram); 
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
        
    handleInput(window, deltaTime, state, translate, scale, rotate, moveCamera, nextCamera, playSound, setObjectDimensions, sendMoveObjectMessage, makeObject);
    
    if (dumpPhysics){
      dumpPhysicsInfo();
    }
    if (enablePhysics){
      stepPhysicsSimulation(physicsEnv, 1.f / 60.f);
      updatePhysicsPositions(rigidbodies, fullscene);      
    }

    glfwPollEvents();
    
    // 2ND pass renders what we care about to the screen.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1, 0.1, 0.1, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    renderScene(fullscene.scene, shaderProgram, projection, view, glm::mat4(1.0f), false);
    renderUI(crosshairSprite, currentFramerate);

    schemeBindings.onFrame();

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
  
  cleanup:    
    deinitPhysics(physicsEnv); 
    cleanupSocket(serverInstance);
    stopSoundSystem();
    glfwTerminate(); 
   
  return 0;
}
