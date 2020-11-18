#include "./main_input.h"

extern World world;
extern engineState state;
extern SchemeBindingCallbacks schemeBindings;
extern bool disableInput;
extern GameObjectVoxel* voxelPtr;
extern glm::mat4 voxelPtrModelMatrix;
extern KeyRemapper keyMapper;
extern bool useYAxis;
extern DrawingParams drawParams;
extern glm::mat4 projection;
extern glm::mat4 view;
extern GameObject defaultCamera;
extern std::vector<Line> permaLines;

void processManipulatorForId(objid id){
  if (id == -1 || !idExists(world, id)){
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
    for (auto id : selectedIds(state.editor, state.multiselectMode)){
      processManipulatorForId(id);
    }
  }
}

void onMouseEvents(GLFWwindow* window, double xpos, double ypos){
  onMouse(disableInput, window, state, xpos, ypos, rotateCamera);  
  schemeBindings.onMouseMoveCallback(state.offsetX, state.offsetY); 
  processManipulator();
}

void expandVoxelUp(){
  if (voxelPtr == NULL){
    return;
  }
  std::cout << "voxels: expand voxel up" << std::endl;
  expandVoxels(voxelPtr -> voxel, 0, useYAxis ? -1 : 0, !useYAxis ? -1 : 0);
}
void expandVoxelDown(){
  if (voxelPtr == NULL){
    return;
  }
  std::cout << "voxels: expand voxel down" << std::endl;
  expandVoxels(voxelPtr -> voxel , 0, useYAxis ? 1 : 0, !useYAxis ? 1 : 0);
}
void expandVoxelLeft(){
  if (voxelPtr == NULL){
    return;
  }
  std::cout << "voxels: expand voxel left" << std::endl;
  expandVoxels(voxelPtr -> voxel, -1, 0, 0);
}
void expandVoxelRight(){
  if (voxelPtr == NULL){
    return;
  }
  std::cout << "voxels: expand voxel right" << std::endl;
  expandVoxels(voxelPtr -> voxel, 1, 0, 0);
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

  for (auto id : getIdsInGroup(world, index)){
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
  if (!state.offsetTextureMode && selected(state.editor) != -1 && idExists(world, selected(state.editor))){
    maybeChangeTexture(selected(state.editor));
  }

  if (voxelPtr == NULL){
    return;
  }
  
  auto activeTexture = activeTextureId();
  if (activeTexture != -1){
    if (yoffset > 0){
      applyTextureToCube(voxelPtr -> voxel, voxelPtr -> voxel.selectedVoxels, activeTexture);
    }
    if (yoffset < 0){
      applyTextureToCube(voxelPtr -> voxel, voxelPtr -> voxel.selectedVoxels, activeTexture);
    }
  }
}

void keyCharCallback(GLFWwindow* window, unsigned int codepoint){
  schemeBindings.onKeyCharCallback(codepoint); 
  applyKey(world.objectMapping, codepoint, [](std::string text) -> void {
    std::cout << "set text placeholder: " << text << std::endl;
  });
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
  if (key == 259 && voxelPtr != NULL){  // backspace
    removeVoxel(voxelPtr -> voxel, voxelPtr -> voxel.selectedVoxels);
    voxelPtr -> voxel.selectedVoxels.clear();
  } 

  if (key == GLFW_KEY_LEFT && action == 1 && selected(state.editor) != -1){
    for (auto id : selectedIds(state.editor, state.multiselectMode)){
      handleSnapEasyLeft(id);
    }
  }
  if (key == GLFW_KEY_RIGHT && action == 1 && selected(state.editor) != -1){
    for (auto id : selectedIds(state.editor, state.multiselectMode)){
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
    state.portalTextureIndex--;
    if (state.portalTextureIndex < 0){
      state.portalTextureIndex = 0;
    }
    std::cout << "portal index: " << state.portalTextureIndex << std::endl;
  }
  if (key == GLFW_KEY_L && action == 1){
    state.portalTextureIndex++;
    std::cout << "portal index: " << state.portalTextureIndex << std::endl;
  }

  if (key == GLFW_KEY_Q && action == 1){
    applyHeightmapMasking(world, selected(state.editor), 1.f);
  }
  if (key == GLFW_KEY_E && action == 1){
    applyHeightmapMasking(world, selected(state.editor), -1.f);
  }

  if (key == GLFW_KEY_P && action == 1){
    state.shouldSelect = !state.shouldSelect;
  }

  if (key == 260 && action == 1){   // ins
    state.multiselectMode = !state.multiselectMode;
    std::cout << "INFO: multi select mode: " << state.multiselectMode << std::endl;
  }
  if (key == 261 && action == 1){   // del
  }
  if (key == 268 && action == 1){   // home
    mirrorAllObjects(state.editor);
  }
  if (key == 269 && action == 1){   // end
    rmAllObjects(state.editor, removeObjectById);
  }
}

void onMouseButton(){    
  //for (auto [id, scene] : world.scenes){
  //  std::cout << scenegraphAsDotFormat(scene, world.objectMapping) << std::endl;
  //}
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
}