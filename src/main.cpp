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
#include "./scene/collision_cache.h"
#include "./scene/scenegraph.h"
#include "./scene/worldloader.h"
#include "./scene/object_types.h"
#include "./scene/common/mesh.h"
#include "./scene/common/vectorgfx.h"
#include "./scene/common/util/loadmodel.h"
#include "./scene/common/util/boundinfo.h"
#include "./scene/sprites/readfont.h"
#include "./scene/sprites/sprites.h"
#include "./scene/voxels.h"
#include "./scene/animation/timeplayback.h"
#include "./scene/animation/animation.h"
#include "./scheme_bindings.h"
#include "./shaders.h"
#include "./translations.h"
#include "./sound.h"
#include "./common/util.h"
#include "./colorselection.h"
#include "./state.h"
#include "./input.h"
#include "./network.h"

GameObject* activeCameraObj;
GameObject defaultCamera = GameObject {
  .id = -1,
  .name = "defaultCamera",
  .transformation = Transformation {
    .position = glm::vec3(0.f, -2.f, 0.f),
    .scale = glm::vec3(1.0f, 1.0f, 1.0f),
    .rotation = glm::quat(0, 1, 0, 0.0f),
  }
};

bool showDebugInfo = false;
bool disableInput = false;
int numChunkingGridCells = 0;
float chunkSize = 100;
bool useChunkingSystem = false;
std::string rawSceneFile;
Texture blacktopTexture;
Texture grassTexture;

GameObjectVoxel* voxelPtr;
short voxelPtrId = -1;
glm::mat4 voxelPtrModelMatrix = glm::mat4(1.f);

engineState state = getDefaultState(1920, 1080);
World world;
DynamicLoading dynamicLoading;
std::vector<Line> lines;
std::vector<Line> permaLines;

std::map<unsigned int, Mesh> fontMeshes;

glm::mat4 projection;
glm::mat4 view;
unsigned int framebufferTexture;
unsigned int fbo;
unsigned int depthTextures[32];

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

const int numTextures = 32;
int activeDepthTexture = 0;

void updateDepthTexturesSize(){
  for (int i = 0; i < numTextures; i++){
    glBindTexture(GL_TEXTURE_2D, depthTextures[i]);
    glTexImage2D(GL_TEXTURE_2D, 0,  GL_DEPTH_COMPONENT32F, state.currentScreenWidth, state.currentScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
}


void setActiveCamera(short cameraId){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(world.objectMapping);
  if (! (std::find(cameraIndexs.begin(), cameraIndexs.end(), cameraId) != cameraIndexs.end())){
    std::cout << "index: " << cameraId << " is not a valid index" << std::endl;
    return;
  }
  auto sceneId = world.idToScene.at(cameraId);
  activeCameraObj = &world.scenes.at(sceneId).scene.idToGameObjects.at(cameraId);
  state.selectedIndex = cameraId;
}
void nextCamera(){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(world.objectMapping);
  if (cameraIndexs.size() == 0){  // if we do not have a camera in the scene, we use default
    state.useDefaultCamera = true;    
    activeCameraObj = NULL;
    return;
  }

  state.activeCamera = (state.activeCamera + 1) % cameraIndexs.size();
  short activeCameraId = cameraIndexs.at(state.activeCamera);
  setActiveCamera(activeCameraId);
  std::cout << "active camera is: " << state.activeCamera << std::endl;
}
void moveCamera(glm::vec3 offset){
  defaultCamera.transformation.position = moveRelative(defaultCamera.transformation.position, defaultCamera.transformation.rotation, glm::vec3(offset));
}
void rotateCamera(float xoffset, float yoffset){
  defaultCamera.transformation.rotation = setFrontDelta(defaultCamera.transformation.rotation, xoffset, yoffset, 0, 1);
}
void drawText(std::string word, float left, float top, unsigned int fontSize){
  drawWords(uiShaderProgram, fontMeshes, word, left, top, fontSize);
}

//////////

void playSound(){
  playSound(soundBuffer);
  //activeDepthTexture = (activeDepthTexture + 1) % numTextures;
  //setActiveDepthTexture(activeDepthTexture);
}
void printSceneGraphAsDot(){
  for (auto [id, scene] : world.scenes){
    std::cout << scenegraphAsDotFormat(scene.scene, world.objectMapping) << std::endl;
  }
}

auto targetModel = "./res/models/bendybox/sentinal.dae";
Animation getTargetAnimation(){
  return world.animations.at(targetModel).at(0);
}

void processNewPoseOnMesh(std::map<std::string, glm::mat4>& nodeToTransformedPose, std::string boneName, glm::mat4 newPose, NameAndMesh& meshData){
  for (int i = 0; i < meshData.meshes.size(); i++){
    Mesh& mesh = meshData.meshes.at(i);
    for (Bone& bone : mesh.bones){
      if (bone.name == boneName){    
        nodeToTransformedPose[bone.name] = newPose;
      }
    }
  }
}

glm::mat4 getParentBone(std::map<std::string, std::string>& boneToParent, std::map<std::string, glm::mat4>& nodeToTransformedPose, std::string boneName){
  if (boneToParent.find(boneName) != boneToParent.end()){
    return nodeToTransformedPose.at(boneToParent.at(boneName));
  }
  return glm::mat4(1.f);
} 

TimePlayback timePlayback(glfwGetTime(), [](float currentTime, float elapsedTime) -> void {
  auto animation = getTargetAnimation();
  std::cout << "animation name:  " << animation.name << std::endl;
  auto meshNameToMeshes = getMeshesForId(world.objectMapping, 9);   // @TODO - this currently just uses all meshes for the 5th item, which only maps to target animation in the specific scene
  
  std::map<std::string, glm::mat4> nodeToTransformedPose;
  advanceAnimation(animation, currentTime, elapsedTime, [&meshNameToMeshes, &nodeToTransformedPose](std::string boneName, glm::mat4 newPose) -> void {
    processNewPoseOnMesh(nodeToTransformedPose, boneName, newPose, meshNameToMeshes);
  });

  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    std::string meshName = meshNameToMeshes.meshNames.at(i);
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    for (Bone& bone : mesh.bones){
      if (nodeToTransformedPose.find(bone.name) != nodeToTransformedPose.end()){
        bone.offsetMatrix = getParentBone(world.meshnameToBoneToParent.at(meshName), nodeToTransformedPose, bone.name) * nodeToTransformedPose.at(bone.name) * bone.initialOffsetMatrix;
      }
    }
  }
},4); 

bool useYAxis = true;
void onDebugKey(){
  useYAxis = !useYAxis;

  playSound();

  if (timePlayback.isPaused()){
    timePlayback.play();
  }else{
    timePlayback.pause();
  }
}

void expandVoxelUp(){
  if (voxelPtr == NULL){
    return;
  }
  expandVoxels(voxelPtr -> voxel, 0, useYAxis ? -1 : 0, !useYAxis ? -1 : 0);
}
void expandVoxelDown(){
  if (voxelPtr == NULL){
    return;
  }
  expandVoxels(voxelPtr -> voxel , 0, useYAxis ? 1 : 0, !useYAxis ? 1 : 0);
}
void expandVoxelLeft(){
  if (voxelPtr == NULL){
    return;
  }
  expandVoxels(voxelPtr -> voxel, -1, 0, 0);
}
void expandVoxelRight(){
  if (voxelPtr == NULL){
    return;
  }
  expandVoxels(voxelPtr -> voxel, 1, 0, 0);
}

void onArrowKey(int key){
  std::cout << "on arrow key pressed: " << key << std::endl;
  if (key == GLFW_KEY_LEFT){
    expandVoxelLeft();
  }
  if (key == GLFW_KEY_RIGHT){
    expandVoxelRight();
  }
  if (key == GLFW_KEY_UP){
    expandVoxelUp();
  }
  if (key == GLFW_KEY_DOWN){
    expandVoxelDown();
  }
}

void handleSerialization(){     // @todo handle serialization for multiple scenes.  Probably be smart about which scene to serialize and then save that chunk
  playSound();
  printSceneGraphAsDot();

  auto rayDirection = getCursorRayDirection(projection, view, state.cursorLeft, state.cursorTop, state.currentScreenWidth, state.currentScreenHeight);

  Line line = {
    .fromPos = defaultCamera.transformation.position,
    .toPos = glm::vec3(rayDirection.x * 1000, rayDirection.y * 1000, rayDirection.z * 1000),
  };
 
  permaLines.clear();
  permaLines.push_back(line);

  if (voxelPtr == NULL){
    return;
  }

  glm::vec4 fromPosModelSpace = glm::inverse(voxelPtrModelMatrix) * glm::vec4(line.fromPos.x, line.fromPos.y, line.fromPos.z, 1.f);
  glm::vec4 toPos =  glm::vec4(line.fromPos.x, line.fromPos.y, line.fromPos.z, 1.f) + glm::vec4(rayDirection.x * 1000, rayDirection.y * 1000, rayDirection.z * 1000, 1.f);
  glm::vec4 toPosModelSpace = glm::inverse(voxelPtrModelMatrix) * toPos;
  glm::vec3 rayDirectionModelSpace =  toPosModelSpace - fromPosModelSpace;


  // This raycast happens in model space of voxel, so specify position + ray in voxel model space
  auto collidedVoxels = raycastVoxels(voxelPtr -> voxel, fromPosModelSpace, rayDirectionModelSpace);
  std::cout << "length is: " << collidedVoxels.size() << std::endl;
  if (collidedVoxels.size() > 0){
    auto collision = collidedVoxels[0];
    voxelPtr -> voxel.selectedVoxels.push_back(collision);
    applyTextureToCube(voxelPtr -> voxel, voxelPtr -> voxel.selectedVoxels, 2);
  }

  std::cout << "num voxels selected: " << voxelPtr -> voxel.selectedVoxels.size() << "(" << voxelPtr << ")" << std::endl;
  // TODO - serialization is broken since didn't keep up with it   
  /*int sceneToSerialize = world.scenes.size() - 1;
  if (sceneToSerialize >= 0){
  //  std::cout << serializeFullScene(world.scenes.begin()->second.scene, world.objectMapping) << std::endl;
  }*/
}

void selectItem(){
  Color pixelColor = getPixelColor(state.cursorLeft, state.cursorTop, state.currentScreenHeight);
  auto selectedId = getIdFromColor(pixelColor.r, pixelColor.g, pixelColor.b);

  if (world.idToScene.find(selectedId) == world.idToScene.end()){
    std::cout << "ERROR: Color management: selected a color id that isn't in the scene" << std::endl;
    return;
  }

  state.selectedIndex = selectedId;

  auto sceneId = world.idToScene.at(state.selectedIndex);
  state.selectedName = world.scenes.at(sceneId).scene.idToGameObjects.at(state.selectedIndex).name + "(" + std::to_string(state.selectedIndex) + ")";
  state.additionalText = "     <" + std::to_string((int)(255 * pixelColor.r)) + ","  + std::to_string((int)(255 * pixelColor.g)) + " , " + std::to_string((int)(255 * pixelColor.b)) + ">  " + " --- " + state.selectedName;
  schemeBindings.onObjectSelected(state.selectedIndex);
}
void processManipulator(){
  if (state.enableManipulator && state.selectedIndex != -1 && !(world.idToScene.find(state.selectedIndex) == world.idToScene.end())){
    auto sceneId = world.idToScene.at(state.selectedIndex);
    auto selectObject = world.scenes.at(sceneId).scene.idToGameObjects.at(state.selectedIndex);
    if (state.manipulatorMode == TRANSLATE){
      applyPhysicsTranslation(world.scenes.at(sceneId), world.rigidbodys.at(state.selectedIndex), state.selectedIndex, selectObject.transformation.position, state.offsetX, state.offsetY, state.manipulatorAxis);
    }else if (state.manipulatorMode == SCALE){
      applyPhysicsScaling(world, world.scenes.at(sceneId), world.rigidbodys.at(state.selectedIndex), state.selectedIndex, selectObject.transformation.position, selectObject.transformation.scale, state.lastX, state.lastY, state.offsetX, state.offsetY, state.manipulatorAxis);
    }else if (state.manipulatorMode == ROTATE){
      applyPhysicsRotation(world.scenes.at(sceneId), world.rigidbodys.at(state.selectedIndex), state.selectedIndex, selectObject.transformation.rotation, state.offsetX, state.offsetY, state.manipulatorAxis);
    }
  }
}

void onMouseEvents(GLFWwindow* window, double xpos, double ypos){
  onMouse(disableInput, window, state, xpos, ypos, rotateCamera);
  processManipulator();
}
void onMouseCallback(GLFWwindow* window, int button, int action, int mods){
  mouse_button_callback(disableInput, window, state, button, action, mods, handleSerialization, selectItem);
  schemeBindings.onMouseCallback(button, action, mods);

  if (button == 0 && voxelPtr != NULL){
    voxelPtr -> voxel.selectedVoxels.clear();
  }
}

int textureId = 0;
void onScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
  scroll_callback(window, state, xoffset, yoffset);

  if (voxelPtr == NULL){
    return;
  }
  
  if (yoffset > 0){
    textureId += 1;
    applyTextureToCube(voxelPtr -> voxel, voxelPtr -> voxel.selectedVoxels, textureId);
  }
  if (yoffset < 0){
    textureId -= 1;
    applyTextureToCube(voxelPtr -> voxel, voxelPtr -> voxel.selectedVoxels, textureId);
  }
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
  schemeBindings.onKeyCallback(key, scancode, action, mods);

  // temp hackey for voxels
  if (key == 261 && voxelPtr != NULL){  // delete
    removeVoxel(voxelPtr -> voxel, voxelPtr -> voxel.selectedVoxels);
    voxelPtr -> voxel.selectedVoxels.clear();
  } 
  ////////////////

}
void keyCharCallback(GLFWwindow* window, unsigned int codepoint){
  schemeBindings.onKeyCharCallback(codepoint); 
}
void translate(float x, float y, float z){
  if (state.selectedIndex == -1 || world.idToScene.find(state.selectedIndex) == world.idToScene.end()){
    return;
  }
  
  auto sceneId = world.idToScene.at(state.selectedIndex);
  physicsTranslate(world.scenes.at(sceneId), world.rigidbodys.at(state.selectedIndex), x, y, z, state.moveRelativeEnabled, state.selectedIndex);
}
void scale(float x, float y, float z){
  if (state.selectedIndex == -1 || world.idToScene.find(state.selectedIndex) == world.idToScene.end()){
    return;
  }
  auto sceneId = world.idToScene.at(state.selectedIndex);
  physicsScale(world, world.scenes.at(sceneId), world.rigidbodys.at(state.selectedIndex), state.selectedIndex, x, y, z);
}
void rotate(float x, float y, float z){
  if (state.selectedIndex == -1 || world.idToScene.find(state.selectedIndex) == world.idToScene.end()){
    return;
  }
  auto sceneId = world.idToScene.at(state.selectedIndex);
  physicsRotate(world.scenes.at(sceneId), world.rigidbodys.at(state.selectedIndex), x, y, z, state.selectedIndex);
}

void setObjectDimensions(short index, float width, float height, float depth){
  if (state.selectedIndex == -1 || world.idToScene.find(state.selectedIndex) == world.idToScene.end()){
    return;
  }
  auto gameObjV = world.objectMapping.at(state.selectedIndex);  // todo this is bs, need a wrapper around objectmappping + scene
  auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); 
  if (meshObj != NULL){
    // @TODO this is resizing based upon first mesh only, which is questionable
    auto newScale = getScaleEquivalent(meshObj -> meshesToRender.at(0).boundInfo, width, height, depth);   // this is correlated to logic in scene//getPhysicsInfoForGameObject, needs to be fixed
    std::cout << "new scale: (" << newScale.x << ", " << newScale.y << ", " << newScale.z << ")" << std::endl;
    auto sceneId = world.idToScene.at(state.selectedIndex);
    world.scenes.at(sceneId).scene.idToGameObjects.at(state.selectedIndex).transformation.scale = newScale;
  } 
}

void removeObjectById(short id){
  std::cout << "removing object by id: " << id << std::endl;
  removeObject(world.objectMapping, id);
  auto sceneId = world.idToScene.at(id);
  removeObjectFromScene(world.scenes.at(sceneId).scene, id);
}
void makeObject(std::string name, std::string meshName, float x, float y, float z){
  //addObjectToFullScene(world, 0, name, meshName, glm::vec3(x,y,z));
  std::cout << "make object called -- this doesn't work anymore with mutli scene" << std::endl;
}

std::vector<short> getObjectsByType(std::string type){
  if (type == "mesh"){
    std::vector indexes = getGameObjectsIndex<GameObjectMesh>(world.objectMapping);
    return indexes;
  }else if (type == "camera"){
    std::vector indexes = getGameObjectsIndex<GameObjectCamera>(world.objectMapping);
    return indexes;
  }
  return getGameObjectsIndex(world.objectMapping);
}

std::string getGameObjectName(short index){
  auto sceneId = world.idToScene.at(index);
  return world.scenes.at(sceneId).scene.idToGameObjects.at(index).name;
}
glm::vec3 getGameObjectPosition(short index){
  auto sceneId = world.idToScene.at(index);
  return world.scenes.at(sceneId).scene.idToGameObjects.at(index).transformation.position;
}
void setGameObjectPosition(short index, glm::vec3 pos){
  auto sceneId = world.idToScene.at(index);
  physicsTranslateSet(world.scenes.at(sceneId), world.rigidbodys.at(index), pos, index);
}
void setGameObjectRotation(short index, glm::quat rotation){
  auto sceneId = world.idToScene.at(index);
  physicsRotateSet(world.scenes.at(sceneId), world.rigidbodys.at(index), rotation,  index);
}
glm::quat getGameObjectRotation(short index){
  auto sceneId = world.idToScene.at(index);
  return world.scenes.at(sceneId).scene.idToGameObjects.at(index).transformation.rotation;
}
short getGameObjectByName(std::string name){    // @todo : odd behavior: currently these names do not have to be unique in different scenes.  this just finds first instance of that name.
  for (int i = 0; i < world.scenes.size(); i++){
    for (auto [id, gameObj]: world.scenes.at(i).scene.idToGameObjects){
      if (gameObj.name == name){
        return id;
      }
    }
  }
  return -1;
}

void setSelectionMode(bool enabled){
  state.isSelectionMode = enabled;
}

void applyImpulse(short index, glm::vec3 impulse){
  applyImpulse(world.rigidbodys.at(index), impulse);
}
void clearImpulse(short index){
  clearImpulse(world.rigidbodys.at(index));
}
short loadScene(std::string sceneFile){
  std::cout << "INFO: SCENE LOADING: loading " << sceneFile << std::endl;
  return addSceneToWorld(world, sceneFile);
}
void unloadScene(short sceneId){  
  std::cout << "INFO: SCENE LOADING: unloading " << sceneId << std::endl;
  removeSceneFromWorld(world, sceneId);
}
std::vector<short> listScenes(){
  std::vector<short> sceneIds;
  for (auto &[id, _] : world.scenes){
    sceneIds.push_back(id);
  }
  return sceneIds;
}

void printObjectIds(){
  auto ids = listObjInScene(world.scenes.at(0).scene);
  for (int i = 0; i < ids.size() ; i++){
    std::cout << "id: " << ids.at(i) << std::endl;
  }
}

void updateVoxelPtr(){
  auto voxelIndexes = getGameObjectsIndex<GameObjectVoxel>(world.objectMapping);
  if (voxelIndexes.size() > 0){
    short id = voxelIndexes.at(0);
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

glm::vec3 getPositionFromMatrix(glm::mat4 matrix){
  glm::vec3 scale;
  glm::quat rotation;
  glm::vec3 translation;
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::decompose(matrix, scale, rotation, translation, skew, perspective);
  return translation;  
}

std::vector<glm::vec3> traversalPositions;
std::vector<glm::vec3> parentTraversalPositions;
void addPositionToRender(glm::mat4 modelMatrix, glm::mat4 parentMatrix){
  traversalPositions.push_back(getPositionFromMatrix(modelMatrix));
  parentTraversalPositions.push_back(getPositionFromMatrix(parentMatrix));
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

void renderScene(FullScene& fullscene, GLint shaderProgram, glm::mat4 projection, glm::mat4 view,  glm::mat4 model, bool useSelectionColor, std::vector<GameObject*>& lights){
  glUseProgram(shaderProgram);
  
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, blacktopTexture.textureId);
  glUniform1i(glGetUniformLocation(shaderProgram, "coolzero"), 1);         // @todo just a sample extra texture, this will eventually become something real
  
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_2D, grassTexture.textureId);
  glUniform1i(glGetUniformLocation(shaderProgram, "coolsomevalue"), 2);

  glActiveTexture(GL_TEXTURE0); 

  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));    
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),  1, GL_FALSE, glm::value_ptr(view));
  glUniform3fv(glGetUniformLocation(shaderProgram, "cameraPosition"), 1, glm::value_ptr(defaultCamera.transformation.position));
  glUniform1i(glGetUniformLocation(shaderProgram, "enableDiffuse"), state.enableDiffuse);
  glUniform1i(glGetUniformLocation(shaderProgram, "enableSpecular"), state.enableSpecular);

  glUniform1i(glGetUniformLocation(shaderProgram, "numlights"), lights.size());
  for (int i = 0; i < lights.size(); i++){
    glm::vec3 position = lights.at(i) -> transformation.position;
    glUniform3fv(glGetUniformLocation(shaderProgram, ("lights[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(position));
  }

  clearTraversalPositions();
  traverseScene(fullscene.scene, [useSelectionColor, shaderProgram, &fullscene](short id, glm::mat4 modelMatrix, glm::mat4 parentModelMatrix) -> void {    
    assert(id >= 0);
    if (id == voxelPtrId){
      voxelPtrModelMatrix = modelMatrix;
    }
    
    GameObject object = fullscene.scene.idToGameObjects.at(id);
    bool objectSelected = state.selectedIndex == id;
    glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(getColorFromGameobject(object, useSelectionColor, objectSelected)));

    if (state.visualizeNormals){
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
      drawMesh(world.meshes.at("./res/models/cone/cone.obj")); 
    }

    // bounding code //////////////////////
    auto gameObjV = world.objectMapping.at(id);
    auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); 
    if (meshObj != NULL){
      // @TODO i use first mesh to get sizing for bounding box, obviously that's questionable
      auto bounding = getBoundRatio(world.meshes.at("./res/models/boundingbox/boundingbox.obj").boundInfo, meshObj -> meshesToRender.at(0).boundInfo);
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(getMatrixForBoundRatio(bounding, modelMatrix)));

      if (objectSelected){
        drawMesh(world.meshes.at("./res/models/boundingbox/boundingbox.obj"));
      }
    }
    /////////////////////////////// end bounding code

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    renderObject(
      shaderProgram, 
      id, 
      world.objectMapping, 
      world.meshes.at("./res/models/ui/node.obj"), 
      objectSelected, 
      world.meshes.at("./res/models/boundingbox/boundingbox.obj"), 
      state.showCameras, 
      modelMatrix,
      state.showBoneWeight,
      state.useBoneTransform
    );

    addPositionToRender(modelMatrix, parentModelMatrix);
  });
  drawTraversalPositions();
}

void renderVector(GLint shaderProgram, glm::mat4 projection, glm::mat4 view, glm::mat4 model){
  glUseProgram(shaderProgram);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));    
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),  1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.05, 0.f, 1.f)));

  if (numChunkingGridCells > 0){
    float offset = ((numChunkingGridCells % 2) == 0) ? (dynamicLoading.chunkXWidth / 2) : 0;
    drawGrid3DCentered(numChunkingGridCells, dynamicLoading.chunkXWidth, offset, offset, offset);

    glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.05, 1.f, 1.f)));
    drawCoordinateSystem(100.f);
  }

  glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.05f, 1.f, 0.f)));
  if (permaLines.size() > 0){
    drawLines(permaLines);
  }
  if (lines.size() > 0){
   drawLines(lines);
  
  }
  lines.clear();
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
    drawText("position: " + print(defaultCamera.transformation.position), 10, 60, 3);
    drawText("fov: " + std::to_string(state.fov), 10, 70, 3);
    drawText("cursor: " + std::to_string(state.cursorLeft) + " / " + std::to_string(state.cursorTop)  + "(" + std::to_string(state.currentScreenWidth) + "||" + std::to_string(state.currentScreenHeight) + ")", 10, 80, 3);
  
    if (state.selectedIndex != -1){
      auto obj = world.scenes.at(world.idToScene.at(state.selectedIndex)).scene.idToGameObjects.at(state.selectedIndex);
      drawText("position: " + print(obj.transformation.position), 10, 90, 3);
      drawText("scale: " + print(obj.transformation.scale), 10, 100, 3);
      drawText("rotation: " + print(obj.transformation.rotation), 10, 110, 3);
    }
    
    Color pixelColor = getPixelColor(state.cursorLeft, state.cursorTop, state.currentScreenHeight);
    drawText("pixel color: " + std::to_string(pixelColor.r) + " " + std::to_string(pixelColor.g) + " " + std::to_string(pixelColor.b), 10, 130, 3);
    drawText("showing color: " + std::string(state.showBoneWeight ? "bone weight" : "bone indicies") , 10, 140, 3);

  }
}

void onData(std::string data){
  std::cout << "got data: " << data << std::endl;
}
void sendMoveObjectMessage(){
  sendMessage((char*)"hello world");
}

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2){
  schemeBindings.onCollisionEnter(getIdForCollisionObject(world, obj1), getIdForCollisionObject(world, obj2));
}
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2){
  schemeBindings.onCollisionExit(getIdForCollisionObject(world, obj1), getIdForCollisionObject(world, obj2));
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
   ("b,bounds", "Show bounds of colliders for physics entities", cxxopts::value<bool>()->default_value("false"))
   ("p,physics", "Enable physics", cxxopts::value<bool>()->default_value("false"))
   ("y,debugphysics", "Enable physics debug drawing", cxxopts::value<bool>()->default_value("false"))
   ("n,noinput", "Disable default input (still allows custom input handling in scripts)", cxxopts::value<bool>()->default_value("false"))
   ("g,grid", "Size of grid chunking grid used for open world streaming, default to zero (no grid)", cxxopts::value<int>()->default_value("0"))
   ("e,chunksize", "Size of worlds chunks", cxxopts::value<float>()->default_value("100"))
   ("w,world", "Use streaming chunk system", cxxopts::value<bool>()->default_value("false"))
   ("r,rawscene", "Rawscene file to use (only used when world = false)", cxxopts::value<std::string>()->default_value("./res/scenes/example.rawscene"))
   ("h,help", "Print help")
  ;        

  const auto result = cxxoption.parse(argc, argv);
  bool dumpPhysics = result["dumpphysics"].as<bool>();
  numChunkingGridCells = result["grid"].as<int>();
  useChunkingSystem = result["world"].as<bool>();
  chunkSize = result["chunksize"].as<float>();
  rawSceneFile = result["rawscene"].as<std::string>();

  if (result["help"].as<bool>()){
    std::cout << cxxoption.help() << std::endl;
    return 0;
  }
  bool enablePhysics = result["physics"].as<bool>();
  bool showPhysicsColliders = result["bounds"].as<bool>();

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
  soundBuffer = loadSound("./res/sounds/sample.wav");

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);  

  glGenTextures(1, &framebufferTexture);
  glBindTexture(GL_TEXTURE_2D, framebufferTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, state.currentScreenWidth, state.currentScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

  generateDepthTextures();
  setActiveDepthTexture(0);

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

     updateDepthTexturesSize();

     glViewport(0, 0, state.currentScreenWidth, state.currentScreenHeight);
     orthoProj = glm::ortho(0.0f, (float)state.currentScreenWidth, (float)state.currentScreenHeight, 0.0f, -1.0f, 1.0f);  
  }; 

  onFramebufferSizeChange(window, state.currentScreenWidth, state.currentScreenHeight);
  glfwSetFramebufferSizeCallback(window, onFramebufferSizeChange); 
  
  std::cout << "INFO: shader file path is " << shaderFolderPath << std::endl;
  unsigned int shaderProgram = loadShader(shaderFolderPath + "/vertex.glsl", shaderFolderPath + "/fragment.glsl");
  
  std::cout << "INFO: framebuffer file path is " << framebufferTexturePath << std::endl;
  unsigned int framebufferProgram = loadShader(framebufferTexturePath + "/vertex.glsl", framebufferTexturePath + "/fragment.glsl");

  std::string depthShaderPath = "./res/shaders/depth";
  std::cout << "INFO: depth file path is " << depthShaderPath << std::endl;
  unsigned int depthProgram = loadShader(depthShaderPath + "/vertex.glsl", depthShaderPath + "/fragment.glsl");

  std::cout << "INFO: ui shader file path is " << uiShaderPath << std::endl;
  uiShaderProgram = loadShader(uiShaderPath + "/vertex.glsl",  uiShaderPath + "/fragment.glsl");

  std::string selectionShaderPath = "./res/shaders/selection";
  std::cout << "INFO: selection shader path is " << selectionShaderPath << std::endl;
  unsigned int selectionProgram = loadShader(selectionShaderPath + "/vertex.glsl", selectionShaderPath + "/fragment.glsl");

  font fontToRender = readFont(result["font"].as<std::string>());
  fontMeshes = loadFontMeshes(fontToRender);
  Mesh crosshairSprite = loadSpriteMesh(result["crosshair"].as<std::string>());

  schemeBindings  = createStaticSchemeBindings(
    result["scriptpath"].as<std::string>(), 
    loadScene,
    unloadScene,
    listScenes,
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
    getGameObjectRotation,
    setGameObjectRotation,
    setFrontDelta,
    getGameObjectByName,
    setSelectionMode,
    applyImpulse,
    clearImpulse
  );

  BulletDebugDrawer drawer(addLineNextCycle);
  btIDebugDraw* debuggerDrawer = result["debugphysics"].as<bool>() ?  &drawer : NULL;

  world = createWorld(onObjectEnter, onObjectLeave, debuggerDrawer);

  dynamicLoading = createDynamicLoading(chunkSize);
  if (!useChunkingSystem){
    loadScene(rawSceneFile);
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

  blacktopTexture = loadTexture("./res/textures/blacktop.jpg");
  grassTexture = loadTexture("./res/textures/grass.png");

  if (result["skiploop"].as<bool>()){
    goto cleanup;
  }

  //glEnable(GL_CULL_FACE);  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  while (!glfwWindowShouldClose(window)){
    frameCount++;
    float now = glfwGetTime();
    deltaTime = now - previous;   
    previous = now;
    timePlayback.setElapsedTime(deltaTime);

    if (frameCount == 60){
      frameCount = 0;
      float timedelta = now - last60;
      last60 = now;
      currentFramerate = (int)60/(timedelta);
      //printObjectIds();
    }

    if (isServer){
      getDataFromSocket(serverInstance, onData);
    }

    if (state.useDefaultCamera || activeCameraObj == NULL){
      view = renderView(defaultCamera.transformation.position, defaultCamera.transformation.rotation);
    }else{
      view = renderView(activeCameraObj -> transformation.position, activeCameraObj -> transformation.rotation);   // this is position incorrect because this needs to traverse the object hierachy
    }
    projection = glm::perspective(glm::radians(state.fov), (float)state.currentScreenWidth / state.currentScreenHeight, 0.1f, 1000.0f); 

    glfwSwapBuffers(window);
    
    auto lightsIndexs = getGameObjectsIndex<GameObjectLight>(world.objectMapping);
    std::vector<GameObject*> lights;
    for (int i = 0; i < lightsIndexs.size(); i++){
      lights.push_back(&world.scenes.at(world.idToScene.at(lightsIndexs.at(i))).scene.idToGameObjects.at(lightsIndexs.at(i)));
    }

    updateVoxelPtr();   // this should be removed.  This basically picks a voxel id to be the one we work on. Better would to just have some way to determine this (like with the core selection mechanism)

    setActiveDepthTexture(1);

    // depth buffer form point of view of 1 light source (all eventually, but 1 for now)

    auto lightPosition = lights.size() > 0 ? lights.at(0) -> transformation.position : glm::vec3(0, 0, 0);
    auto lightRotation = lights.size() > 0 ? lights.at(0) -> transformation.rotation : glm::identity<glm::quat>();
    auto lightView = renderView(lightPosition, lightRotation);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glClearColor(255.0, 255.0, 255.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto &[_, scene] : world.scenes){
      renderScene(scene, selectionProgram, projection, lightView, glm::mat4(1.0f), true, lights);    // selection program since it's lightweight and we just care about depth buffer
    }

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "worldtolight"), 1, GL_FALSE, glm::value_ptr(lightView));  // leftover from shadow mapping attempt, will revisit

    setActiveDepthTexture(0);

    // 1ST pass draws selection program shader to be able to handle selection 
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1, 0.1, 0.1, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for (auto &[_, scene] : world.scenes){
      renderScene(scene, selectionProgram, projection, view, glm::mat4(1.0f), true, lights);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(framebufferProgram); 
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
        
    if (useChunkingSystem){
      handleChunkLoading(dynamicLoading, defaultCamera.transformation.position.x, defaultCamera.transformation.position.y, defaultCamera.transformation.position.z, loadScene, unloadScene);
    }
    if (enablePhysics){
      onPhysicsFrame(world, deltaTime, dumpPhysics); 
    }
    
    handleInput(disableInput, window, deltaTime, state, translate, scale, rotate, moveCamera, nextCamera, playSound, setObjectDimensions, sendMoveObjectMessage, makeObject, onDebugKey, onArrowKey);
    glfwPollEvents();

    // 2ND pass renders what we care about to the screen.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1, 0.1, 0.1, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
    for (auto &[_, scene] : world.scenes){
      renderScene(scene, shaderProgram, projection, view, glm::mat4(1.0f), false, lights);
    }

    renderVector(shaderProgram, projection, view, glm::mat4(1.0f));
    renderUI(crosshairSprite, currentFramerate);

    schemeBindings.onFrame();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(state.showDepthBuffer ? depthProgram : framebufferProgram); 
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, state.showDepthBuffer ? depthTextures[1] : framebufferTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  std::cout << "LIFECYCLE: program exiting" << std::endl;
  
  cleanup:    
    deinitPhysics(world.physicsEnvironment); 
    cleanupSocket(serverInstance);
    stopSoundSystem();
    glfwTerminate(); 
   
  return 0;
}
