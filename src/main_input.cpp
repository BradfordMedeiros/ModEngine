#include "./main_input.h"

extern World world;
extern engineState state;
extern SchemeBindingCallbacks schemeBindings;
extern bool disableInput;
extern KeyRemapper keyMapper;
extern bool useYAxis;
extern DrawingParams drawParams;
extern glm::mat4 projection;
extern glm::mat4 view;
extern GameObject defaultCamera;
extern std::vector<Line> permaLines;

void processManipulatorForId(objid id){
  if (id == -1 || !idExists(world.sandbox, id)){
    return;
  }
  auto selectObject = getGameObject(world, id); 
  if (state.manipulatorMode == TRANSLATE){
    applyPhysicsTranslation(world, id, selectObject.transformation.position, state.offsetX, state.offsetY, state.manipulatorAxis);
  }else if (state.manipulatorMode == SCALE){
    applyPhysicsScaling(world, id, selectObject.transformation.position, selectObject.transformation.scale, state.lastX, state.lastY, state.offsetX, state.offsetY, state.manipulatorAxis);
  }else if (state.manipulatorMode == ROTATE){
    applyPhysicsRotation(world, id, selectObject.transformation.rotation, state.offsetX, state.offsetY, state.manipulatorAxis);
  } 
}

void processManipulator(){
  if (state.enableManipulator){
    for (auto id : selectedIds(state.editor)){
      processManipulatorForId(id);
    }
  }
}

void onMouseEvents(GLFWwindow* window, double xpos, double ypos){
  onMouse(disableInput, window, state, xpos, ypos, rotateCamera);  
  schemeBindings.onMouseMoveCallback(state.offsetX, state.offsetY); 
  processManipulator();
}

void joystickCallback(int jid, int event){
  if (event == GLFW_CONNECTED){
    std::cout << "gamepad connected" << std::endl;
  }else if (event == GLFW_DISCONNECTED){
    std::cout << "gamepad disconnected" << std::endl;
    // this disconnection thing seems to suck.  Can turn controller off and no message here
  }
}

void onJoystick(std::vector<JoyStickInfo> infos){
  std::cout << "Joystick callback:" << std::endl;
  for (auto info : infos){
    std::cout << "( " << info.index << ", " << info.value << ")" << std::endl;
  }
}


void expandVoxelUp(){
  for (auto voxelData : getSelectedVoxels()){
    std::cout << "voxels: expand voxel up" << std::endl;
    expandVoxels(voxelData.voxelPtr -> voxel, 0, useYAxis ? -1 : 0, !useYAxis ? -1 : 0);
  }
}
void expandVoxelDown(){
  for (auto voxelData : getSelectedVoxels()){
    std::cout << "voxels: expand voxel down" << std::endl;
    expandVoxels(voxelData.voxelPtr -> voxel , 0, useYAxis ? 1 : 0, !useYAxis ? 1 : 0);
  }  
}
void expandVoxelLeft(){
  for (auto voxelData : getSelectedVoxels()){
    std::cout << "voxels: expand voxel left" << std::endl;
    expandVoxels(voxelData.voxelPtr -> voxel, -1, 0, 0); 
  }  
}
void expandVoxelRight(){
  for (auto voxelData : getSelectedVoxels()){
    std::cout << "voxels: expand voxel right" << std::endl;
    expandVoxels(voxelData.voxelPtr -> voxel, 1, 0, 0);
  }  
}

void onArrowKey(int key){
  if (key == 262){ // right
    std::cout << "next texture" << std::endl;
    nextTexture();
    state.bloomAmount += 0.1f;
    expandVoxelRight();
  }
  if (key == 263){ // left
    std::cout << "previous texture" << std::endl;
    previousTexture();
    state.bloomAmount -= 0.1f;
    if (state.bloomAmount < 0.f){
      state.bloomAmount = 0.f;
    }
    expandVoxelLeft();
  }

  if (key == 265){ // up
    expandVoxelUp();
  }
  if (key == 264){ // down
    expandVoxelDown();
  }

  std::cout << "key: " << key << std::endl;
}

void maybeApplyTextureOffset(int index, glm::vec2 offset){
  GameObjectMesh* meshObj = std::get_if<GameObjectMesh>(&world.objectMapping.at(index));
  if (meshObj == NULL){
    return;
  }

  for (auto id : getIdsInGroup(world.sandbox, index)){
    GameObjectMesh* meshObj = std::get_if<GameObjectMesh>(&world.objectMapping.at(id));
    assert(meshObj != NULL);
    meshObj -> texture.textureoffset = meshObj -> texture.textureoffset + offset;
  }
}


void onScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
  scroll_callback(window, state, xoffset, yoffset);
  schemeBindings.onScrollCallback(yoffset);
  
  if (state.offsetTextureMode && selected(state.editor) != -1){
    float offsetAmount = yoffset * 0.001;
    maybeApplyTextureOffset(selected(state.editor), glm::vec2(state.manipulatorAxis == YAXIS ? offsetAmount : 0, state.manipulatorAxis == YAXIS ? 0 : offsetAmount));
  }
  if (!state.offsetTextureMode && selected(state.editor) != -1 && idExists(world.sandbox, selected(state.editor))){
    maybeChangeTexture(selected(state.editor));
  }

  for (auto voxelData : getSelectedVoxels()){ 
    auto activeTexture = activeTextureId();
    if (activeTexture != -1){
      if (yoffset > 0){
        applyTextureToCube(voxelData.voxelPtr -> voxel, voxelData.voxelPtr -> voxel.selectedVoxels, activeTexture);
      }
      if (yoffset < 0){
        applyTextureToCube(voxelData.voxelPtr -> voxel, voxelData.voxelPtr -> voxel.selectedVoxels, activeTexture);
      }
    }
  } 

}

void keyCharCallback(unsigned int codepoint){
  schemeBindings.onKeyCharCallback(codepoint); 
  applyKey(world.objectMapping, codepoint, [](std::string text) -> void {
    std::cout << "set text placeholder: " << text << std::endl;
  }); 
}
void keyCharCallback(GLFWwindow* window, unsigned int codepoint){
  keyCharCallback(codepoint);
}

void handleSnapEasyLeft(objid id){
  if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
    setGameObjectPosition(id, snapTranslateUp(getGameObjectPosition(id, false), state.manipulatorAxis));
  }else if (state.manipulatorMode == ROTATE){
    setGameObjectRotation(id, snapAngleDown(getGameObjectRotation(id, false), state.manipulatorAxis));
  }else if (state.manipulatorMode == SCALE){
    setGameObjectScale(id, snapScaleDown(getGameObjectScale(id), state.manipulatorAxis));
  }
}
void handleSnapEasyRight(objid id){
  if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
    setGameObjectPosition(id, snapTranslateDown(getGameObjectPosition(id, false), state.manipulatorAxis));
  }else if (state.manipulatorMode == ROTATE){
    setGameObjectRotation(id, snapAngleUp(getGameObjectRotation(id, false), state.manipulatorAxis));
  }else if (state.manipulatorMode == SCALE){
    setGameObjectScale(id, snapScaleUp(getGameObjectScale(id), state.manipulatorAxis));
  }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
  schemeBindings.onKeyCallback(getKeyRemapping(keyMapper, key), scancode, action, mods);

  // below stuff is editor/misc stuff
  if (key == 259){  // backspace
    for (auto voxelData : getSelectedVoxels()){
      removeVoxel(voxelData.voxelPtr -> voxel, voxelData.voxelPtr -> voxel.selectedVoxels);
      voxelData.voxelPtr -> voxel.selectedVoxels.clear();
    }
  } 


  if (key == GLFW_KEY_LEFT && action == 1 && selected(state.editor) != -1){
    for (auto id : selectedIds(state.editor)){
      handleSnapEasyLeft(id);
    }
  }
  if (key == GLFW_KEY_RIGHT && action == 1 && selected(state.editor) != -1){
    for (auto id : selectedIds(state.editor)){
      handleSnapEasyRight(id);
    }
  }
  if (key == GLFW_KEY_UP && action == 1 && selected(state.editor) != -1){
    if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
      setSnapTranslateUp();
    }else if (state.manipulatorMode == ROTATE){
      setSnapAngleUp();
    }else if (state.manipulatorMode == SCALE){
      setSnapScaleUp();
    }
  }
  if (key == GLFW_KEY_DOWN && action == 1 && selected(state.editor) != -1){
    if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
      setSnapTranslateDown();
    }else if (state.manipulatorMode == ROTATE){
      setSnapAngleDown();
    }else if (state.manipulatorMode == SCALE){
      setSnapScaleDown();
    }
  }

  if (key == GLFW_KEY_K && action == 1){
    state.textureIndex--;
    if (state.textureIndex < 0){
      state.textureIndex = 0;
    }
    std::cout << "texture index: " << state.textureIndex << std::endl;
  }
  if (key == GLFW_KEY_L && action == 1){
    state.textureIndex++;
    std::cout << "texture index: " << state.textureIndex << std::endl;
  }

  if (key == GLFW_KEY_Q && action == 1){
    state.shouldTerrainPaint = !state.shouldTerrainPaint;
    std::cout << "toggling terrain paint: " << state.shouldTerrainPaint << std::endl;
  }
  if (key == GLFW_KEY_E && action == 1){
    state.terrainPaintDown = !state.terrainPaintDown;
    std::cout << "toggling terrain paint direction: " << state.terrainPaintDown << std::endl;
  }

  if (key == GLFW_KEY_U && action == 1){
    saveHeightmap(world, selected(state.editor));
  }

  if (key == GLFW_KEY_P && action == 1){
    state.shouldSelect = !state.shouldSelect;
  }

  if (key == GLFW_KEY_T && action == 1){
    for (auto id : selectedIds(state.editor)){
      // can have multiple objects w/ same group id, just waste
      auto groupId = getGroupId(world.sandbox, id);
      auto meshNameToMeshes = getMeshesForGroupId(world, groupId);  
      updateBonePoses(meshNameToMeshes, getModelTransform);
    }
  }

  if (key == GLFW_KEY_N && action == 1){
    return;
    assert(!state.isRecording);
    auto gameobj = getGameObjectByName(world, "record");
    if (gameobj.has_value()){
      std::cout << "INPUT -> STARTED RECORDING" << std::endl;
      state.isRecording = true;
      state.recordingIndex = createRecording(gameobj.value());
    }

  } 
  if (key == GLFW_KEY_M && action == 1){
    return;
    assert(state.isRecording);
    std::cout << "INPUT -> STOPPED RECORDING" << std::endl;
    saveRecording(state.recordingIndex, "./res/recordings/move.rec");
    state.isRecording = false;
    state.recordingIndex = -1;
  }

  auto controlPressed = ((glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS   ) || (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS));
  if (controlPressed && key == 67 && action == 1){   // C
    setClipboardFromSelected(state.editor);
  }
  if (controlPressed && key == 86 && action == 1){  // V
    copyAllObjects(state.editor, copyObject);
  }
  if (key == 269 && action == 1){   // end
    rmAllObjects(state.editor, removeObjectById);
  }
  if (key == 340){
    if (action == 0){
      state.multiselect = false;
    }
    if (action == 1){
      state.multiselect = true;
    }
  }

  if (key == 260){
    clearSelectedIndexs(state.editor);
  }

  std::cout << "key is: " << key << std::endl;
}

void onMouseButton(){    
  for (auto id : allSceneIds(world.sandbox)){
    std::cout << scenegraphAsDotFormat(world.sandbox, id, world.objectMapping) << std::endl;
  }
  auto rayDirection = getCursorRayDirection(projection, view, state.cursorLeft, state.cursorTop, state.currentScreenWidth, state.currentScreenHeight);
  Line line = {
    .fromPos = defaultCamera.transformation.position,
    .toPos = glm::vec3(rayDirection.x * 1000, rayDirection.y * 1000, rayDirection.z * 1000),
  };
 
  permaLines.clear();
  permaLines.push_back(line);

  for (auto voxelData : getSelectedVoxels()){
    auto voxelPtrModelMatrix = fullModelTransform(world.sandbox, voxelData.index);
    glm::vec4 fromPosModelSpace = glm::inverse(voxelPtrModelMatrix) * glm::vec4(line.fromPos.x, line.fromPos.y, line.fromPos.z, 1.f);
    glm::vec4 toPos =  glm::vec4(line.fromPos.x, line.fromPos.y, line.fromPos.z, 1.f) + glm::vec4(rayDirection.x * 1000, rayDirection.y * 1000, rayDirection.z * 1000, 1.f);
    glm::vec4 toPosModelSpace = glm::inverse(voxelPtrModelMatrix) * toPos;
    glm::vec3 rayDirectionModelSpace =  toPosModelSpace - fromPosModelSpace;
    // This raycast happens in model space of voxel, so specify position + ray in voxel model space
    auto collidedVoxels = raycastVoxels(voxelData.voxelPtr -> voxel, fromPosModelSpace, rayDirectionModelSpace);
    std::cout << "length is: " << collidedVoxels.size() << std::endl;
    if (collidedVoxels.size() > 0){
      auto collision = collidedVoxels[0];
      voxelData.voxelPtr -> voxel.selectedVoxels.push_back(collision);
      applyTextureToCube(voxelData.voxelPtr -> voxel, voxelData.voxelPtr -> voxel.selectedVoxels, 2);
    }
  }  
}

void drop_callback(GLFWwindow* window, int count, const char** paths){
  for (int i = 0;  i < count;  i++){
    std::cout << "Detected dropped file: " << paths[i] << std::endl;
    auto fileType = getFileType(paths[i]);

    std::string objectName = "random";
    if (fileType == IMAGE_EXTENSION){
      setTexture(selected(state.editor), paths[i]);
    }else if (fileType == AUDIO_EXTENSION){
      makeObjectAttr("&" + objectName, {{ "clip", paths[i] }}, {}, {});
    }else if (fileType == VIDEO_EXTENSION){
      std::cout << "VIDEO FILE: doing nothing" << std::endl;
      makeObjectAttr("=" + objectName, {{ "source", paths[i] }}, {}, {});
    }else if (fileType == MODEL_EXTENSION){
      makeObjectAttr(objectName, {{ "mesh", paths[i] }}, {}, {});
    }else if (fileType == UNKNOWN_EXTENSION){
      std::cout << "UNKNOWN file format, so doing nothing: " << paths[i] << std::endl;
    }
  }
}

