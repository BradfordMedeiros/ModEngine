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
#include "./scene/rails.h"
#include "./easy_use.h"
#include "./scene/recorder.h"
#include "./sequencer.h"
#include "./keymapper.h"

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
bool disableInput = false;
int numChunkingGridCells = 0;
float chunkSize = 100;
bool useChunkingSystem = false;
std::string rawSceneFile;
bool bootStrapperMode = false;
NetCode netcode { };

GameObjectVoxel* voxelPtr;
short voxelPtrId = -1;
glm::mat4 voxelPtrModelMatrix = glm::mat4(1.f);

engineState state = getDefaultState(1920, 1080);
World world;
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
unsigned int fbo;
unsigned int depthTextures[32];

glm::mat4 orthoProj;
unsigned int uiShaderProgram;

SchemeBindingCallbacks schemeBindings;
std::queue<std::string> channelMessages;
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

//////////

std::vector<short> playbacksToRemove;
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
  if (key == GLFW_KEY_LEFT){
    //expandVoxelLeft();
  }
  if (key == GLFW_KEY_RIGHT){
    //expandVoxelRight();
  }
  if (key == GLFW_KEY_UP){
    //expandVoxelUp();
  }
  if (key == GLFW_KEY_DOWN){
    //expandVoxelDown();
  }
}
void onDelete(){
  if (state.selectedIndex != -1){
    std::cout << "OnDelete object id: " << state.selectedIndex << std::endl;
    removeObjectById(state.selectedIndex);
    state.selectedIndex = -1;
  }
}

void handleSerialization(){     // @todo handle serialization for multiple scenes.  Probably be smart about which scene to serialize and then save that chunk  
  for (auto [id, scene] : world.scenes){
    std::cout << scenegraphAsDotFormat(scene, world.objectMapping) << std::endl;
  }
/*
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
  */
  // TODO - serialization is broken since didn't keep up with it   use to be here but obviously this needs to have a real api

}

bool selectItemCalled = false;

void selectItem(){
  if (!showDebugInfo){
    return;
  }
  Color pixelColor = getPixelColor(state.cursorLeft, state.cursorTop, state.currentScreenHeight);
  auto selectedId = getIdFromColor(pixelColor);

  if (world.idToScene.find(selectedId) == world.idToScene.end()){
    std::cout << "ERROR: Color management: selected a color id that isn't in the scene" << std::endl;
    return;
  }

  Scene& scene = world.scenes.at(world.idToScene.at(selectedId));
  auto actualSelectedObject = scene.idToGameObjectsH.at(selectedId);
  auto selectedObject = scene.idToGameObjects.at(actualSelectedObject.groupId);
  state.selectedIndex = actualSelectedObject.groupId;

  state.selectedName = selectedObject.name + "(" + std::to_string(state.selectedIndex) + ")";
  state.additionalText = "     <" + std::to_string((int)(255 * pixelColor.r)) + ","  + std::to_string((int)(255 * pixelColor.g)) + " , " + std::to_string((int)(255 * pixelColor.b)) + ">  " + " --- " + state.selectedName;
  schemeBindings.onObjectSelected(state.selectedIndex);
}
void processManipulator(){
  if (state.enableManipulator && state.selectedIndex != -1 && !(world.idToScene.find(state.selectedIndex) == world.idToScene.end())){
    auto sceneId = world.idToScene.at(state.selectedIndex);
    auto selectObject = getGameObject(world, state.selectedIndex); 
    if (state.manipulatorMode == TRANSLATE){
      applyPhysicsTranslation(world, state.selectedIndex, selectObject.transformation.position, state.offsetX, state.offsetY, state.manipulatorAxis);
    }else if (state.manipulatorMode == SCALE){
      applyPhysicsScaling(world, state.selectedIndex, selectObject.transformation.position, selectObject.transformation.scale, state.lastX, state.lastY, state.offsetX, state.offsetY, state.manipulatorAxis);
    }else if (state.manipulatorMode == ROTATE){
      applyPhysicsRotation(world, state.selectedIndex, selectObject.transformation.rotation, state.offsetX, state.offsetY, state.manipulatorAxis);
    }
  }
}
void onMouseEvents(GLFWwindow* window, double xpos, double ypos){
  onMouse(disableInput, window, state, xpos, ypos, rotateCamera);  
  schemeBindings.onMouseMoveCallback(state.offsetX, state.offsetY); 
  processManipulator();
}
void onMouseCallback(GLFWwindow* window, int button, int action, int mods){
  mouse_button_callback(disableInput, window, state, button, action, mods, handleSerialization);
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
    selectItemCalled = true;
  }

  schemeBindings.onMouseCallback(button, action, mods);

  if (button == 0 && voxelPtr != NULL){
    voxelPtr -> voxel.selectedVoxels.clear();
  }
}

void maybeApplyTextureOffset(int index, glm::vec2 offset){
  GameObjectMesh* meshObj = std::get_if<GameObjectMesh>(&world.objectMapping.at(index));
  if (meshObj == NULL){
    return;
  }

  Scene& scene =  world.scenes.at(world.idToScene.at(index)); 
  auto groupId = scene.idToGameObjectsH.at(index).groupId;
  auto ids = getIdsInGroup(scene, groupId);

  bool rootIdHasMesh = false;
  for (auto id : ids){
    GameObjectMesh* meshObj = std::get_if<GameObjectMesh>(&world.objectMapping.at(id));
    assert(meshObj != NULL);
    meshObj -> textureoffset = meshObj -> textureoffset + offset;
  }
}

struct TextureAndName {
  Texture texture;
  std::string textureName;
};
std::vector<TextureAndName> worldTextures(World& world){
  std::vector<TextureAndName> textures;
  for (auto [textureName, texture] : world.textures){
    textures.push_back(TextureAndName{
      .texture = texture,
      .textureName = textureName
    });
  }
  return textures;
}
void maybeChangeTexture(int index){
    GameObjectMesh* meshObj = std::get_if<GameObjectMesh>(&world.objectMapping.at(index));
    if (meshObj == NULL){
      return;
    }
    Scene& scene =  world.scenes.at(world.idToScene.at(index)); 
    auto groupId = scene.idToGameObjectsH.at(index).groupId;
    auto ids = getIdsInGroup(scene, groupId);   

    auto textures = worldTextures(world);
    for (auto id : ids){
      GameObjectMesh* meshObj = std::get_if<GameObjectMesh>(&world.objectMapping.at(id));
      assert(meshObj != NULL);
      std::string currentOverload = meshObj -> textureOverloadName;
      bool hasOverload = currentOverload != "";

      int overloadId = 0;
      for (int i = 0; i < textures.size(); i++){
        if (meshObj -> textureOverloadId == textures.at(i).texture.textureId){
          overloadId = (i + 1) % textures.size();
        }
      }
      meshObj -> textureOverloadName = textures.at(overloadId).textureName;
      meshObj -> textureOverloadId = textures.at(overloadId).texture.textureId;
    }
}

// This is wasteful, as obviously I shouldn't be loading in all the textures on load, but ok for now. 
// This shoiuld really just be creating a list of names, and then the cycle above should cycle between possible textures to load, instead of what is loaded 
void loadAllTextures(){
  for (auto texturePath : listFilesWithExtensions(textureFolderPath, { "png", "jpg" })){
    loadTextureWorld(world, texturePath);
  }
}

int textureId = 0;
void onScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
  scroll_callback(window, state, xoffset, yoffset);

  if (state.offsetTextureMode && state.selectedIndex != -1){
    float offsetAmount = yoffset * 0.001;
    maybeApplyTextureOffset(state.selectedIndex, glm::vec2(state.manipulatorAxis == YAXIS ? offsetAmount : 0, state.manipulatorAxis == YAXIS ? 0 : offsetAmount));
  }
  if (!state.offsetTextureMode && state.selectedIndex != -1 && world.idToScene.find(state.selectedIndex) != world.idToScene.end()){
    maybeChangeTexture(state.selectedIndex);
  }

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
  schemeBindings.onKeyCallback(getKeyRemapping(keyMapper, key), scancode, action, mods);
  if (key == 261 && voxelPtr != NULL){  // delete
    removeVoxel(voxelPtr -> voxel, voxelPtr -> voxel.selectedVoxels);
    voxelPtr -> voxel.selectedVoxels.clear();
  } 

  if (key == GLFW_KEY_LEFT && action == 1 && state.selectedIndex != -1){
    if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
      setGameObjectPosition(state.selectedIndex, snapTranslateUp(getGameObjectPosition(state.selectedIndex, false), state.manipulatorAxis));
    }else if (state.manipulatorMode == ROTATE){
      setGameObjectRotation(state.selectedIndex, snapAngleDown(getGameObjectRotation(state.selectedIndex, false), state.manipulatorAxis));
    }else if (state.manipulatorMode == SCALE){
      setGameObjectScale(state.selectedIndex, snapScaleDown(getGameObjectScale(state.selectedIndex), state.manipulatorAxis));
    }
  }
  if (key == GLFW_KEY_RIGHT && action == 1 && state.selectedIndex != -1){
    if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
      setGameObjectPosition(state.selectedIndex, snapTranslateDown(getGameObjectPosition(state.selectedIndex, false), state.manipulatorAxis));
    }else if (state.manipulatorMode == ROTATE){
      setGameObjectRotation(state.selectedIndex, snapAngleUp(getGameObjectRotation(state.selectedIndex, false), state.manipulatorAxis));
    }else if (state.manipulatorMode == SCALE){
      setGameObjectScale(state.selectedIndex, snapScaleUp(getGameObjectScale(state.selectedIndex), state.manipulatorAxis));
    }
  }
  if (key == GLFW_KEY_UP && action == 1 && state.selectedIndex != -1){
    if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
      setSnapTranslateUp();
    }else if (state.manipulatorMode == ROTATE){
      setSnapAngleUp();
    }else if (state.manipulatorMode == SCALE){
      setSnapScaleUp();
    }
  }
  if (key == GLFW_KEY_DOWN && action == 1 && state.selectedIndex != -1){
    if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
      setSnapTranslateDown();
    }else if (state.manipulatorMode == ROTATE){
      setSnapAngleDown();
    }else if (state.manipulatorMode == SCALE){
      setSnapScaleDown();
    }
  }
}
void keyCharCallback(GLFWwindow* window, unsigned int codepoint){
  schemeBindings.onKeyCharCallback(codepoint); 
}
void translate(float x, float y, float z){
  if (state.selectedIndex == -1 || world.idToScene.find(state.selectedIndex) == world.idToScene.end()){
    return;
  }
  physicsTranslate(world, state.selectedIndex, x, y, z, state.moveRelativeEnabled);
}
void scale(float x, float y, float z){
  if (state.selectedIndex == -1 || world.idToScene.find(state.selectedIndex) == world.idToScene.end()){
    return;
  }
  auto sceneId = world.idToScene.at(state.selectedIndex);
  physicsScale(world, state.selectedIndex, x, y, z);
}
void rotate(float x, float y, float z){
  if (state.selectedIndex == -1 || world.idToScene.find(state.selectedIndex) == world.idToScene.end()){
    return;
  }
  auto sceneId = world.idToScene.at(state.selectedIndex);
  physicsRotate(world, state.selectedIndex, x, y, z);
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
    world.scenes.at(sceneId).idToGameObjects.at(state.selectedIndex).transformation.scale = newScale;
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


void displayRails(std::map<short, RailConnection> railPairs){
  for (auto [id, rail] : railPairs){
    auto scene = world.scenes.at(world.idToScene.at(id));
    bluelines.push_back(Line {
      .fromPos = scene.idToGameObjects.at(scene.nameToId.at(rail.from)).transformation.position,
      .toPos = scene.idToGameObjects.at(scene.nameToId.at(rail.to)).transformation.position
    });
  }
}

void renderScene(Scene& scene, GLint shaderProgram, glm::mat4 projection, glm::mat4 view,  glm::mat4 model, bool useSelectionColor, std::vector<LightInfo>& lights){
  glUseProgram(shaderProgram);
  
  glUniform1i(glGetUniformLocation(shaderProgram, "maintexture"), 0);        
  glUniform1i(glGetUniformLocation(shaderProgram, "emissionTexture"), 1);
  glUniform1i(glGetUniformLocation(shaderProgram, "opacityTexture"), 2);

  glActiveTexture(GL_TEXTURE0); 

  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));    
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),  1, GL_FALSE, glm::value_ptr(view));
  glUniform3fv(glGetUniformLocation(shaderProgram, "cameraPosition"), 1, glm::value_ptr(defaultCamera.transformation.position));
  glUniform1i(glGetUniformLocation(shaderProgram, "enableDiffuse"), state.enableDiffuse);
  glUniform1i(glGetUniformLocation(shaderProgram, "enableSpecular"), state.enableSpecular);

  glUniform1i(glGetUniformLocation(shaderProgram, "numlights"), lights.size());
  for (int i = 0; i < lights.size(); i++){
    glm::vec3 position = lights.at(i).pos;
    glUniform3fv(glGetUniformLocation(shaderProgram, ("lights[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(shaderProgram, ("lightscolor[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(lights.at(i).color));
  }

  clearTraversalPositions();
  traverseScene(scene, [useSelectionColor, shaderProgram, &scene, projection](short id, glm::mat4 modelMatrix, glm::mat4 parentModelMatrix, bool orthographic) -> void {
    if (orthographic){
     glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(glm::ortho(-1.f, 1.f, -1.f, 1.f, 0.f, 100.0f)));    
    }else{
     glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));    
    }

    assert(id >= 0);
    if (id == voxelPtrId){
      voxelPtrModelMatrix = modelMatrix;
    }
    
    GameObject object = scene.idToGameObjects.at(id);
    bool objectSelected = idInGroup(world, id, state.selectedIndex);
    glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(getColorFromGameobject(object.id, useSelectionColor, objectSelected)));

    if (state.visualizeNormals){
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
      drawMesh(world.meshes.at("./res/models/cone/cone.obj"), shaderProgram); 
    }

    // bounding code //////////////////////
    auto gameObjV = world.objectMapping.at(id);
    auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); 
    if (meshObj != NULL && meshObj -> meshesToRender.size() > 0){
      // @TODO i use first mesh to get sizing for bounding box, obviously that's questionable
      auto bounding = getBoundRatio(world.meshes.at("./res/models/boundingbox/boundingbox.obj").boundInfo, meshObj -> meshesToRender.at(0).boundInfo);
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(getMatrixForBoundRatio(bounding, modelMatrix)));

      if (objectSelected){
        drawMesh(world.meshes.at("./res/models/boundingbox/boundingbox.obj"), shaderProgram);
      }
    }
    /////////////////////////////// end bounding code

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glUniform1f(glGetUniformLocation(shaderProgram, "discardTexAmount"), state.discardAmount);
    renderObject(
      shaderProgram, 
      id, 
      world.objectMapping, 
      world.meshes.at("./res/models/ui/node.obj"),
      world.meshes.at("./res/models/camera/camera.dae"), 
      state.showCameras, 
      state.showBoneWeight,
      state.useBoneTransform
    );

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
    int numCells = 10;
    if (state.manipulatorMode == TRANSLATE){
      float snapGridSize = getSnapTranslateSize();
      if (snapGridSize > 0){
        auto position = getGameObjectPosition(state.selectedIndex, false);
        drawGrid3DCentered(numCells, snapGridSize, position.x, position.y, position.z);  
        glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.05, 1.f, 1.f)));     
      }
    }
  }
  drawCoordinateSystem(100.f);

  glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.05f, 1.f, 0.f)));
  if (permaLines.size() > 0){
  }
  if (lines.size() > 0){
   drawLines(lines);
  }

  glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.05f, 0.f, 1.f)));
  if (bluelines.size() > 0){
   drawLines(bluelines);
  }
  lines.clear();
  bluelines.clear();
}

void renderUI(Mesh& crosshairSprite, unsigned int currentFramerate){
  glUseProgram(uiShaderProgram);
  glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(orthoProj)); 

  if (!showDebugInfo){
    return;
  }
  if (!state.isSelectionMode){
     drawSpriteAround(uiShaderProgram, crosshairSprite, state.currentScreenWidth/2, state.currentScreenHeight/2, 40, 40);
  }else if (!state.isRotateSelection){
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
    
  Color pixelColor = getPixelColor(state.cursorLeft, state.cursorTop, state.currentScreenHeight);
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

  auto id = create.id;   
  if (idExists(world, id)){     // could conceptually do a comparison to see if it changed, but probably not
    std::cout << "INFO: id already exits: " << id << std::endl;
    return;
  }
  std::string serialobj = create.serialobj;
  assert(serialobj.size() > 0);
  auto newObjId = makeObject(serialobj, create.id, true);                        
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

int main(int argc, char* argv[]){
  cxxopts::Options cxxoption("ModEngine", "ModEngine is a game engine for hardcore fps");
  cxxoption.add_options()
   ("s,shader", "Folder path of default shader", cxxopts::value<std::string>()->default_value("./res/shaders/default"))
   ("t,texture", "Additional textures folder to use", cxxopts::value<std::string>()->default_value("./res/textures"))
   ("x,scriptpath", "Script file to use", cxxopts::value<std::vector<std::string>>()->default_value(""))
   ("f,framebuffer", "Folder path of framebuffer", cxxopts::value<std::string>()->default_value("./res/shaders/framebuffer"))
   ("u,uishader", "Shader to use for ui", cxxopts::value<std::string>()->default_value("./res/shaders/ui"))
   ("c,crosshair", "Icon to use for crosshair", cxxopts::value<std::string>()->default_value("./res/textures/crosshairs/crosshair029.png"))
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

  const std::string shaderFolderPath = result["shader"].as<std::string>();
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

     // TODO orthoproj is using current screen width and height.  Switch this to match NDI for simplification. 
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

  fontMeshes = loadFontMeshes(readFont(result["font"].as<std::string>()));
  Mesh crosshairSprite = loadSpriteMesh(result["crosshair"].as<std::string>(), loadTexture);
 
  createStaticSchemeBindings(
    loadScene,
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
    raycast
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
      if (obj.netsynchronize){   
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
      }
    }, 
    [](GameObject &obj) -> void {
      std::cout << "created obj id: " << obj.id << std::endl;
      UdpPacket packet { .type = CREATE };

      packet.payload.createpacket = CreatePacket { .id = obj.id };
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
    [](short id) -> void {
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

  loadAllTextures();

  dynamicLoading = createDynamicLoading(chunkSize);
  if (!useChunkingSystem){
    std::cout << "INFO: # of intitial raw scenes: " << rawScenes.size() << std::endl;
    for (auto rawScene : rawScenes){
      loadScene(rawScene);
    }
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
    onWorldFrame(world, deltaTime, enablePhysics, dumpPhysics); 
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

    if (state.useDefaultCamera || activeCameraObj == NULL){
      view = renderView(defaultCamera.transformation.position, defaultCamera.transformation.rotation);
    }else{
      auto cameraId = activeCameraObj -> id;
      Scene& scene = world.scenes.at(world.idToScene.at(cameraId));
      auto transformation = fullTransformation(scene, cameraId);
      view = renderView(transformation.position, transformation.rotation);   // this is position incorrect because this needs to traverse the object hierachy
    }
    projection = glm::perspective(glm::radians(state.fov), (float)state.currentScreenWidth / state.currentScreenHeight, 0.1f, 1000.0f); 

    glfwSwapBuffers(window);
    
    std::vector<LightInfo> lights = getLightInfo(world);

    updateVoxelPtr();   // this should be removed.  This basically picks a voxel id to be the one we work on. Better would to just have some way to determine this (like with the core selection mechanism)

    setActiveDepthTexture(1);

    // depth buffer form point of view of 1 light source (all eventually, but 1 for now)

    auto lightPosition = lights.size() > 0 ? lights.at(0).pos : glm::vec3(0, 0, 0);
    auto lightRotation = lights.size() > 0 ? lights.at(0).rotation : glm::identity<glm::quat>();
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
    if (selectItemCalled){
      selectItem();
      selectItemCalled = false;
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
  
    // 2ND pass renders what we care about to the screen.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1, 0.1, 0.1, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
    for (auto &[_, scene] : world.scenes){
      renderScene(scene, shaderProgram, projection, view, glm::mat4(1.0f), false, lights);
    }

    if (showDebugInfo){
      displayRails(getRails(world.objectMapping));
      renderVector(shaderProgram, projection, view, glm::mat4(1.0f));
    }

    renderUI(crosshairSprite, currentFramerate);

    handleInput(disableInput, window, deltaTime, state, translate, scale, rotate, moveCamera, nextCamera, setObjectDimensions, onDebugKey, onArrowKey, schemeBindings.onCameraSystemChange, onDelete);
    glfwPollEvents();
    schemeBindings.onFrame();
    schemeBindings.onMessage(channelMessages);  // modifies the queue

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
    stopSoundSystem();
    glfwTerminate(); 
   
  return 0;
}
