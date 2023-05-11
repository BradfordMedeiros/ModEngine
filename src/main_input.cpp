#include "./main_input.h"

extern World world;
extern RenderStages renderStages;
extern engineState state;
extern CScriptBindingCallbacks cBindings;
extern bool disableInput;
extern KeyRemapper keyMapper;
extern DrawingParams drawParams;
extern glm::mat4 view;
extern DefaultResources defaultResources;
extern float deltaTime;
extern Benchmark benchmark;
extern bool selectItemCalled;
extern DynamicLoading dynamicLoading;
extern SysInterface interface;
extern GLFWwindow* window;
extern LineData lineData;
extern ManipulatorTools tools;
extern TimePlayback timePlayback;

std::string dumpDebugInfo(bool fullInfo){
  auto sceneInfo = std::string("final scenegraph\n") + scenegraphAsDotFormat(world.sandbox, world.objectMapping) + "\n\n";
  auto gameobjInfo = debugAllGameObjects(world.sandbox);
  auto gameobjhInfo = debugAllGameObjectsH(world.sandbox);
  auto cacheInfo = debugTransformCache(world.sandbox);
  auto textureInfo = debugLoadedTextures(world.textures);
  auto meshInfo = debugLoadedMeshes(world.meshes);
  auto animationInfo = debugAnimations(world.animations);

  auto benchmarkingContent = benchmarkResult(benchmark);
  auto profilingInfo = fullInfo ? dumpProfiling() : "" ;

  auto content = "gameobj info - id id name\n" + gameobjInfo + "\n" + 
    "gameobjh info - id id sceneId groupId parentId | [children]\n" + gameobjhInfo + "\n" + 
    "transform cache - id pos scale\n" + cacheInfo + "\n" + 
    "texture cache\n" + textureInfo + "\n" +
    "mesh cache\n" + meshInfo + "\n" + 
    "animation cache\n" + animationInfo + "\n" + 
    sceneInfo +  benchmarkingContent + "\n" + profilingInfo;
  return content;
}

void debugInfo(std::string infoType, std::string filepath){
  saveFile(filepath, dumpDebugInfo(false));
}

void processManipulatorForId(objid id){
  if (id == -1 || !idExists(world.sandbox, id)){
    return;
  }

  if (state.manipulatorMode == TRANSLATE){
    applyPhysicsTranslation(world, id, state.offsetX, state.offsetY, state.manipulatorAxis);
  }else if (state.manipulatorMode == SCALE){
    applyPhysicsScaling(world, id, state.lastX, state.lastY, state.offsetX, state.offsetY, state.manipulatorAxis);
  }else if (state.manipulatorMode == ROTATE){
    applyPhysicsRotation(world, id, state.offsetX, state.offsetY, state.manipulatorAxis);
  } 
}

void processManipulator(){
  if (state.enableManipulator){
    for (auto id : selectedIds(state.editor)){
      processManipulatorForId(id);
    }
  }
}

void onMouse(bool disableInput,  engineState& state, double xpos, double ypos, void(*rotateCamera)(float, float)){
    if(state.firstMouse){
        state.lastX = xpos;
        state.lastY = ypos;
        state.firstMouse = false;
        return;
    }
  
    float xoffset = xpos - state.lastX;
    float yoffset = state.lastY - ypos; 
    state.lastX = xpos;
    state.lastY = ypos;
    state.offsetX = xoffset;
    state.offsetY = yoffset;

    auto coords = ndiCoord();
    cBindings.onMouseMoveCallback(state.offsetX, state.offsetY, coords.x, coords.y);
    
    if (disableInput){
      return;
    }

    float rotateSensitivity = 0.05;
    if (state.isRotateSelection){
      rotateCamera(xoffset * rotateSensitivity, -yoffset * rotateSensitivity);   // -y offset because mouse move forward is negative, which is ok, but inverted
    }else{
      state.cursorLeft = xpos;
      state.cursorTop = ypos;
    }

}


glm::vec2 ndiCoord(){
  float xNdc = 2 * (state.cursorLeft / (float)state.resolution.x) - 1;
  float yNdc = -1 * (2 * (state.cursorTop  / (float)state.resolution.y) - 1);
  return glm::vec2(xNdc, yNdc);
}

void onMouseEvents(GLFWwindow* window, double xpos, double ypos){
  //std::cout << "mouse events: " << xpos << ", " << ypos << std::endl;
  onMouse(state.disableInput,  state, xpos, ypos, rotateCamera); 
}

void onMouse(int button, int action, int mods){
  if (button == 0 && action == 1){
    state.mouseIsDown = true;
  }else if (button == 0 && action == 0){
    state.mouseIsDown = false;
  }

  mouse_button_callback(state.disableInput, state, button, action, mods, onMouseButton);
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
    selectItemCalled = true;
    onManipulatorMouseDown(state.manipulatorState);
  }
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
    onManipulatorMouseRelease(state.manipulatorState);
    setNoActiveObj(state.editor);
  }

  modlog("input", "mouse callback: " + std::to_string(button) + ", action: " + std::to_string(action) + ", mods: " + std::to_string(mods));
  cBindings.onMouseCallback(button, action, mods);

  if (button == 0){
    for (auto voxelData : getSelectedVoxels()){
      voxelData.voxelPtr -> voxel.selectedVoxels.clear();
    }
  }
}
void onMouseCallback(GLFWwindow* window, int button, int action, int mods){
  onMouse(button, action, mods);
}

void dispatchClick(int button, int action){
  std::cout << "dispatch click placeholder" << std::endl;
  onMouse(button, action,  0/*mods*/); // since this forces a click, could get into weird edge cases in behavior
}


void onSelectNullItem(){
  clearSelectedIndexs(state.editor); 
}

void moveCursor(float x, float y){
  state.cursorLeft = x;
  state.cursorTop = y;
  state.lastY = state.cursorTop;
  state.lastX = state.cursorLeft;
  glfwSetCursorPos(window, x, y);
}
void moveMouse(glm::vec2 ndi){
  //std::cout << "Move mouse to ndi: " << print(ndi) << std::endl;
  auto pixelCoords = ndiToPixelCoord(glm::vec2(ndi.x, -1 * ndi.y), state.resolution);
  moveCursor(pixelCoords.x, pixelCoords.y);
}

void mouse_button_callback(bool disableInput, engineState& state, int button, int action, int mods, void (*handleSerialization) (void)){
  if (disableInput){
    return;
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
    state.enableManipulator = true;
    handleSerialization();
    onSelectNullItem();
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
    state.enableManipulator = false;
  }

  if (button == GLFW_MOUSE_BUTTON_MIDDLE){
    if (action == GLFW_PRESS){
      state.isRotateSelection = true;
    }else if (action == GLFW_RELEASE){
      state.isRotateSelection = false;
    }

    moveCursor(state.currentScreenWidth / 2, state.currentScreenHeight / 2);
  }
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
    expandVoxels(voxelData.voxelPtr -> voxel, 0, state.useYAxis ? -1 : 0, !state.useYAxis ? -1 : 0);
  }
}
void expandVoxelDown(){
  for (auto voxelData : getSelectedVoxels()){
    std::cout << "voxels: expand voxel down" << std::endl;
    expandVoxels(voxelData.voxelPtr -> voxel , 0, state.useYAxis ? 1 : 0, !state.useYAxis ? 1 : 0);
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

void scroll_callback(GLFWwindow *window, engineState& state, double xoffset, double yoffset){
}

void onScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
  scroll_callback(window, state, xoffset, yoffset);
  cBindings.onScrollCallback(yoffset);

  for (auto &selectedIndex : selectedIds(state.editor)){
    if (idExists(world.sandbox, selectedIndex) && getLayerForId(selectedIndex).selectIndex != -2){
      maybeChangeTexture(selectedIndex);
    }
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
  cBindings.onKeyCharCallback(codepoint);
  //std::cout << "Key is: " << codepoint << std::endl;
  applyKey(world.objectMapping, codepoint, [](std::string text) -> void {
    std::cout << "set text placeholder: " << text << std::endl;
  }); 
}
void keyCharCallback(GLFWwindow* window, unsigned int codepoint){
  keyCharCallback(codepoint);
}

void handleSnapEasy(objid id, bool left){
  if (getLayerForId(id).selectIndex == -2){
    return;
  }
  if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
    auto objPos = getGameObjectPosition(id, true);
    auto snapAmount = left ? snapTranslateDown(state.easyUse, state.snappingMode, objPos, state.manipulatorAxis) : snapTranslateUp(state.easyUse, state.snappingMode, objPos, state.manipulatorAxis);
    setGameObjectPosition(id, snapAmount, true);
  }else if (state.manipulatorMode == ROTATE){
    auto objRot = getGameObjectRotation(id, true);
    auto snapAmount = left ? snapAngleDown(state.easyUse, state.snappingMode, objRot, state.manipulatorAxis) : snapAngleUp(state.easyUse, state.snappingMode, objRot, state.manipulatorAxis);
    setGameObjectRotation(id, snapAmount, false);
  }else if (state.manipulatorMode == SCALE){
    auto objScale = getGameObjectScale(id);
    auto snapAmount = left ? snapScaleDown(state.easyUse, state.snappingMode, objScale, state.manipulatorAxis) : snapScaleUp(state.easyUse, state.snappingMode, objScale, state.manipulatorAxis);
    setGameObjectScale(id, snapAmount);  // THis might be wrong
    std::cout << "WARNING: SNAP TRANSLATE SET INPUT HANDLING MIGHT BE WRONG (world vs local)" << std::endl;
  }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
  if (state.printKeyStrokes){
    std::cout << "key: " << key << " action: " << action << std::endl;
  }
  cBindings.onKeyCallback(getKeyRemapping(keyMapper, key), scancode, action, mods);

  // below stuff is editor/misc stuff
  if (key == 259){  // backspace
    for (auto voxelData : getSelectedVoxels()){
      removeVoxel(voxelData.voxelPtr -> voxel, voxelData.voxelPtr -> voxel.selectedVoxels);
      voxelData.voxelPtr -> voxel.selectedVoxels.clear();
    }
  } 

  auto selectedIndex = latestSelected(state.editor);
  if (key == GLFW_KEY_UP && action == 1 && selectedIndex.has_value()){
    setSnapEasyUseUp(state.easyUse, state.manipulatorMode);
  }
  if (key == GLFW_KEY_DOWN && action == 1 && selectedIndex.has_value()){
    setSnapEasyUseDown(state.easyUse, state.manipulatorMode);
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
  
  auto id = state.currentHoverIndex;
  if (!idExists(world.sandbox, id) || !isVoxel(world, id)){
    return;
  }
  auto layer = layerByName(getGameObject(world, id).layer);
  auto proj = projectionFromLayer(layer);
  auto rayDirection = getCursorRayDirection(proj, view, state.cursorLeft, state.currentScreenHeight - state.cursorTop, state.currentScreenWidth, state.currentScreenHeight);
  Line line = {
    .fromPos = defaultResources.defaultCamera.transformation.position,
    .toPos = glm::vec3(rayDirection.x * 1000, rayDirection.y * 1000, rayDirection.z * 1000),
  };
  handleVoxelRaycast(world, id, line.fromPos, line.toPos, drawParams.activeTextureIndex);
}

void drop_callback(GLFWwindow* window, int count, const char** paths){
  for (int i = 0;  i < count;  i++){
    std::cout << "Detected dropped file: " << paths[i] << std::endl;
    auto fileType = getFileType(paths[i]);

    std::string objectName = "random";

    auto sceneId = 0;  
    MODTODO("dropping element into scene assumes sceneId " + sceneId);

    if (fileType == IMAGE_EXTENSION){
      auto selectedIndex = latestSelected(state.editor);
      if (selectedIndex.has_value()){
        setTexture(selectedIndex.value(), paths[i]);
      }
    }else if (fileType == AUDIO_EXTENSION){
      GameobjAttributes attr {
        .stringAttributes = {{ "clip", paths[i] }}, 
        .numAttributes = {}, 
        .vecAttr = vectorAttributes { .vec3 = {}, .vec4 = {}},
      };
      std::map<std::string, GameobjAttributes> submodelAttributes;
      makeObjectAttr(sceneId, "&" + objectName, attr, submodelAttributes);
    }else if (fileType == MODEL_EXTENSION){
      GameobjAttributes attr {
        .stringAttributes = {{ "mesh", paths[i] }}, 
        .numAttributes = {}, 
        .vecAttr = vectorAttributes { .vec3 = {}, .vec4 = {}},
      };
      std::map<std::string, GameobjAttributes> submodelAttributes;
      makeObjectAttr(sceneId, objectName, attr, submodelAttributes);
    }else if (fileType == UNKNOWN_EXTENSION){
      std::cout << "UNKNOWN file format, so doing nothing: " << paths[i] << std::endl;
    }
  }
}

void handleInput(GLFWwindow* window){
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
    glfwSetWindowShouldClose(window, true);
  }
  processControllerInput(keyMapper, moveCamera, deltaTime, keyCharCallback, onJoystick);

  if (!state.disableInput){    // we return after escape, so escape still quits
    bool lockSuccess = lock("input", 0);
    if (lockSuccess){
      processKeyBindings(window, keyMapper);
      unlock("input", 0);
    }
  }
  processManipulator();
}

void printControllerDebug(const unsigned char* buttons, int buttonCount){
  std::cout << "== buttons == ( ";
  for (int i = 0; i < buttonCount; i++){
    if (buttons[i] == GLFW_PRESS){
      std::cout << "DOWN ";
    }else {
      std::cout << "UP ";
    }
  }
  std::cout << " )" << std::endl;
}
void printAxisDebug(const float* axises, int count){
  std::cout << "== axises == ( ";
  for (int i = 0; i < count; i++){
    std::cout << " " << axises[i];
  }
  std::cout << " )" << std::endl;
}

float calcAxisValue(const float* axises, int count, int index, float deadzonemin, float deadzonemax, bool invert){
  if (index >= count){
    std::cout << "index: " << index << ", count: " << count << std::endl;
    assert(false);
  }
  float axisValue = axises[index];
  if (axisValue > deadzonemin && axisValue < deadzonemax){
    return 0.f;
  }
  if (invert){
    axisValue = axisValue * -1.f;
  }
  return axisValue;
}

void processControllerInput(KeyRemapper& remapper, void (*moveCamera)(glm::vec3), float deltaTime,  void (*onKeyChar)(unsigned int codepoint), void (*onJoystick)(std::vector<JoyStickInfo> infos)){
  if (!glfwJoystickPresent(GLFW_JOYSTICK_1)){
    //std::cout << "joystick 0 not present" << std::endl;
    return;
  }
  std::cout << "joystick is present" << std::endl;
  int count;
  auto axises = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);  
  int buttonCount;
  auto buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);

  for (auto [index, axisConfiguration] : remapper.axisConfigurations){
    if (axisConfiguration.hasKeyMapping && axisConfiguration.mapping.sourceKey < buttonCount){
      if (buttons[axisConfiguration.mapping.sourceKey] == GLFW_PRESS){
        onKeyChar(axisConfiguration.mapping.destinationKey);
      }
    }
  }

  std::vector<JoyStickInfo> joystickInfos;
  for (auto [index, axisConfig] :  remapper.axisConfigurations){
    auto axisValue = calcAxisValue(axises, count, index, axisConfig.deadzonemin, axisConfig.deadzonemax, axisConfig.invert); 
    if (axisConfig.shouldMapKey && axisValue > axisConfig.amount){
      onKeyChar(axisConfig.destinationKey);
    }
    joystickInfos.push_back(JoyStickInfo{
      .index = index,
      .value = axisValue,
    });
  }
  onJoystick(joystickInfos);
  //printControllerDebug(buttons, buttonCount);
  //printAxisDebug(axises, count);
}

void processKeyBindings(GLFWwindow *window, KeyRemapper& remapper){
  std::map<int, bool> lastFrameDown = {};
  for (auto inputFn : remapper.inputFns){
    auto mainKeyPressed = glfwGetKey(window, inputFn.sourceKey);
    lastFrameDown[inputFn.sourceKey] = mainKeyPressed;
    auto prereqOk = true; 
    if (inputFn.hasPreq){
      auto prereqDown = glfwGetKey(window, inputFn.prereqKey);
      lastFrameDown[inputFn.prereqKey] = prereqDown;
      prereqOk = prereqDown;
    }
    if (prereqOk){
      if (inputFn.sourceType == BUTTON_PRESS){
        if (mainKeyPressed && !remapper.lastFrameDown[inputFn.sourceKey]){
          inputFn.fn();
        }
      }else if (inputFn.sourceType == BUTTON_RELEASE){
        if (!mainKeyPressed && remapper.lastFrameDown[inputFn.sourceKey]){
          inputFn.fn();
        }
      }else if (inputFn.sourceType == BUTTON_HOLD){
        if (mainKeyPressed){
          inputFn.fn();
        }
      }
    }
  } 
  remapper.lastFrameDown = lastFrameDown;
}

void toggleCursor(CURSOR_TYPE cursorBehavior){
  if (cursorBehavior == CURSOR_NORMAL){
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }else if (cursorBehavior == CURSOR_CAPTURE){
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }else if (cursorBehavior == CURSOR_HIDDEN){
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  }
}

float cameraSpeed = 1.f;
std::vector<InputDispatch> inputFns = {
  InputDispatch{
    .sourceKey = 'B',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0,
    .hasPreq = false,
    .fn = [&state]() -> void {
      state.useDefaultCamera = !state.useDefaultCamera;
      std::cout << "Camera option: " << (state.useDefaultCamera ? "default" : "new") << std::endl;
      if (state.useDefaultCamera){
        state.activeCameraObj = NULL;
        state.activeCameraData = NULL;
      }else{
        nextCamera();
      }
    }
  }, 
  InputDispatch{
    .sourceKey = 'N',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0,
    .hasPreq = false,
    .fn = [&state]() -> void {
      if (!state.useDefaultCamera){
        nextCamera();
      }
    }
  },
  InputDispatch{
    .sourceKey = 'G',  // G 
    .sourceType = BUTTON_PRESS,
    //.prereqKey = 341,  // ctrl,
    .hasPreq = false,
    .fn = [&state]() -> void {
      std::cout << "mode set to translate" << std::endl;
      state.manipulatorMode = TRANSLATE;
      sendNotifyMessage("alert", "mode: translate");
    }
  },
  InputDispatch{
    .sourceKey = 'R',  // R
    .sourceType = BUTTON_PRESS,
    //.prereqKey = 341,  // ctrl,
    .hasPreq = false,
    .fn = [&state]() -> void {
      std::cout << "mode set to rotate" << std::endl;
      state.manipulatorMode = ROTATE;
      sendNotifyMessage("alert", "mode: rotate");
    }
  },
  InputDispatch{
    .sourceKey = 'T',  // T
    .sourceType = BUTTON_PRESS,
    //.prereqKey = 341,  // ctrl,
    .hasPreq = false,
    .fn = [&state]() -> void {
      std::cout << "mode set to scale" << std::endl;
      state.manipulatorMode = SCALE;
      sendNotifyMessage("alert", "mode: scale");
    }
  },
  InputDispatch{
    .sourceKey = '1', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      state.renderMode =  RENDER_FINAL;
      std::cout << "render mode: final" << std::endl;
    }
  }, 
  InputDispatch{
    .sourceKey = '2',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      state.renderMode = RENDER_DEPTH;
      std::cout << "render mode: depth" << std::endl;
    }
  }, 
  InputDispatch{
    .sourceKey = '3',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      state.renderMode = RENDER_PORTAL;
      std::cout << "render mode: portal" << std::endl;
    }
  }, 
  InputDispatch{
    .sourceKey = '4', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      state.renderMode = RENDER_PAINT;
      std::cout << "render mode: paint" << std::endl;
    }
  }, 
  InputDispatch{
    .sourceKey = '5', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      state.renderMode = RENDER_BLOOM;
      std::cout << "render mode: bloom" << std::endl;
    }
  },
  InputDispatch{
    .sourceKey = '6', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      state.renderMode = RENDER_GRAPHS;
      std::cout << "render mode: graph" << std::endl;
    }
  }, 
  InputDispatch{
    .sourceKey = 67,  // 4
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,
    .hasPreq = true,
    .fn = [&state]() -> void {
      handleClipboardSelect();
    }
  }, 
  InputDispatch{
    .sourceKey = 86,  // 4
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,
    .hasPreq = true,
    .fn = [&state]() -> void {
      handleCopy();
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
    .sourceKey = 'W',  // w
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
    .sourceKey = 'A',  // a
    .sourceType = BUTTON_HOLD,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = [&state]() -> void {
      moveCamera(glm::vec3(cameraSpeed * -40.0 * deltaTime, 0.0, 0.0));
    }
  },  
  InputDispatch{
    .sourceKey = 'S',  // s
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
    .sourceKey = 'D',  // d
    .sourceType = BUTTON_HOLD,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = [&state]() -> void {
      moveCamera(glm::vec3(cameraSpeed * 40.0f * deltaTime, 0.0, 0.0f));
    }
  },  
  InputDispatch{
    .sourceKey = 'S',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,
    .hasPreq = true,
    .fn = [&state]() -> void {
      std::cout << "saving heightmap" << std::endl;
      auto selectedId = latestSelected(state.editor);
      if (selectedId.has_value() && isHeightmap(world, selectedId.value())){
        saveHeightmap(world, selectedId.value(), "./res/heightmaps/testmap.png");
      }
    }
  }, 
  InputDispatch{
    .sourceKey = 'J',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 'R',  
    .hasPreq = true,
    .fn = [&state]() -> void {
      std::cout << "rechunking data!" << std::endl;
      auto selectedObjects = selectedIds(state.editor);

      std::vector<objid> selectedHeightmaps;
      std::vector<objid> selectedVoxels;
      for (auto id : selectedObjects){
        if (isHeightmap(world, id)){
          selectedHeightmaps.push_back(id);
        }else if (isVoxel(world, id)){
          selectedVoxels.push_back(id);
        }
      }
      std::cout << "want to join heightmaps (size = " << selectedHeightmaps.size() << ") = [ ";
      std::vector<HeightMapData> heightmaps;
      for (auto id : selectedHeightmaps){
        std::cout << id << " ";
        heightmaps.push_back(getHeightmap(world, id).heightmap);
      }
      std::cout << "]" << std::endl;

      std::vector<objid> voxels;
      for (auto id : selectedVoxels){
        voxels.push_back(id);
      }

      if (heightmaps.size() > 0){
        auto heightmapData = joinHeightmaps(heightmaps);
        saveHeightmap(heightmapData, "./res/heightmaps/joinedmap.png");
      }
      if (voxels.size() > 0){
        std::cout << "joined voxel data! size = " << voxels.size() << std::endl;
        std::vector<Voxels> voxelBodies;
        std::vector<Transformation> transforms;
        for (auto id : voxels){
          voxelBodies.push_back(getVoxel(world, id).value() -> voxel);
          transforms.push_back(gameobjectTransformation(world, id, false));
        }
        auto voxelData = joinVoxels(voxelBodies, transforms);
        std::cout << "serialized voxel: \n" << serializeVoxelDefault(world, voxelData) << std::endl;
      }

    }
  },
  InputDispatch{
    .sourceKey = 'S',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 'R',  
    .hasPreq = true,
    .fn = [&state]() -> void {
      std::cout << "splitting data!" << std::endl;
      auto objectId = latestSelected(state.editor);
      if (!objectId.has_value()){
        std::cout << "no object to split" << std::endl;
        return;
      }
      if (isHeightmap(world, objectId.value())){
        std::string heightmapBaseName = ". /res/heightmaps/";
        std::cout << "want to split heightmap: " << objectId.value() << std::endl;
        auto hm = getHeightmap(world, objectId.value());
        auto newHeightmaps = splitHeightmap(hm.heightmap);
        for (int i = 0; i < newHeightmaps.size(); i++){
          auto newHm = newHeightmaps.at(i);
          auto newMapPath = heightmapBaseName + "splitmap_" + std::to_string(i) + ".png";
          saveHeightmap(newHm, newMapPath);
        }
      }else if (isVoxel(world, objectId.value())){
        std::cout << "Splitting voxel:" << std::endl;
        auto voxel = getVoxel(world, objectId.value());
        if (voxel.has_value()){
          auto voxels = voxel.value();
          auto localTransform = gameobjectTransformation(world, objectId.value(), false);
          auto voxelFragments = splitVoxel(voxels -> voxel, localTransform, 2);
          auto newVoxels = groupVoxelChunks(voxelFragments);
          std::cout << "Voxel fragments size: " << voxelFragments.size() << std::endl;
          std::cout << "New Voxels size: " << newVoxels.size() << std::endl;

          for (auto voxelFragment : newVoxels){
            std::cout << "serialized voxel: \n" << serializeVoxelDefault(world, voxelFragment.voxel) << std::endl;
          }
        }
      }
    }
  },
  InputDispatch{
    .sourceKey = 'A',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      std::cout << "setting snap absolute" << std::endl;
      state.snappingMode = SNAP_ABSOLUTE;
    }
  },   
  InputDispatch{
    .sourceKey = 'C',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = [&state]() -> void {
      std::cout << "setting snap continuous" << std::endl;
      state.snappingMode = SNAP_CONTINUOUS;
    }
  },   
  InputDispatch{
    .sourceKey = 'R',  
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
        handleSnapEasy(id, true);
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
        handleSnapEasy(id, false);
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
    .sourceKey = GLFW_KEY_LEFT_ALT,  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = [&cameraSpeed, &state]() -> void {
      state.cameraFast = !state.cameraFast;
      std::cout << "camera fast: " << state.cameraFast << std::endl;
      cameraSpeed = state.cameraFast ? 1.f : 0.1f;
      sendNotifyMessage("alert", std::string("camera speed: ") + (state.cameraFast ? "fast" : "slow"));
    }
  },  
  InputDispatch{
    .sourceKey = 341,  // ctrl
    .sourceType = BUTTON_RELEASE,
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
      for (auto id : selectedIds(state.editor)){
        std::cout << "delete object id: " << id << std::endl;
        removeObjectById(id);
      }
      clearSelectedIndexs(state.editor);   
    }
  },
  InputDispatch{
    .sourceKey = 'O',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.showDebug = !state.showDebug;
    }
  },
  /*InputDispatch{
    .sourceKey = 'O',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {

      //state.drawPoints = !state.drawPoints;
      //std::cout << "draw points: " << state.drawPoints << std::endl;
    }
  },*/
  InputDispatch{
    .sourceKey = ';',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      std::cout << "enforcing layouts" << std::endl;
      enforceAllLayouts(world);
    }
  },

  InputDispatch{
    .sourceKey = 322,  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      snapCameraDown(setCameraRotation);
      sendNotifyMessage("alert", "snap camera: -y");
    }
  },
  InputDispatch{
    .sourceKey = 322,  
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT, 
    .hasPreq = true,
    .fn = []() -> void {
      onManipulatorEvent(state.manipulatorState, tools, OBJECT_ORIENT_DOWN);
      sendNotifyMessage("alert", "set orientation: -y");
    }
  },
  InputDispatch{
    .sourceKey = 324,  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      snapCameraLeft(setCameraRotation);
      sendNotifyMessage("alert", "snap camera: -x");
    }
  },
  InputDispatch{
    .sourceKey = 324,  
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT, 
    .hasPreq = true,
    .fn = []() -> void {
      onManipulatorEvent(state.manipulatorState, tools, OBJECT_ORIENT_LEFT);
      sendNotifyMessage("alert", "set orientation: -x");
    }
  },
  InputDispatch{
    .sourceKey = 326, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      snapCameraRight(setCameraRotation);
      sendNotifyMessage("alert", "snap camera: +x");
    }
  },
  InputDispatch{
    .sourceKey = 326, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      onManipulatorEvent(state.manipulatorState, tools, OBJECT_ORIENT_RIGHT);
      sendNotifyMessage("alert", "set orientation: +x");
    }
  },
  InputDispatch{
    .sourceKey = 327, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      snapCameraForward(setCameraRotation);
      sendNotifyMessage("alert", "snap camera: -z");
    }
  },
  InputDispatch{
    .sourceKey = 327, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      onManipulatorEvent(state.manipulatorState, tools, OBJECT_ORIENT_FORWARD);
      sendNotifyMessage("alert", "set orientation: -z");
    }
  },
  InputDispatch{
    .sourceKey = 328,  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      snapCameraUp(setCameraRotation);
      sendNotifyMessage("alert", "snap camera: +y");
    }
  },
  InputDispatch{
    .sourceKey = 328, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      onManipulatorEvent(state.manipulatorState, tools, OBJECT_ORIENT_UP);
      sendNotifyMessage("alert", "set orientation: +y");
    }
  },
  InputDispatch{
    .sourceKey = 329,  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      snapCameraBackward(setCameraRotation);
      sendNotifyMessage("alert", "snap camera: +z");
    }
  },
  InputDispatch{
    .sourceKey = 329, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT,
    .hasPreq = true,
    .fn = []() -> void {
      onManipulatorEvent(state.manipulatorState, tools, OBJECT_ORIENT_BACK);
      sendNotifyMessage("alert", "set orientation: +z");
    }
  },
  InputDispatch{
    .sourceKey = '0', // zero 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      std::cout << dumpDebugInfo(false) << std::endl;
      sendNotifyMessage("alert", "*dumped debug data to console*");
    }
  },
  InputDispatch{
    .sourceKey = 348, // to the right of fn key, looks like notepad
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.printKeyStrokes = !state.printKeyStrokes;
      std::cout << "print key strokes: " << state.printKeyStrokes << std::endl;
    }
  },
  InputDispatch{
    .sourceKey = '\\', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.cullEnabled = !state.cullEnabled;
      setCulling(state.cullEnabled);
    }
  },
  InputDispatch{
    .sourceKey = 'U', // u
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      if (state.manipulatorMode == TRANSLATE){
        if (state.manipulatorPositionMode == SNAP_CONTINUOUS){
          state.manipulatorPositionMode = SNAP_RELATIVE;
          sendNotifyMessage("alert", std::string("snap positions: on - relative"));
        }else if (state.manipulatorPositionMode == SNAP_RELATIVE){
          state.manipulatorPositionMode = SNAP_ABSOLUTE;
          sendNotifyMessage("alert", std::string("snap positions: on - absolute"));
        }else if (state.manipulatorPositionMode == SNAP_ABSOLUTE){
          state.manipulatorPositionMode = SNAP_CONTINUOUS;
          sendNotifyMessage("alert", std::string("snap positions: off"));
        }
      }else if (state.manipulatorMode == SCALE){
        state.snapManipulatorScales = !state.snapManipulatorScales;
        sendNotifyMessage("alert", std::string("snap scales: ") + (state.snapManipulatorScales ? "enabled" : "disabled"));
      }else if (state.manipulatorMode == ROTATE){
        if (state.rotateMode == SNAP_CONTINUOUS){
          state.rotateMode = SNAP_RELATIVE;
          sendNotifyMessage("alert", std::string("snap rotate: on - relative"));
        }else if (state.rotateMode == SNAP_RELATIVE){
          state.rotateMode = SNAP_ABSOLUTE;
          sendNotifyMessage("alert", std::string("snap rotate: on - absolute"));
        }else if (state.rotateMode == SNAP_ABSOLUTE){
          state.rotateMode = SNAP_CONTINUOUS;
          sendNotifyMessage("alert", std::string("snap rotate: off"));
        }
      }
    }
  },
  InputDispatch{
    .sourceKey = '[',
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      if (state.manipulatorMode == TRANSLATE){
      }else if (state.manipulatorMode == SCALE){
        state.preserveRelativeScale = !state.preserveRelativeScale;
        sendNotifyMessage("alert", std::string("scale preserve relative: ") + (state.preserveRelativeScale ? "enabled" : "disabled"));
      }else if (state.manipulatorMode == ROTATE){
      }
    }
  },
  InputDispatch{
    .sourceKey = ']',
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      if (state.manipulatorMode == TRANSLATE){
      }else if (state.manipulatorMode == SCALE){
        if (state.scalingGroup == INDIVIDUAL_SCALING){
          state.scalingGroup = GROUP_SCALING;
          sendNotifyMessage("alert", "scaling grouping: group");
        }else if (state.scalingGroup == GROUP_SCALING){
          state.scalingGroup = INDIVIDUAL_SCALING;
          sendNotifyMessage("alert", "scaling grouping: individual");
        }
      }else if (state.manipulatorMode == ROTATE){
      }
    }
  },
  InputDispatch{
    .sourceKey = 'I', // i
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.enableBloom = !state.enableBloom;
      std::cout << "bloom: " << state.enableBloom << std::endl;
      sendNotifyMessage("alert", std::string("bloom: ") + (state.enableBloom ? "enabled" : "disabled"));
    }
  },
  InputDispatch{
    .sourceKey = 'H', // h
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.enableDiffuse = !state.enableDiffuse;
      std::cout << "diffuse: " << state.enableDiffuse << std::endl;
      sendNotifyMessage("alert", std::string("diffuse: ") + (state.enableDiffuse ? "enabled" : "disabled"));
    }
  },
  InputDispatch{
    .sourceKey = 'J', // j
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.enableSpecular = !state.enableSpecular;
      std::cout << "specular: " << state.enableSpecular << std::endl;
      sendNotifyMessage("alert", std::string("specular: ") + (state.enableSpecular ? "enabled" : "disabled"));
    }
  },
  InputDispatch{
    .sourceKey = 'F', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.useBoneTransform = !state.useBoneTransform;
      std::cout << "state: use bone transform: " << state.useBoneTransform << std::endl;
    }
  },
  InputDispatch{
    .sourceKey = 'G', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.showBoneWeight = !state.showBoneWeight;
      std::cout << "state: show bone weight " << state.showBoneWeight << std::endl;
    }
  },
  InputDispatch{
    .sourceKey = 'X', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.manipulatorAxis = XAXIS;
      sendNotifyMessage("alert", "axis: XAXIS");
    }
  },
  InputDispatch{
    .sourceKey = 'Y', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.manipulatorAxis = YAXIS;
      sendNotifyMessage("alert", "axis: YAXIS");
    }
  },
  InputDispatch{
    .sourceKey = 'Z', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.manipulatorAxis = ZAXIS;
      sendNotifyMessage("alert", "axis: ZAXIS");
    }
  },
  InputDispatch{
    .sourceKey = GLFW_KEY_RIGHT, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      onArrowKey(GLFW_KEY_RIGHT);
    }
  },
  InputDispatch{
    .sourceKey = GLFW_KEY_LEFT, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      onArrowKey(GLFW_KEY_LEFT);
    }
  },
  InputDispatch{
    .sourceKey = GLFW_KEY_UP, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      onArrowKey(GLFW_KEY_UP);
    }
  },
  InputDispatch{
    .sourceKey = GLFW_KEY_DOWN, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      onArrowKey(GLFW_KEY_DOWN);
    }
  },
  InputDispatch{
    .sourceKey = 'R', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.moveRelativeEnabled = !state.moveRelativeEnabled;
      std::cout << "move relative: " << state.moveRelativeEnabled << std::endl;
    }
  },
  InputDispatch{
    .sourceKey = 'L', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      auto ids = selectedIds(state.editor);
      std::cout << "l pressed" << std::endl;
      if (ids.size() > 0){
        std::cout << "setting pose!" << std::endl;
        updateBonePose(world, ids.at(0));
      }
    }
  },
  InputDispatch{
    .sourceKey = 'Q', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.useYAxis = !state.useYAxis;
      if (timePlayback.isPaused()){
        timePlayback.play();
      }else{
        timePlayback.pause();
      }
      std::cout << "group selection: " << state.groupSelection << std::endl;
      //state.groupSelection = !state.groupSelection;
    }
  },
  InputDispatch{
    .sourceKey = 'P', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.worldpaused = !state.worldpaused;
      std::cout << "worldpaused: " << (state.worldpaused ? "true" : "false") << std::endl;
    }
  },

  // Testing offline scene functions
  InputDispatch{
    .sourceKey = 'N', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 'L', 
    .hasPreq = true,
    .fn = []() -> void {
      //offlineNewScene("./build/testscene.rawscene");

      std::cout << "move element with parent" << std::endl;
      /*offlineMoveElementAndChildren(
        "./res/scenes/world/elementwithparent.rawscene", 
        "./res/scenes/world/empty.rawscene", 
        "someparentelement"
      );*/
      /*offlineUpdateElementAttributes(
        "./res/scenes/world/elementwithparent.rawscene",
        "someparentelement",
        {{ "anewfield", "regular_field" }}
      );*/

      auto elements = offlineGetElementsNoChildren("./res/scenes/world/elementwithparent.rawscene", interface.readFile);
      std::cout << "elements in scene that are not children" << std::endl;
      for (auto element : elements){
        std::cout << element << " ";
      }
      if (elements.size() == 0){
        std::cout << "(none)";
      }
      std::cout << std::endl;

      //offlineMoveElement(
      //  "./res/scenes/world/voxelchunksmall_000.rawscene", 
      //  "./res/scenes/world/empty.rawscene", 
      //  "]default_voxel"
      //);
    }
  },
  InputDispatch{
    .sourceKey = 'D', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 'L', 
    .hasPreq = true,
    .fn = []() -> void {
      offlineDeleteScene("./build/testscene.rawscene");
    }
  },
  InputDispatch{
    .sourceKey = 'C', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 'L', 
    .hasPreq = true,
    .fn = []() -> void {
      offlineCopyScene("./build/testscene.rawscene", "./build/testscene2.rawscene", interface.readFile);
    }
  },
  InputDispatch{
    .sourceKey = 'R', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 'L', 
    .hasPreq = true,
    .fn = []() -> void {
      offlineRemoveElement("./build/testscene.rawscene", "someitem", interface.readFile);
    }
  },
  InputDispatch{
    .sourceKey = 'A', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 'L', 
    .hasPreq = true,
    .fn = []() -> void {
      offlineSetElementAttributes("./build/testscene.rawscene", "someitem", { {"one", "1" }, {"two", "2" }, {"3", "three"}}, interface.readFile);
    }
  },
  InputDispatch{
    .sourceKey = GLFW_KEY_ENTER, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT,
    .hasPreq = true,
    .fn = []() -> void {
      state.fullscreen = !state.fullscreen;
      toggleFullScreen(state.fullscreen);
    }
  },
  InputDispatch{
    .sourceKey = 'C',
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT,
    .hasPreq = true,
    .fn = []() -> void {
      if (state.cursorBehavior == CURSOR_NORMAL){
        state.cursorBehavior = CURSOR_CAPTURE;
      }else if (state.cursorBehavior == CURSOR_CAPTURE){
        state.cursorBehavior = CURSOR_HIDDEN;
      }else if (state.cursorBehavior == CURSOR_HIDDEN){
        state.cursorBehavior = CURSOR_NORMAL;
      }
      toggleCursor(state.cursorBehavior);
      if (state.cursorBehavior == CURSOR_NORMAL){
        sendNotifyMessage("alert", std::string("capture cursor: cursor normal"));
      }else if (state.cursorBehavior == CURSOR_CAPTURE){
        sendNotifyMessage("alert", std::string("capture cursor: cursor capture"));
      }else if (state.cursorBehavior == CURSOR_HIDDEN){
        sendNotifyMessage("alert", std::string("capture cursor: cursor hidden"));
      }
      
    }
  },
  InputDispatch{
    .sourceKey = 'R',
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT,
    .hasPreq = true,
    .fn = []() -> void {
      std::cout << renderStagesToString(renderStages) << std::endl;
    }
  },
  InputDispatch{
    .sourceKey = 'S',
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT,
    .hasPreq = true,
    .fn = []() -> void {
      auto styles = loadStyles("./res/test.style", interface.readFile);
      auto tokens = parseFormat(interface.readFile("./res/scenes/example.p.rawscene"));
      applyStyles(tokens, styles);
      auto serializedContent = serializeSceneTokens(tokens);
      std::cout << serializedContent << std::endl;
    }
  },
  InputDispatch{
    .sourceKey = 258,
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0,
    .hasPreq = false,
    .fn = []() -> void {
      state.selectionDisabled = true;
    }
  },
  InputDispatch{
    .sourceKey = 258,
    .sourceType = BUTTON_RELEASE,
    .prereqKey = 0,
    .hasPreq = false,
    .fn = []() -> void {
      state.selectionDisabled = false;
    }
  },
};

