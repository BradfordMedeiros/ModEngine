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
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

#include "./main_api.h"
#include "./main_input.h"
#include "./scene/scene.h"
#include "./scene/physics.h"
#include "./scene/collision_cache.h"
#include "./scene/scenegraph.h"
#include "./scene/object_types.h"
#include "./scene/common/mesh.h"
#include "./scene/common/vectorgfx.h"
#include "./scene/common/util/loadmodel.h"
#include "./scene/common/util/boundinfo.h"
#include "./scene/sprites/readfont.h"
#include "./scene/sprites/sprites.h"
#include "./scene/animation/timeplayback.h"
#include "./scene/animation/animation.h"
#include "./scene/animation/playback.h"
#include "./scheme/scheme_bindings.h"
#include "./scheme/scriptmanager.h"
#include "./shaders.h"
#include "./translations.h"
#include "./sounds/soundmanager.h"
#include "./common/util.h"
#include "./colorselection.h"
#include "./state.h"
#include "./input.h"
#include "./network/network.h"
#include "./network/servers.h"
#include "./network/activemanager.h"
#include "./scene/recorder.h"
#include "./worldloader.h"
#include "./gizmo/sequencer.h"
#include "./gizmo/keymapper.h"
#include "./common/sysinterface.h"
#include "./drawing.h"
#include "./ainav.h"

unsigned int framebufferProgram;
unsigned int drawingProgram;
unsigned int blurProgram;
unsigned int quadVAO;

GameObject* activeCameraObj;
GameObject defaultCamera = GameObject {
  .id = -1,
  .name = "defaultCamera",
  .transformation = Transformation {
    .position = glm::vec3(0.f, -2.f, 0.f),
    .scale = glm::vec3(1.0f, 1.0f, 1.0f),
    .rotation = glm::quat(1.0, 0, 0, 0.0f),
  }
};

bool showDebugInfo = false;
std::string shaderFolderPath;

bool disableInput = false;
int numChunkingGridCells = 0;
float chunkSize = 100;
bool useChunkingSystem = false;
std::string rawSceneFile;
bool bootStrapperMode = false;
NetCode netcode { };

GameObjectVoxel* voxelPtr;
int32_t voxelPtrId = -1;
glm::mat4 voxelPtrModelMatrix = glm::mat4(1.f);

engineState state = getDefaultState(1920, 1080);
World world;
SysInterface interface;
std::string textureFolderPath;
float now = 0;

AnimationState animations;

DynamicLoading dynamicLoading;
std::vector<Line> lines;
std::vector<Line> bluelines;
std::vector<Line> permaLines;

std::map<unsigned int, Mesh> fontMeshes;

glm::mat4 projection;
glm::mat4 view;
unsigned int framebufferTexture;
unsigned int framebufferTexture2;
unsigned int framebufferTexture3;
unsigned int fbo;
unsigned int depthTextures[32];

const int numPortalTextures = 16;
unsigned int portalTextures[16];
std::map<objid, unsigned int> portalIdCache;

glm::mat4 orthoProj;
unsigned int uiShaderProgram;

SchemeBindingCallbacks schemeBindings;
std::queue<std::string> channelMessages;
std::queue<StringFloat> channelFloatMessages;
KeyRemapper keyMapper;

float quadVertices[] = {
  -1.0f,  1.0f,  0.0f, 1.0f,
  -1.0f, -1.0f,  0.0f, 0.0f,
   1.0f, -1.0f,  1.0f, 0.0f,

  -1.0f,  1.0f,  0.0f, 1.0f,
   1.0f, -1.0f,  1.0f, 0.0f,
   1.0f,  1.0f,  1.0f, 1.0f
};

const int numTextures = 32;
int activeDepthTexture = 0;

DrawingParams drawParams = getDefaultDrawingParams();

void updateDepthTexturesSize(){
  for (int i = 0; i < numTextures; i++){
    glBindTexture(GL_TEXTURE_2D, depthTextures[i]);
    // GL_DEPTH_COMPONENT32F
    glTexImage2D(GL_TEXTURE_2D, 0,  GL_DEPTH_STENCIL, state.currentScreenWidth, state.currentScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  }
}
void generateDepthTextures(){
  glGenTextures(numTextures, depthTextures);
  for (int i = 0; i < numTextures; i++){
    glBindTexture(GL_TEXTURE_2D, depthTextures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    updateDepthTexturesSize();
  }
}
void setActiveDepthTexture(int index){
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);  
  unsigned int texture = depthTextures[index];
  glBindTexture(GL_TEXTURE_2D, texture);
  // GL_DEPTH_ATTACHMENT
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
}

void updatePortalTexturesSize(){
  for (int i = 0; i < numPortalTextures; i++){
    glBindTexture(GL_TEXTURE_2D, portalTextures[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state.currentScreenWidth, state.currentScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);   
  }
}
void generatePortalTextures(){
  glGenTextures(numPortalTextures, portalTextures);
  for (int i = 0; i < numPortalTextures; i++){
    glBindTexture(GL_TEXTURE_2D, portalTextures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    updatePortalTexturesSize();
  }
}

std::vector<int32_t> playbacksToRemove;
void tickAnimations(AnimationState& animationState, float elapsedTime){
  for (auto &[_, playback] : animationState.playbacks){
    playback.setElapsedTime(elapsedTime);
  }
  for (auto groupId : playbacksToRemove){
    std::cout << "removed playback: " << groupId << std::endl;
    animationState.playbacks.erase(groupId);
  }
  playbacksToRemove.clear();
}

float initialTime = glfwGetTime();
TimePlayback timePlayback(
  initialTime, 
  [](float currentTime, float elapsedTime) -> void {
    tickAnimations(animations, elapsedTime);
  }, 
  []() -> void {}
); 



bool useYAxis = true;
void onDebugKey(){
  useYAxis = !useYAxis;
  if (timePlayback.isPaused()){
    timePlayback.play();
  }else{
    timePlayback.pause();
  }
}

void onDelete(){
  if (state.selectedIndex != -1){
    std::cout << "OnDelete object id: " << state.selectedIndex << std::endl;
    removeObjectById(state.selectedIndex);
    state.selectedIndex = -1;
  }
}

unsigned int textureToPaint = -1;
bool canPaint = false;

void applyPainting(objid id){
  auto texture = textureForId(world, id);
  if (texture.has_value()){
    textureToPaint = texture.value().textureId;
    canPaint = true;
  }
  //std::cout << "texture id is: " << texture.textureId << std::endl;
}

glm::vec3 uvToOffset(UVCoord coord){
  float xCoord = convertBase(coord.x, 0, 1, -1, 1);
  float yCoord = convertBase(coord.y, 0, 1, -1, 1);
  return glm::vec3(xCoord, yCoord, 0.f);
}

void handlePainting(UVCoord uvsToPaint){
  if (!canPaint || !state.shouldPaint){
    return;
  }

  glUseProgram(drawingProgram); 

  glBindTexture(GL_TEXTURE_2D, textureToPaint);
  int w, h;
  int miplevel = 0;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);

  glViewport(0, 0, w, h);

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureToPaint, 0);

  glUniformMatrix4fv(glGetUniformLocation(drawingProgram, "model"), 1, GL_FALSE, glm::value_ptr(
    glm::scale(
      glm::translate(glm::mat4(1.0f), uvToOffset(uvsToPaint)), 
      glm::vec3(0.01f, 0.01f, 0.01f) * drawParams.scale)
    )
  );
  glUniform1f(glGetUniformLocation(drawingProgram, "opacity"), drawParams.opacity);
  glUniform3fv(glGetUniformLocation(drawingProgram, "tint"), 1, glm::value_ptr(drawParams.tint));

  glBindTexture(GL_TEXTURE_2D, world.textures.at(activeTextureName(drawParams, world.textures)).textureId);
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glViewport(0, 0, state.currentScreenWidth, state.currentScreenHeight);
}

bool selectItemCalled = false;
bool shouldCallItemSelected = false;
void selectItem(objid selectedId, Color pixelColor){
  if (!showDebugInfo){
    return;
  }

  applyPainting(selectedId);

  auto groupid = getGroupId(world, selectedId);
  auto selectedObject =  getGameObject(world, groupid);
  applyFocusUI(world.objectMapping, selectedId, sendNotifyMessage);

  shouldCallItemSelected = true;

  if (!state.shouldSelect){
    return;
  }
  state.selectedIndex =  groupid;
  state.selectedName = selectedObject.name + "(" + std::to_string(state.selectedIndex) + ")";
  state.additionalText = "     <" + std::to_string((int)(255 * pixelColor.r)) + ","  + std::to_string((int)(255 * pixelColor.g)) + " , " + std::to_string((int)(255 * pixelColor.b)) + ">  " + " --- " + state.selectedName;
}

glm::mat4 renderPortalView(PortalInfo info, Transformation transform){
  if (!info.perspective){
    return renderView(info.cameraPos, info.cameraRotation);
  }
  auto cameraToPortalOffset = transform.position - info.portalPos;
  return glm::inverse(renderView(glm::vec3(0.f, 0.f, 0.f), info.portalRotation) *  glm::inverse(renderView(cameraToPortalOffset, transform.rotation))) * renderView(info.cameraPos, info.cameraRotation);
}

// TODO - needs to be done relative to parent, not local space
void teleportObject(objid objectId, objid portalId){
  std::cout << "teleporting object: " << objectId << std::endl;
  GameObject& gameobject = getGameObject(world, objectId);
  auto portalView = glm::inverse(renderPortalView(getPortalInfo(world, portalId), gameobject.transformation));
  auto newTransform = getTransformationFromMatrix(portalView);
  auto newPosition = newTransform.position;
  physicsTranslateSet(world, objectId, newPosition);

}

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2, glm::vec3 contactPos){
  auto obj1Id = getIdForCollisionObject(world, obj1);
  auto obj2Id = getIdForCollisionObject(world, obj2);
  schemeBindings.onCollisionEnter(obj1Id, obj2Id, contactPos);

  auto obj1Name = getGameObject(world, obj1Id).name;
  auto obj2Name = getGameObject(world, obj2Id).name;
  std::cout << "collision: " << obj1Name << " colliden with: " << obj2Name << std::endl;

  auto obj1IsPortal = isPortal(world, obj1Id);
  auto obj2IsPortal = isPortal(world, obj2Id);
  if (obj1IsPortal && !obj2IsPortal){
    std::cout << "teleport " << obj2Name << " through " << obj1Name << std::endl;
    teleportObject(obj2Id, obj1Id);
  }else if (!obj1IsPortal && obj2IsPortal){
    std::cout << "teleport " << obj1Name << " through " << obj2Name << std::endl;
    teleportObject(obj1Id, obj2Id);
  } 
}
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2){
  schemeBindings.onCollisionExit(getIdForCollisionObject(world, obj1), getIdForCollisionObject(world, obj2));
}


void onMouseCallback(GLFWwindow* window, int button, int action, int mods){
  mouse_button_callback(disableInput, window, state, button, action, mods, onMouseButton);
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
    selectItemCalled = true;
  }

  schemeBindings.onMouseCallback(button, action, mods);

  if (button == 0 && voxelPtr != NULL){
    voxelPtr -> voxel.selectedVoxels.clear();
  }
}


// This is wasteful, as obviously I shouldn't be loading in all the textures on load, but ok for now. 
// This shoiuld really just be creating a list of names, and then the cycle above should cycle between possible textures to load, instead of what is loaded 
void loadAllTextures(){
  for (auto texturePath : listFilesWithExtensions(textureFolderPath, { "png", "jpg" })){
    loadTextureWorld(world, texturePath);
  }
}


void translate(float x, float y, float z){
  if (state.selectedIndex == -1 || !idExists(world, state.selectedIndex)){
    return;
  }
  physicsTranslate(world, state.selectedIndex, x, y, z, state.moveRelativeEnabled);
}
void scale(float x, float y, float z){
  if (state.selectedIndex == -1 || !idExists(world, state.selectedIndex)){
    return;
  }
  physicsScale(world, state.selectedIndex, x, y, z);
}
void rotate(float x, float y, float z){
  if (state.selectedIndex == -1 || !idExists(world, state.selectedIndex)){
    return;
  }
  physicsRotate(world, state.selectedIndex, x, y, z);
}
void setObjectDimensions(int32_t index, float width, float height, float depth){
  if (state.selectedIndex == -1 || !idExists(world, state.selectedIndex)){
    return;
  }
  auto gameObjV = world.objectMapping.at(state.selectedIndex);  // todo this is bs, need a wrapper around objectmappping + scene
  auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); 
  if (meshObj != NULL){
    // @TODO this is resizing based upon first mesh only, which is questionable
    auto newScale = getScaleEquivalent(meshObj -> meshesToRender.at(0).boundInfo, width, height, depth);   // this is correlated to logic in scene//getPhysicsInfoForGameObject, needs to be fixed
    std::cout << "new scale: (" << newScale.x << ", " << newScale.y << ", " << newScale.z << ")" << std::endl;
    getGameObject(world, state.selectedIndex).transformation.scale = newScale;
  } 
}


void updateVoxelPtr(){
  auto voxelIndexes = getGameObjectsIndex<GameObjectVoxel>(world.objectMapping);
  if (voxelIndexes.size() > 0){
    int32_t id = voxelIndexes.at(0);
    GameObjectObj& toRender = world.objectMapping.at(id);
    auto voxelObj = std::get_if<GameObjectVoxel>(&toRender);
    assert(voxelObj != NULL);
    voxelPtr = voxelObj;
    voxelPtrId = id;    
  }else{
    voxelPtr = NULL;
    voxelPtrId = -1;
  }
}

void addLineNextCycle(glm::vec3 fromPos, glm::vec3 toPos){
  Line line = {
    .fromPos = fromPos,
    .toPos = toPos
  };
  lines.push_back(line);
}

std::vector<glm::vec3> traversalPositions;
std::vector<glm::vec3> parentTraversalPositions;
void addPositionToRender(glm::mat4 modelMatrix, glm::mat4 parentMatrix){
  traversalPositions.push_back(getTransformationFromMatrix(modelMatrix).position);
  parentTraversalPositions.push_back(getTransformationFromMatrix(parentMatrix).position);
}
void clearTraversalPositions(){
  traversalPositions.clear();
  parentTraversalPositions.clear();
}
void drawTraversalPositions(){
  for (int i = 0; i < traversalPositions.size(); i++){
    auto fromPos = traversalPositions.at(i);
    auto toPos = parentTraversalPositions.at(i);
    addLineNextCycle(fromPos, toPos);
  }
}


void displayRails(std::map<int32_t, RailConnection> railPairs){
  for (auto [id, rail] : railPairs){
    bluelines.push_back(Line {
      .fromPos = getGameObject(world, rail.from).transformation.position,
      .toPos = getGameObject(world, rail.to).transformation.position
    });
  }
}

std::map<std::string, GLint> shaderNameToId;
GLint getShaderByName(std::string fragShaderName, GLint shaderProgram){
  if (fragShaderName == ""){
    return shaderProgram;
  }
  if (shaderNameToId.find(fragShaderName) == shaderNameToId.end()){
    auto shaderId = loadShader(shaderFolderPath + "/vertex.glsl", fragShaderName);
    shaderNameToId[fragShaderName] = shaderId;   
  }
  return shaderNameToId.at(fragShaderName);
}
void setShaderData(GLint shader, glm::mat4 projection, glm::mat4 view, std::vector<LightInfo>& lights, bool orthographic, glm::vec3 color, objid id){
  glUseProgram(shader);
  glUniform1i(glGetUniformLocation(shader, "maintexture"), 0);        
  glUniform1i(glGetUniformLocation(shader, "emissionTexture"), 1);
  glUniform1i(glGetUniformLocation(shader, "opacityTexture"), 2);
  glActiveTexture(GL_TEXTURE0); 

  glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));    
  glUniformMatrix4fv(glGetUniformLocation(shader, "view"),  1, GL_FALSE, glm::value_ptr(view));
  glUniform3fv(glGetUniformLocation(shader, "cameraPosition"), 1, glm::value_ptr(defaultCamera.transformation.position));
  glUniform1i(glGetUniformLocation(shader, "enableDiffuse"), state.enableDiffuse);
  glUniform1i(glGetUniformLocation(shader, "enableSpecular"), state.enableSpecular);

  glUniform1i(glGetUniformLocation(shader, "numlights"), lights.size());
  for (int i = 0; i < lights.size(); i++){
    glm::vec3 position = lights.at(i).pos;
    glUniform3fv(glGetUniformLocation(shader, ("lights[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(shader, ("lightscolor[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(lights.at(i).light.color));
    glUniform3fv(glGetUniformLocation(shader, ("lightsdir[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(directionFromQuat(lights.at(i).rotation)));
  }
  if (orthographic){
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(glm::ortho(-1.f, 1.f, -1.f, 1.f, 0.f, 100.0f)));    
  }else{
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));    
  }

  glUniform3fv(glGetUniformLocation(shader, "tint"), 1, glm::value_ptr(color));
  glUniform4fv(glGetUniformLocation(shader, "encodedid"), 1, glm::value_ptr(getColorFromGameobject(id)));
}

glm::vec3 getTintIfSelected(bool isSelected, glm::vec3 defaultTint){
  if (isSelected && state.highlight){
    return glm::vec3(1.0f, 0.0f, 0.0f);
  }
  return defaultTint;
}

float getTotalTime(){
  return now - initialTime;
}
void renderScene(Scene& scene, GLint shaderProgram, glm::mat4 projection, glm::mat4 view,  glm::mat4 model, std::vector<LightInfo>& lights, std::vector<PortalInfo> portals){
  if (scene.isNested){
    return;
  }
  glUseProgram(shaderProgram);

  clearTraversalPositions();
  traverseScene(world, scene, [shaderProgram, &scene, projection, view, &portals, &lights](int32_t id, glm::mat4 modelMatrix, glm::mat4 parentModelMatrix, bool orthographic, std::string shader, glm::vec3 tint) -> void {
    assert(id >= 0);
    if (id == voxelPtrId){
      voxelPtrModelMatrix = modelMatrix;
    }
    
    bool objectSelected = idInGroup(world, id, state.selectedIndex);

    auto newShader = getShaderByName(shader, shaderProgram);
    setShaderData(newShader, projection, view, lights, orthographic, getTintIfSelected(objectSelected, tint), id);

    if (state.visualizeNormals){
      glUniformMatrix4fv(glGetUniformLocation(newShader, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
      drawMesh(world.meshes.at("./res/models/cone/cone.obj"), newShader); 
    }

    // bounding code //////////////////////
    auto gameObjV = world.objectMapping.at(id);
    auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); 
    if (meshObj != NULL && meshObj -> meshesToRender.size() > 0){
      // @TODO i use first mesh to get sizing for bounding box, obviously that's questionable
      auto bounding = getBoundRatio(world.meshes.at("./res/models/boundingbox/boundingbox.obj").boundInfo, meshObj -> meshesToRender.at(0).boundInfo);
      glUniformMatrix4fv(glGetUniformLocation(newShader, "model"), 1, GL_FALSE, glm::value_ptr(glm::scale(getMatrixForBoundRatio(bounding, modelMatrix), glm::vec3(1.01f, 1.01f, 1.01f))));

      if (objectSelected){
        drawMesh(world.meshes.at("./res/models/boundingbox/boundingbox.obj"), newShader);
      }
    }
    /////////////////////////////// end bounding code

    glUniformMatrix4fv(glGetUniformLocation(newShader, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glUniform1f(glGetUniformLocation(newShader, "discardTexAmount"), state.discardAmount);  // TODO - remove this global discard tex amount
    glUniform1f(glGetUniformLocation(newShader, "time"), getTotalTime());

    bool isPortal = false;
    bool isPerspectivePortal = false;

    for (auto portal : portals){
      if (id == portal.id){
        isPortal = true;
        isPerspectivePortal = portal.perspective;
        break;
      }
    }


    bool portalTextureInCache = portalIdCache.find(id) != portalIdCache.end();
    glStencilMask(isPortal ? 0xFF : 0x00);

    renderObject(
      newShader, 
      id, 
      world.objectMapping, 
      world.meshes.at("./res/models/ui/node.obj"),
      world.meshes.at("./res/models/camera/camera.dae"),
      world.meshes.at("./res/models/box/plane.dae"),
      state.showCameras, 
      state.showBoneWeight,
      state.useBoneTransform,
      (isPortal && portalTextureInCache &&  !isPerspectivePortal) ? portalIdCache.at(id) : -1
    );

    glStencilFunc(GL_EQUAL, 1, 0xFF);
    if (isPortal && portalTextureInCache && isPerspectivePortal){
      glUseProgram(framebufferProgram); 
      glDisable(GL_DEPTH_TEST);
      glBindVertexArray(quadVAO);
      loadTextureWorld(world, "./res/textures/wood.jpg");
      auto textureId = world.textures.at("./res/textures/wood.jpg").textureId;
      glBindTexture(GL_TEXTURE_2D,  portalIdCache.at(id));
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glEnable(GL_DEPTH_TEST);
      glUseProgram(newShader); 
    }
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);
    addPositionToRender(modelMatrix, parentModelMatrix);
  });
  if (state.showCameras){
    drawTraversalPositions();   
  }
}

void renderVector(GLint shaderProgram, glm::mat4 projection, glm::mat4 view, glm::mat4 model){
  glUseProgram(shaderProgram);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));    
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),  1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.05, 0.f, 1.f)));

  // Draw grid for the chunking logic if that is specified, else lots draw the snapping translations
  if (numChunkingGridCells > 0){
    float offset = ((numChunkingGridCells % 2) == 0) ? (dynamicLoading.chunkXWidth / 2) : 0;
    drawGrid3DCentered(numChunkingGridCells, dynamicLoading.chunkXWidth, offset, offset, offset);
    glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.05, 1.f, 1.f)));
  }else{
    if (state.manipulatorMode == TRANSLATE){
      float snapGridSize = getSnapTranslateSize();
      if (snapGridSize > 0){
        auto position = getGameObjectPosition(state.selectedIndex, false);
        drawGrid3DCentered(10, snapGridSize, position.x, position.y, position.z);  
        glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.05, 1.f, 1.f)));     
      }
    }
  }
  drawCoordinateSystem(100.f);

  glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(1.f, 0.f, 0.f)));
  if (permaLines.size() > 0){
   drawLines(permaLines);
  }
  if (lines.size() > 0){
   drawLines(lines);
  }

  glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(1.f, 0.f, 0.f)));
  if (bluelines.size() > 0){
   drawLines(bluelines);
  }
  lines.clear();
  bluelines.clear();
}

void renderUI(Mesh& crosshairSprite, unsigned int currentFramerate, Color pixelColor){
  glUseProgram(uiShaderProgram);
  glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(orthoProj)); 

  if (!showDebugInfo){
    return;
  }

  if(!state.isRotateSelection){
     drawSpriteAround(uiShaderProgram, crosshairSprite, state.cursorLeft, state.cursorTop, 20, 20);
  }

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
  drawText("position: " + print(defaultCamera.transformation.position), 10, 60, 3);
  drawText("rotation: " + print(defaultCamera.transformation.rotation), 10, 70, 3);

  drawText("fov: " + std::to_string(state.fov), 10, 80, 3);
  drawText("cursor: " + std::to_string(state.cursorLeft) + " / " + std::to_string(state.cursorTop)  + "(" + std::to_string(state.currentScreenWidth) + "||" + std::to_string(state.currentScreenHeight) + ")", 10, 90, 3);
  
  if (state.selectedIndex != -1){
    auto obj = getGameObject(world, state.selectedIndex);
    drawText("position: " + print(obj.transformation.position), 10, 100, 3);
    drawText("scale: " + print(obj.transformation.scale), 10, 110, 3);
    drawText("rotation: " + print(obj.transformation.rotation), 10, 120, 3);
  }
    
  drawText("pixel color: " + std::to_string(pixelColor.r) + " " + std::to_string(pixelColor.g) + " " + std::to_string(pixelColor.b), 10, 140, 3);
  drawText("showing color: " + std::string(state.showBoneWeight ? "bone weight" : "bone indicies") , 10, 150, 3);

  drawText(std::string("animation info: ") + (timePlayback.isPaused() ? "paused" : "playing"), 10, 170, 3);
  drawText("using animation: " + std::to_string(-1) + " / " + std::to_string(-1) , 40, 180, 3);
  drawText("using object id: -1" , 40, 190, 3);
}

void onClientMessage(std::string message){
  schemeBindings.onTcpMessage(message);
}

 // @TODO --  this needs to makeObject in the right scene
void handleCreate(UdpPacket& packet){
  auto create = packet.payload.createpacket;
  if (world.scenes.find(packet.payload.createpacket.sceneId) == world.scenes.end()){
    return;
  }

  auto id = create.id;   
  if (idExists(world, id)){     // could conceptually do a comparison to see if it changed, but probably not
    std::cout << "INFO: id already exits: " << id << std::endl;
    return;
  }
  std::string serialobj = create.serialobj;
  assert(serialobj.size() > 0);
  auto newObjId = makeObject(serialobj, create.id, true, packet.payload.createpacket.sceneId, true);                        
  assert(newObjId == id);
}
void handleDelete(UdpPacket& packet){
  auto deletep = packet.payload.deletepacket;
  if (idExists(world, deletep.id)){
    std::cout << "UDP CLIENT MESSAGE: DELETING: " << deletep.id << std::endl;
    removeObjectById(deletep.id);
  }else{
    std::cout << "UDP CLIENT MESSAGE: ID NOT EXIST: " << deletep.id << std::endl;
  }
}

void handleUpdate(UdpPacket& packet){
  auto update = packet.payload.updatepacket;

  if (idExists(world, update.id)){
    setProperties(world, update.id, update.properties);
  }else{
    std::cout << "WARNING: Udp client update: does not exist " << update.id << std::endl;
  }
}
void onUdpClientMessage(UdpPacket& packet){
  std::cout << "INFO: GOT UDP CLIENT MESSAGE" << std::endl;
  if (packet.type == SETUP){
    std::cout << "WARNING: should not get setup packet type" << std::endl;
  }
  else if (packet.type == LOAD){
    std::string sceneData = packet.payload.loadpacket.sceneData;
    std::cout << "trying to load scene packet!" << std::endl;
    loadSceneData(sceneData, packet.payload.loadpacket.sceneId);  
  }else if (packet.type == UPDATE){
    handleUpdate(packet);
  }else if (packet.type == CREATE){
    handleCreate(packet);
  }else if (packet.type == DELETE){
    handleDelete(packet);
  }
  //schemeBindings.onUdpMessage(message);
}

void onUdpServerMessage(UdpPacket& packet){
  if (packet.type == SETUP){
    std::cout << "INFO: SETUP PACKET HANDLED IN SERVER CODE" << packet.payload.setuppacket.connectionHash << std::endl;
  }else if (packet.type == LOAD){
    std::cout << "WARNING: LOAD message server, ignoring" << std::endl;
  }else if (packet.type == UPDATE){
    handleUpdate(packet);
  }else if (packet.type == CREATE){
    handleCreate(packet);
  }else if (packet.type == DELETE){
    handleDelete(packet);
  }else {
    std::cout << "ERROR: unknown packet type" << std::endl;
  }
}

std::string screenshotPath = "./res/textures/screenshot.png";
void takeScreenshot(std::string filepath){
  state.takeScreenshot = true;
  screenshotPath = filepath;
}
void saveScreenshot(){
  int w, h;
  int miplevel = 0;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);
  char* data = new char[w * h * 3];
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, data);
  saveTextureData(screenshotPath, data, w, h);
  delete data;
}


void genFramebufferTexture(unsigned int *texture){
  glGenTextures(1, texture);
  glBindTexture(GL_TEXTURE_2D, *texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state.currentScreenWidth, state.currentScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

bool isNav(objid id){
  return isNavmesh(world.objectMapping, id);
}

NavGraph navgraph = createNavGraph();
glm::vec3 navPosition(objid id, glm::vec3 target){
  auto currentMesh = targetNavmesh(getGameObjectPosition(id, true), raycastW, isNav, getGameObjectName);
  auto destinationMesh = targetNavmesh(target, raycastW, isNav, getGameObjectName);
  auto searchResult = aiNavSearchPath(navgraph, currentMesh, destinationMesh);

  if (!searchResult.found || searchResult.path.size() == 0){
    return getGameObjectPosition(id, true);
  }

  auto targetNav = searchResult.path.at(0);
  auto targetLink = aiTargetLink(navgraph, currentMesh, targetNav);

  std::cout << "path is: [ ";
  for (auto node : searchResult.path){
    std::cout << node << " ";
  }
  std::cout << " ]" << std::endl;
  std::cout << "going from: " << currentMesh << " to " << targetNav << std::endl;
  std::cout << "to link: " << print(targetLink) << std::endl;
  return aiNavPosition(id, targetLink, getGameObjectPosition, raycastW, isNav);
}

int main(int argc, char* argv[]){
  cxxopts::Options cxxoption("ModEngine", "ModEngine is a game engine for hardcore fps");
  cxxoption.add_options()
   ("s,shader", "Folder path of default shader", cxxopts::value<std::string>()->default_value("./res/shaders/default"))
   ("t,texture", "Additional textures folder to use", cxxopts::value<std::string>()->default_value("./res"))
   ("x,scriptpath", "Script file to use", cxxopts::value<std::vector<std::string>>()->default_value(""))
   ("f,framebuffer", "Folder path of framebuffer", cxxopts::value<std::string>()->default_value("./res/shaders/framebuffer"))
   ("u,uishader", "Shader to use for ui", cxxopts::value<std::string>()->default_value("./res/shaders/ui"))
   ("c,camera", "Camera to use after initial load", cxxopts::value<std::string>()->default_value(""))
   ("o,font", "Font to use", cxxopts::value<std::string>()->default_value("./res/textures/fonts/gamefont"))
   ("z,fullscreen", "Enable fullscreen mode", cxxopts::value<bool>()->default_value("false"))
   ("i,info", "Show debug info", cxxopts::value<bool>()->default_value("false"))
   ("k,skiploop", "Skip main game loop", cxxopts::value<bool>()->default_value("false"))
   ("d,dumpphysics", "Dump physics info to file for external processing", cxxopts::value<bool>()->default_value("false"))
   ("b,bootstrapper", "Run the server as a server bootstrapper only", cxxopts::value<bool>()->default_value("false"))
   ("p,physics", "Enable physics", cxxopts::value<bool>()->default_value("false"))
   ("y,debugphysics", "Enable physics debug drawing", cxxopts::value<bool>()->default_value("false"))
   ("n,noinput", "Disable default input (still allows custom input handling in scripts)", cxxopts::value<bool>()->default_value("false"))
   ("g,grid", "Size of grid chunking grid used for open world streaming, default to zero (no grid)", cxxopts::value<int>()->default_value("0"))
   ("e,chunksize", "Size of worlds chunks", cxxopts::value<float>()->default_value("100"))
   ("w,world", "Use streaming chunk system", cxxopts::value<bool>()->default_value("false"))
   ("r,rawscene", "Rawscene file to use (only used when world = false)", cxxopts::value<std::vector<std::string>>() -> default_value(""))
   ("m,mapping", "Key mapping file to use", cxxopts::value<std::string>()->default_value(""))
   ("h,help", "Print help")
  ;        

  const auto result = cxxoption.parse(argc, argv);
  bool dumpPhysics = result["dumpphysics"].as<bool>();
  numChunkingGridCells = result["grid"].as<int>();
  useChunkingSystem = result["world"].as<bool>();
  chunkSize = result["chunksize"].as<float>();

  auto rawScenes = result["rawscene"].as<std::vector<std::string>>();
  rawSceneFile =  rawScenes.size() > 0 ? rawScenes.at(0) : "./res/scenes/example.rawscene";

  keyMapper = readMapping(result["mapping"].as<std::string>());

  if (result["help"].as<bool>()){
    std::cout << cxxoption.help() << std::endl;
    return 0;
  }
  bool enablePhysics = result["physics"].as<bool>();
  bootStrapperMode = result["bootstrapper"].as<bool>();

  shaderFolderPath = result["shader"].as<std::string>();
  textureFolderPath = result["texture"].as<std::string>();
  const std::string framebufferTexturePath = result["framebuffer"].as<std::string>();
  const std::string uiShaderPath = result["uishader"].as<std::string>();
  showDebugInfo = result["info"].as<bool>();
  
  std::cout << "LIFECYCLE: program starting" << std::endl;
  disableInput = result["noinput"].as<bool>();

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

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);  

  genFramebufferTexture(&framebufferTexture);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

  genFramebufferTexture(&framebufferTexture2);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebufferTexture2, 0);

  genFramebufferTexture(&framebufferTexture3);

  generateDepthTextures();
  generatePortalTextures();
  setActiveDepthTexture(0);

  if (!glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE){
    std::cerr << "ERROR: framebuffer incomplete" << std::endl;
    return -1;
  }

  unsigned int quadVBO;
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
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state.currentScreenWidth, state.currentScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

     glBindTexture(GL_TEXTURE_2D, framebufferTexture2);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state.currentScreenWidth, state.currentScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

     glBindTexture(GL_TEXTURE_2D, framebufferTexture3);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state.currentScreenWidth, state.currentScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

     updateDepthTexturesSize();
     updatePortalTexturesSize();

     glViewport(0, 0, state.currentScreenWidth, state.currentScreenHeight);

     // TODO orthoproj is using current screen width and height.  Switch this to match NDI for simplification. 
     orthoProj = glm::ortho(0.0f, (float)state.currentScreenWidth, (float)state.currentScreenHeight, 0.0f, -1.0f, 1.0f);  
  }; 

  onFramebufferSizeChange(window, state.currentScreenWidth, state.currentScreenHeight);
  glfwSetFramebufferSizeCallback(window, onFramebufferSizeChange); 
  
  std::cout << "INFO: shader file path is " << shaderFolderPath << std::endl;
  unsigned int shaderProgram = loadShader(shaderFolderPath + "/vertex.glsl", shaderFolderPath + "/fragment.glsl");
  
  std::cout << "INFO: framebuffer file path is " << framebufferTexturePath << std::endl;
  framebufferProgram = loadShader(framebufferTexturePath + "/vertex.glsl", framebufferTexturePath + "/fragment.glsl");

  std::string depthShaderPath = "./res/shaders/depth";
  std::cout << "INFO: depth file path is " << depthShaderPath << std::endl;
  unsigned int depthProgram = loadShader(depthShaderPath + "/vertex.glsl", depthShaderPath + "/fragment.glsl");

  std::cout << "INFO: ui shader file path is " << uiShaderPath << std::endl;
  uiShaderProgram = loadShader(uiShaderPath + "/vertex.glsl",  uiShaderPath + "/fragment.glsl");

  std::string selectionShaderPath = "./res/shaders/selection";
  std::cout << "INFO: selection shader path is " << selectionShaderPath << std::endl;
  unsigned int selectionProgram = loadShader(selectionShaderPath + "/vertex.glsl", selectionShaderPath + "/fragment.glsl");

  std::string drawingShaderPath = "./res/shaders/drawing";
  std::cout << "INFO: drawing shader path is: " << drawingShaderPath << std::endl;
  drawingProgram = loadShader(drawingShaderPath + "/vertex.glsl", drawingShaderPath + "/fragment.glsl");

  std::string blurShaderPath = "./res/shaders/blur";
  std::cout << "INFO: blur shader path is: " << blurShaderPath << std::endl;
  blurProgram = loadShader(blurShaderPath + "/vertex.glsl", blurShaderPath + "/fragment.glsl");

  fontMeshes = loadFontMeshes(readFont(result["font"].as<std::string>()));
  Mesh crosshairSprite = loadSpriteMesh("./res/textures/crosshairs/crosshair029.png", loadTexture);
 
  createStaticSchemeBindings(
    loadScene,
    loadSceneObj,
    unloadScene,
    unloadAllScenes,
    listScenes,
    sendLoadScene,
    moveCamera, 
    rotateCamera, 
    removeObjectById, 
    getObjectsByType, 
    setActiveCamera,
    drawText,
    addLineNextCycle,
    getGameObjectName,
    getGameObjectAttr,
    setGameObjectAttr,
    getGameObjectPosition,
    setGameObjectPosition,
    setGameObjectPositionRelative,
    getGameObjectRotation,
    setGameObjectRotation,
    setFrontDelta,
    moveRelative,
    orientationFromPos,
    getGameObjectByName,
    setSelectionMode,
    applyImpulse,
    applyImpulseRel,
    clearImpulse,
    listAnimations,
    playAnimation,
    listSounds,
    playSoundState,
    listModels,
    sendEventMessage,
    sendNotifyMessage,
    attachToRail,
    unattachFromRail,
    timeSeconds,
    saveScene,
    listServers,
    connectServer,
    disconnectServer,
    sendMessageToActiveServer,
    sendDataUdp,
    createTrack,
    playbackTrack,
    createStateMachine,
    playStateMachine,
    setStateMachine,
    startRecording,
    playRecording,
    makeObjectAttr,
    raycastW,
    takeScreenshot,
    setState,
    setFloatState,
    setIntState,
    setTexture,
    navPosition
  );

  schemeBindings = getSchemeCallbacks();
  if(bootStrapperMode){
    netcode = initNetCode(schemeBindings.onPlayerJoined, schemeBindings.onPlayerLeave);
  }

  for (auto script : result["scriptpath"].as<std::vector<std::string>>()){
    loadScript(script, -1, bootStrapperMode);
  }

  BulletDebugDrawer drawer(addLineNextCycle);
  btIDebugDraw* debuggerDrawer = result["debugphysics"].as<bool>() ?  &drawer : NULL;

  world = createWorld(
    onObjectEnter, 
    onObjectLeave, 
    [](GameObject& obj) -> void { 
      if (!obj.netsynchronize){   
        return;
      }
      UdpPacket packet { .type = UPDATE };
      packet.payload.updatepacket = UpdatePacket { 
        .id = obj.id,
        .properties = getProperties(world, obj.id),
      };
      if (bootStrapperMode){
        sendUdpPacketToAllUdpClients(netcode, toNetworkPacket(packet));
      }else if (isConnectedToServer()){
        sendDataOnUdpSocket(toNetworkPacket(packet));
      }
    }, 
    [](GameObject &obj) -> void {
      if (!obj.netsynchronize){
        return;
      }

      std::cout << "created obj id: " << obj.id << std::endl;
      UdpPacket packet { .type = CREATE };

      packet.payload.createpacket = CreatePacket { 
        .id = obj.id,
        .sceneId = world.idToScene.at(obj.id),
      };
      auto serialobj = serializeObject(world, obj.id);
      if (serialobj == ""){
        return; // "" is sentinal, that specifies that the group id != the id, which we do not send over a network.  This needs to be more explicit
      }

      copyStr(serialobj, packet.payload.createpacket.serialobj, sizeof(packet.payload.createpacket.serialobj));

      if (bootStrapperMode){
        sendUdpPacketToAllUdpClients(netcode, toNetworkPacket(packet));
      }else if (isConnectedToServer()){
        sendDataOnUdpSocket(toNetworkPacket(packet));
      }
    },
    [](int32_t id, bool isNet) -> void {
      if (!isNet){
        return;
      }
      std::cout << "deleted obj id: " << id << std::endl;
      if (activeCameraObj != NULL &&  id == activeCameraObj -> id){
        activeCameraObj = NULL;
        std::cout << "active camera reset" << std::endl;
      }
      if (id == state.selectedIndex){
        state.selectedIndex = -1;
      }

      UdpPacket packet { .type = DELETE };
      packet.payload.deletepacket =  DeletePacket { .id = id };
      if (bootStrapperMode){
        sendUdpPacketToAllUdpClients(netcode, toNetworkPacket(packet));
      }else if (isConnectedToServer()){
        sendDataOnUdpSocket(toNetworkPacket(packet));
      }
    },
    debuggerDrawer
  );

  interface = SysInterface {
    .loadClip = loadSoundState,
    .unloadClip = unloadSoundState,
    .loadScript = loadScriptFromWorld,
    .unloadScript = unloadScript,
    .getCurrentTime = getTotalTime,
  };

  loadAllTextures();

  dynamicLoading = createDynamicLoading(chunkSize);
  if (!useChunkingSystem){
    std::cout << "INFO: # of intitial raw scenes: " << rawScenes.size() << std::endl;
    for (auto rawScene : rawScenes){
      loadScene(rawScene);
    }
  }

  auto defaultCameraName = result["camera"].as<std::string>();
  if (defaultCameraName != ""){
    setActiveCamera(defaultCameraName);
  }

  glfwSetCursorPosCallback(window, onMouseEvents); 
  glfwSetMouseButtonCallback(window, onMouseCallback);
  glfwSetScrollCallback(window, onScrollCallback);
  glfwSetKeyCallback(window, keyCallback);
  glfwSetCharCallback(window, keyCharCallback);
  
  float deltaTime = 0.0f; // Time between current frame and last frame

  unsigned int frameCount = 0;
  float previous = glfwGetTime();
  float last60 = previous;

  unsigned int currentFramerate = 0;
  std::cout << "INFO: render loop starting" << std::endl;

  GLenum buffers_to_render[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2,buffers_to_render);
  
  if (result["skiploop"].as<bool>()){
    goto cleanup;
  }

  //glEnable(GL_CULL_FACE);  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while (!glfwWindowShouldClose(window)){
    frameCount++;
    now = glfwGetTime();
    deltaTime = now - previous;   
    previous = now;
    timePlayback.setElapsedTime(deltaTime);

    if (frameCount == 60){
      frameCount = 0;
      float timedelta = now - last60;
      last60 = now;
      currentFramerate = (int)60/(timedelta);
    }


    processStateMachines();
    onWorldFrame(world, deltaTime, getTotalTime(), enablePhysics, dumpPhysics, interface);

    maybeGetClientMessage(onClientMessage);

    UdpPacket udpPacket { };
    auto hasClientMessage = maybeGetUdpClientMessage(&udpPacket, sizeof(udpPacket));
    if (hasClientMessage){
      onUdpClientMessage(udpPacket);
    }

    if (bootStrapperMode){
      UdpPacket packet { };
      auto networkPacket = toNetworkPacket(packet);
      bool udpPacketHasData = tickNetCode(netcode, networkPacket, [&packet]() -> std::string {
        if (packet.type == SETUP){
          return packet.payload.setuppacket.connectionHash;          
        }
        return "";
      });
      if (udpPacketHasData){
        onUdpServerMessage(packet);
      }
    }

    auto viewTransform = (state.useDefaultCamera || activeCameraObj == NULL) ? defaultCamera.transformation : fullTransformation(world, activeCameraObj -> id);
    view = renderView(viewTransform.position, viewTransform.rotation);

    projection = glm::perspective(glm::radians(state.fov), (float)state.currentScreenWidth / state.currentScreenHeight, 0.1f, 1000.0f); 

    glfwSwapBuffers(window);
    
    std::vector<LightInfo> lights = getLightInfo(world);
    std::vector<PortalInfo> portals = getPortalInfo(world);
    assert(portals.size() <= numPortalTextures);

    updateVoxelPtr();   // this should be removed.  This basically picks a voxel id to be the one we work on. Better would to just have some way to determine this (like with the core selection mechanism)

    setActiveDepthTexture(1);

    // depth buffer form point of view of 1 light source (all eventually, but 1 for now)

    auto lightPosition = lights.size() > 0 ? lights.at(0).pos : glm::vec3(0, 0, 0);
    auto lightRotation = lights.size() > 0 ? lights.at(0).rotation : glm::identity<glm::quat>();
    auto lightView = renderView(lightPosition, lightRotation);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebufferTexture2, 0);

    glEnable(GL_DEPTH_TEST);
    glClearColor(255.0, 255.0, 255.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto &[_, scene] : world.scenes){
      renderScene(scene, selectionProgram, projection, lightView, glm::mat4(1.0f), lights, portals);    // selection program since it's lightweight and we just care about depth buffer
    }

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "worldtolight"), 1, GL_FALSE, glm::value_ptr(lightView));  // leftover from shadow mapping attempt, will revisit

    setActiveDepthTexture(0);

    // 1ST pass draws selection program shader to be able to handle selection 
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    
    for (auto &[_, scene] : world.scenes){
      renderScene(scene, selectionProgram, projection, view, glm::mat4(1.0f), lights, portals);
    }

    // Each portal requires a render pass
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_BLEND);

    auto uvCoord = getUVCoord(state.cursorLeft, state.cursorTop, state.currentScreenHeight);
    Color hoveredItemColor = getPixelColor(state.cursorLeft, state.cursorTop, state.currentScreenHeight);
    auto hoveredId= getIdFromColor(hoveredItemColor);
    
    state.lastHoveredIdInScene = state.hoveredIdInScene;
    state.hoveredIdInScene = idExists(world, hoveredId);
    state.lastHoverIndex = state.currentHoverIndex;
    state.currentHoverIndex = hoveredId;

    if (selectItemCalled){
      if (state.hoveredIdInScene){
        selectItem(hoveredId, hoveredItemColor);
      }
      selectItemCalled = false;
      applyUICoord(
        world.objectMapping, 
        [](std::string topic, float value) -> void { 
          StringFloat message {
            .strValue = topic,
            .floatValue = value,
          };
          channelFloatMessages.push(message);
        }, 
        state.selectedIndex, 
        uvCoord.x, 
        uvCoord.y
      );
    }
    handlePainting(uvCoord);
     
    if (useChunkingSystem){
      handleChunkLoading(dynamicLoading, defaultCamera.transformation.position.x, defaultCamera.transformation.position.y, defaultCamera.transformation.position.z, loadScene, unloadScene);
    }

    assert(portals.size() <= numPortalTextures);

    std::map<objid, unsigned int> nextPortalCache;
    for (int i = 0; i < portals.size(); i++){
      auto portal = portals.at(i);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, portalTextures[i], 0);
      glClearColor(0.0, 0.0, 0.0, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      auto portalViewMatrix = renderPortalView(portal, viewTransform);
      for (auto &[_, scene] : world.scenes){
        renderScene(scene, shaderProgram, projection, portalViewMatrix, glm::mat4(1.0f), lights, portals);
      }
      nextPortalCache[portal.id] = portalTextures[i];
    }
    portalIdCache = nextPortalCache;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(framebufferProgram); 
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 2ND pass renders what we care about to the screen.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  
    glStencilFunc(GL_ALWAYS, 1, 0xFF);

    glClearColor(0.0, 0.0, 0.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |  GL_STENCIL_BUFFER_BIT);

    for (auto &[_, scene] : world.scenes){
      renderScene(scene, shaderProgram, projection, view, glm::mat4(1.0f), lights, portals);
    }

    Color pixelColor = getPixelColor(state.cursorLeft, state.cursorTop, state.currentScreenHeight);
    if (shouldCallItemSelected){
      schemeBindings.onObjectSelected(state.selectedIndex, glm::vec3(pixelColor.r, pixelColor.g, pixelColor.b));
      shouldCallItemSelected = false;
    }

    if (state.lastHoverIndex != state.currentHoverIndex){
      if (state.lastHoveredIdInScene){
        schemeBindings.onObjectHover(state.lastHoverIndex, false);
      }
      if (state.hoveredIdInScene){
        schemeBindings.onObjectHover(state.currentHoverIndex, true);
      }
    }
   

    glDisable(GL_STENCIL_TEST);

    if (showDebugInfo){
      displayRails(getRails(world.objectMapping));
      renderVector(shaderProgram, projection, view, glm::mat4(1.0f));
    }
    renderUI(crosshairSprite, currentFramerate, pixelColor);

    handleInput(disableInput, window, deltaTime, state, translate, scale, rotate, moveCamera, nextCamera, setObjectDimensions, onDebugKey, onArrowKey, schemeBindings.onCameraSystemChange, onDelete);
    
    glfwPollEvents();
    
    schemeBindings.onFrame();
    schemeBindings.onMessage(channelMessages);  // modifies the queue
    schemeBindings.onFloatMessage(channelFloatMessages);

    portalIdCache.clear();


    // depends on framebuffer texture, outputs to framebuffer texture 2
    // Blurring draws the framebuffer texture 
    // The blur program blurs it one in one direction and saves in framebuffer texture 3 
    // then we take framebuffer texture 3, and use that like the original framebuffer texture
    // run it through again, blurring in other fucking direction 
    // We swap to attachment 2 which was just the old bloom attachment for final render pass
    glUseProgram(blurProgram);
    glUniform1i(glGetUniformLocation(blurProgram, "firstpass"), true);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture3, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebufferTexture3, 0);

    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST | GL_STENCIL_TEST);

    glBindTexture(GL_TEXTURE_2D, framebufferTexture2);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
   
    glUniform1i(glGetUniformLocation(blurProgram, "firstpass"), false);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture2, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebufferTexture2, 0);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture3);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ///

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(state.showDepthBuffer ? depthProgram : framebufferProgram); 
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(glGetUniformLocation(framebufferProgram, "enableBloom"), state.enableBloom);
    glUniform1f(glGetUniformLocation(framebufferProgram, "bloomAmount"), state.bloomAmount);
    glUniform1i(glGetUniformLocation(framebufferProgram, "bloomTexture"), 1);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture2);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(framebufferProgram, "framebufferTexture"), 0);

    if (state.portalTextureIndex == 0 || (state.textureDisplayMode && textureToPaint == -1)){
      glBindTexture(GL_TEXTURE_2D, state.showDepthBuffer ? depthTextures[1] : framebufferTexture);
    }else{
      if (state.textureDisplayMode){
        glBindTexture(GL_TEXTURE_2D, textureToPaint);
      }else{
        assert(state.portalTextureIndex <= numPortalTextures);
        glBindTexture(GL_TEXTURE_2D, portalTextures[state.portalTextureIndex - 1]);  // new code
      }
    }
    glDrawArrays(GL_TRIANGLES, 0, 6);
    if (state.takeScreenshot){
      state.takeScreenshot = false;
      saveScreenshot();
    }
  }

  std::cout << "LIFECYCLE: program exiting" << std::endl;
  
  cleanup:    
    deinitPhysics(world.physicsEnvironment); 
    stopSoundSystem();
    glfwTerminate(); 
   
  return 0;
}
