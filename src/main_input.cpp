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
extern float deltaTime;
extern Benchmark benchmark;

std::string dumpDebugInfo(bool fullInfo){
  auto sceneInfo = std::string("final scenegraph\n") + scenegraphAsDotFormat(world.sandbox, world.objectMapping) + "\n\n";
  auto gameobjInfo = debugAllGameObjects(world.sandbox);
  auto gameobjhInfo = debugAllGameObjectsH(world.sandbox);
  auto cacheInfo = debugTransformCache(world.sandbox);

  auto benchmarkingContent = benchmarkResult(benchmark);
  auto profilingInfo = fullInfo ? dumpProfiling() : "" ;

  auto content = "gameobj info - id id name\n" + gameobjInfo + "\n" + 
    "gameobjh info - id id sceneId groupId parentId | [children]\n" + gameobjhInfo + "\n" + 
    "transform cache - id pos scale" + cacheInfo + "\n" + 
    sceneInfo +  benchmarkingContent + "\n" + profilingInfo;
  return content;
}

void processManipulatorForId(objid id){
  if (id == -1 || !idExists(world.sandbox, id)){
    return;
  }
  //auto transform = fullTransformation(world.sandbox, id);
  auto transform = getGameObject(world, id).transformation; 
  if (state.manipulatorMode == TRANSLATE){
    applyPhysicsTranslation(world, id, transform.position, state.offsetX, state.offsetY, state.manipulatorAxis);
  }else if (state.manipulatorMode == SCALE){
    applyPhysicsScaling(world, id, transform.position, transform.scale, state.lastX, state.lastY, state.offsetX, state.offsetY, state.manipulatorAxis);
  }else if (state.manipulatorMode == ROTATE){
    applyPhysicsRotation(world, id, transform.rotation, state.offsetX, state.offsetY, state.manipulatorAxis);
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
  //std::cout << "Joystick callback:" << std::endl;
  //for (auto info : infos){
  //  std::cout << "( " << info.index << ", " << info.value << ")" << std::endl;
  //}
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
    expandVoxelRight();
  }
  if (key == 263){ // left
    std::cout << "previous texture" << std::endl;
    previousTexture();
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

void onScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
  scroll_callback(window, state, xoffset, yoffset);
  schemeBindings.onScrollCallback(yoffset);
  
  if (selected(state.editor) != -1 && idExists(world.sandbox, selected(state.editor))){
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
  //std::cout << "Key is: " << codepoint << std::endl;
  applyKey(world.objectMapping, codepoint, [](std::string text) -> void {
    std::cout << "set text placeholder: " << text << std::endl;
  }); 
}
void keyCharCallback(GLFWwindow* window, unsigned int codepoint){
  keyCharCallback(codepoint);
}

void handleSnapEasyLeft(objid id){
  if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
    setGameObjectPosition(id, snapTranslateUp(state.snappingMode, getGameObjectPosition(id, false), state.manipulatorAxis));
  }else if (state.manipulatorMode == ROTATE){
    setGameObjectRotation(id, snapAngleDown(state.snappingMode, getGameObjectRotation(id, false), state.manipulatorAxis));
  }else if (state.manipulatorMode == SCALE){
    setGameObjectScale(id, snapScaleDown(state.snappingMode, getGameObjectScale(id), state.manipulatorAxis));
  }
}
void handleSnapEasyRight(objid id){
  if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
    setGameObjectPosition(id, snapTranslateDown(state.snappingMode, getGameObjectPosition(id, false), state.manipulatorAxis));
  }else if (state.manipulatorMode == ROTATE){
    setGameObjectRotation(id, snapAngleUp(state.snappingMode, getGameObjectRotation(id, false), state.manipulatorAxis));
  }else if (state.manipulatorMode == SCALE){
    setGameObjectScale(id, snapScaleUp(state.snappingMode, getGameObjectScale(id), state.manipulatorAxis));
  }
}

void onDelete(){
  for (auto id : selectedIds(state.editor)){
    std::cout << "OnDelete object id: " << id << std::endl;
    removeObjectById(id);
  }
  clearSelectedIndexs(state.editor);   
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
  if (state.printKeyStrokes){
    std::cout << "key: " << key << " action: " << action << std::endl;
  }
  schemeBindings.onKeyCallback(getKeyRemapping(keyMapper, key), scancode, action, mods);

  // below stuff is editor/misc stuff
  if (key == 259){  // backspace
    for (auto voxelData : getSelectedVoxels()){
      removeVoxel(voxelData.voxelPtr -> voxel, voxelData.voxelPtr -> voxel.selectedVoxels);
      voxelData.voxelPtr -> voxel.selectedVoxels.clear();
    }
  } 

  if (key == GLFW_KEY_UP && action == 1 && selected(state.editor) != -1){
    setSnapEasyUseUp(state.manipulatorMode);
  }
  if (key == GLFW_KEY_DOWN && action == 1 && selected(state.editor) != -1){
    setSnapEasyUseDown(state.manipulatorMode);
  }

  /*if (key == GLFW_KEY_K && action == 1){
    state.textureIndex--;
    if (state.textureIndex < 0){
      state.textureIndex = 0;
    }
    std::cout << "texture index: " << state.textureIndex << std::endl;
  }
  if (key == GLFW_KEY_L && action == 1){
    state.textureIndex++;
    std::cout << "texture index: " << state.textureIndex << std::endl;
  }*/

  if (key == GLFW_KEY_Q && action == 1){
    state.shouldTerrainPaint = !state.shouldTerrainPaint;
    std::cout << "toggling terrain paint: " << state.shouldTerrainPaint << std::endl;
  }
  if (key == GLFW_KEY_E && action == 1){
    state.terrainPaintDown = !state.terrainPaintDown;
    std::cout << "toggling terrain paint direction: " << state.terrainPaintDown << std::endl;
  }

  /*if (key == GLFW_KEY_N && action == 1){
    return;
    assert(!state.isRecording);
    auto gameobj = getGameObjectByName(world, "record");
    if (gameobj.has_value()){
      std::cout << "INPUT -> STARTED RECORDING" << std::endl;
      state.isRecording = true;
      state.recordingIndex = createRecording(gameobj.value());
    }
  } */

  if (key == GLFW_KEY_M && action == 1){
    return;
    assert(state.isRecording);
    std::cout << "INPUT -> STOPPED RECORDING" << std::endl;
    saveRecording(state.recordingIndex, "./res/recordings/move.rec");
    state.isRecording = false;
    state.recordingIndex = -1;
  }

  if (key == 260){
    clearSelectedIndexs(state.editor);
  }

  if (key == 340){
    state.moveUp = (action == 1);
  }
}


void onMouseButton(){    
  std::cout << scenegraphAsDotFormat(world.sandbox, world.objectMapping) << std::endl;
  auto rayDirection = getCursorRayDirection(projection, view, state.cursorLeft, state.cursorTop, state.currentScreenWidth, state.currentScreenHeight);
  Line line = {
    .fromPos = defaultCamera.transformation.position,
    .toPos = glm::vec3(rayDirection.x * 1000, rayDirection.y * 1000, rayDirection.z * 1000),
  };
  for (auto id : selectedIds(state.editor)){
    handleVoxelRaycast(world, id, line.fromPos, line.toPos);
  }
  permaLines.clear();
  permaLines.push_back(line);
}

void drop_callback(GLFWwindow* window, int count, const char** paths){
  for (int i = 0;  i < count;  i++){
    std::cout << "Detected dropped file: " << paths[i] << std::endl;
    auto fileType = getFileType(paths[i]);

    std::string objectName = "random";

    auto sceneId = 0;  //  todo -> which scene should it be loaded into?
    assert(false);

    if (fileType == IMAGE_EXTENSION){
      setTexture(selected(state.editor), paths[i]);
    }else if (fileType == AUDIO_EXTENSION){
      makeObjectAttr(sceneId, "&" + objectName, {{ "clip", paths[i] }}, {}, {});
    }else if (fileType == VIDEO_EXTENSION){
      std::cout << "VIDEO FILE: doing nothing" << std::endl;
      makeObjectAttr(sceneId, "=" + objectName, {{ "source", paths[i] }}, {}, {});
    }else if (fileType == MODEL_EXTENSION){
      makeObjectAttr(sceneId, objectName, {{ "mesh", paths[i] }}, {}, {});
    }else if (fileType == UNKNOWN_EXTENSION){
      std::cout << "UNKNOWN file format, so doing nothing: " << paths[i] << std::endl;
    }
  }
}

float cameraSpeed = 1.f;
std::vector<InputDispatch> inputFns = {
  InputDispatch{
    .sourceKey = 71,  // G 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,
    .hasPreq = true,
    .fn = [&state]() -> void {
      state.manipulatorMode = TRANSLATE;
    }
  },
  InputDispatch{
    .sourceKey = 82,  // R
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,
    .hasPreq = true,
    .fn = [&state]() -> void {
      state.manipulatorMode = ROTATE;
    }
  },
  InputDispatch{
    .sourceKey = 83,  // S
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,
    .hasPreq = true,
    .fn = [&state]() -> void {
      state.manipulatorMode = SCALE;
    }
  },
  InputDispatch{
    .sourceKey = 49,  // 1
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      state.renderMode =  RENDER_FINAL;
    }
  }, 
  InputDispatch{
    .sourceKey = 50,  // 2
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      state.renderMode = RENDER_DEPTH;
    }
  }, 
  InputDispatch{
    .sourceKey = 51,  // 3
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      state.renderMode = RENDER_PORTAL;
    }
  }, 
  InputDispatch{
    .sourceKey = 52,  // 4
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      state.renderMode = RENDER_PAINT;
    }
  }, 
  InputDispatch{
    .sourceKey = 67,  // 4
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,
    .hasPreq = true,
    .fn = [&state]() -> void {
      setClipboardFromSelected(state.editor);
    }
  }, 
  InputDispatch{
    .sourceKey = 86,  // 4
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,
    .hasPreq = true,
    .fn = [&state]() -> void {
      copyAllObjects(state.editor, copyObject);
    }
  }, 
  InputDispatch{
    .sourceKey = 340,  // 4
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = [&state]() -> void {
      state.multiselect = true;
    }
  },
  InputDispatch{
    .sourceKey = 340,  // 4
    .sourceType = BUTTON_RELEASE,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = [&state]() -> void {
      state.multiselect = false;
    }
  },  
  InputDispatch{
    .sourceKey = 87,  // w
    .sourceType = BUTTON_HOLD,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = [&state]() -> void {
      auto speed = cameraSpeed * -40.0f * deltaTime;
      glm::vec3 moveVec = state.moveUp ? glm::vec3(0.0, -1 * speed, 0.f) : glm::vec3(0.0, 0.0, speed);
      moveCamera(moveVec);
    }
  },  
  InputDispatch{
    .sourceKey = 65,  // a
    .sourceType = BUTTON_HOLD,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = [&state]() -> void {
      moveCamera(glm::vec3(cameraSpeed * -40.0 * deltaTime, 0.0, 0.0));
    }
  },  
  InputDispatch{
    .sourceKey = 83,  // s
    .sourceType = BUTTON_HOLD,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = [&state]() -> void {
      auto speed = cameraSpeed * 40.0f * deltaTime;
      glm::vec3 moveVec = state.moveUp ? glm::vec3(0.0, -1 * speed, 0.f) : glm::vec3(0.0, 0.0, speed);
      moveCamera(moveVec);
    }
  },  
  InputDispatch{
    .sourceKey = 68,  // d
    .sourceType = BUTTON_HOLD,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = [&state]() -> void {
      moveCamera(glm::vec3(cameraSpeed * 40.0f * deltaTime, 0.0, 0.0f));
    }
  },  
  InputDispatch{
    .sourceKey = 83,  // s
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,
    .hasPreq = true,
    .fn = [&state]() -> void {
      saveHeightmap(world, selected(state.editor));
    }
  }, 
  InputDispatch{
    .sourceKey = 65,  // a
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      std::cout << "setting snap absolute" << std::endl;
      state.snappingMode = SNAP_ABSOLUTE;
    }
  },   
  InputDispatch{
    .sourceKey = 67,  // c
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      std::cout << "setting snap continuous" << std::endl;
      state.snappingMode = SNAP_CONTINUOUS;
    }
  },   
  InputDispatch{
    .sourceKey = 82,  // r
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      std::cout << "setting snap relative" << std::endl;
      state.snappingMode = SNAP_RELATIVE;
    }
  }, 
  InputDispatch{
    .sourceKey = 263,  // left arrow
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = [&state]() -> void {
      for (auto id : selectedIds(state.editor)){
        handleSnapEasyLeft(id);
      }
    }
  }, 
  InputDispatch{
    .sourceKey = 262,  // right arrow
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = [&state]() -> void {
      for (auto id : selectedIds(state.editor)){
        handleSnapEasyRight(id);
      }
    }
  }, 
  InputDispatch{
    .sourceKey = 341,  // ctrl
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = [&cameraSpeed, &state]() -> void {
      state.cameraFast = !state.cameraFast;
      std::cout << "camera fast: " << state.cameraFast << std::endl;
      cameraSpeed = state.cameraFast ? 1.f : 0.1f;
    }
  },
  InputDispatch{
    .sourceKey = 261,  // delete
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      onDelete();
    }
  },
  InputDispatch{
    .sourceKey = 79,  // O
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.showCameras = !state.showCameras;
      std::cout << "show cameras: " << state.showCameras << std::endl;
    }
  },
  InputDispatch{
    .sourceKey = 79,  // O
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.drawPoints = !state.drawPoints;
    }
  },
  InputDispatch{
    .sourceKey = 59,  // ;
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      enforceAllLayouts(world);
    }
  },

  InputDispatch{
    .sourceKey = 322,  // ;
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      snapCameraDown(setCameraRotation);
    }
  },
  InputDispatch{
    .sourceKey = 324,  // ;
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      snapCameraLeft(setCameraRotation);
    }
  },
  InputDispatch{
    .sourceKey = 326,  // ;
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      snapCameraRight(setCameraRotation);
    }
  },
  InputDispatch{
    .sourceKey = 327,  // ;
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      snapCameraForward(setCameraRotation);
    }
  },
  InputDispatch{
    .sourceKey = 328,  // ;
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      snapCameraUp(setCameraRotation);
    }
  },
  InputDispatch{
    .sourceKey = 329,  // ;
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      snapCameraBackward(setCameraRotation);
    }
  },
  InputDispatch{
    .sourceKey = 79, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      std::cout << dumpDebugInfo(false) << std::endl;
    }
  },
  InputDispatch{
    .sourceKey = 348, // to the right of fn key, looks like notepad
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.printKeyStrokes = !state.printKeyStrokes;
    }
  },
  InputDispatch{
    .sourceKey = 92, // "\"
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.cullEnabled = !state.cullEnabled;
      if (state.cullEnabled){
        glEnable(GL_CULL_FACE);  
      }else{
        glDisable(GL_CULL_FACE);  
      }
    }
  },
  InputDispatch{
    .sourceKey = 85, // u
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      auto selected = selectedIds(state.editor);
      setObjectDimensions(world, selected, 10, 5, 10);
    }
  },
};






 