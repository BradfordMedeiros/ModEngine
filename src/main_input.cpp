#include "./main_input.h"

extern World world;
extern RenderStages renderStages;
extern engineState state;
extern CScriptBindingCallbacks cBindings;
extern KeyRemapper keyMapper;
extern glm::mat4 view;
extern DefaultResources defaultResources;
extern Stats statistics;
extern Benchmark benchmark;
extern bool selectItemCalled;
extern DynamicLoading dynamicLoading;
extern SysInterface interface;
extern GLFWwindow* window;
extern LineData lineData;
extern ManipulatorTools tools;
extern TimePlayback timePlayback;

std::string readFileOrPackage(std::string filepath);

std::string dumpDebugInfo(bool fullInfo){
  // this line is commented out b/c was segfaulting, probably should be written in a way that assumes the structure might be invalid
  auto scenegraphInfo = std::string("final scenegraph\n") + scenegraphAsDotFormat(world.sandbox, world.objectMapping) + "\n\n";
  auto gameobjInfo = debugAllGameObjects(world.sandbox);
  auto gameobjhInfo = debugAllGameObjectsH(world.sandbox);
  auto cacheInfo = debugTransformCache(world.sandbox);
  auto textureInfo = debugLoadedTextures(world.textures);
  auto meshInfo = debugLoadedMeshes(world.meshes);
  auto animationInfo = debugAnimations(world.animations);
  auto physicsInfo = debugPhysicsInfo(world.rigidbodys);
  auto sceneInfo = debugSceneInfo(world.sandbox);

  auto benchmarkingContent = benchmarkResult(benchmark);
  auto profilingInfo = fullInfo ? dumpProfiling() : "" ;
//
    auto content = "gameobj info - id id name\n" + gameobjInfo + "\n" + 
      "scenegraph info\n" + scenegraphInfo + "\n" +
      "gameobjh info - id id sceneId groupId parentId | [children]\n" + gameobjhInfo + "\n" +
      "transform cache - id pos scale\n" + cacheInfo + "\n" + 
      "texture cache\n" + textureInfo + "\n" +
      "mesh cache\n" + meshInfo + "\n" + 
      "animation cache\n" + animationInfo + "\n" +
      "physics info\n" + physicsInfo + "\n" + 
      "scene info\n" + sceneInfo + "\n";
//    sceneInfo +  benchmarkingContent + "\n" + profilingInfo;
  return content;

}

void debugInfo(std::optional<std::string> filepath){
  if (filepath.has_value()){
    realfiles::saveFile(filepath.value(), dumpDebugInfo(false));
  }else{
    std::cout << dumpDebugInfo(false) << std::endl;
  }
}

void onMouse(engineState& state, double xpos, double ypos, void(*rotateCamera)(float, float)){
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
    
    if (state.isRotateSelection || state.inputMode == CAMERA_ONLY){
      if (state.inputMode == ENABLED || state.inputMode == CAMERA_ONLY){
        float rotateSensitivity = 0.05;
        rotateCamera(xoffset * rotateSensitivity, -yoffset * rotateSensitivity);   // -y offset because mouse move forward is negative, which is ok, but inverted        
      }
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

bool hasMouseEvent = false;
void onMouseEvents(GLFWwindow* window, double xpos, double ypos){
  hasMouseEvent = true;
}

void handleMouseEvents(){
  double xpos = 0;
  double ypos = 0;
  glfwGetCursorPos(window, &xpos, &ypos);

  onMouse(state, xpos, ypos, rotateCamera); 
  std::cout << "frame xpos = " << xpos << ", ypos = " << ypos << std::endl;

  hasMouseEvent = false;
}


void onMouse(int button, int action, int mods){
  if (button == 0 && action == 1){
    state.mouseIsDown = true;
  }else if (button == 0 && action == 0){
    state.mouseIsDown = false;
  }

  mouse_button_callback(state, button, action, mods, onMouseButton);
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
    selectItemCalled = true;
    onManipulatorMouseDown(state.manipulatorState);
  }
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
    onManipulatorMouseRelease(state.manipulatorState);
  }

  modlog("input", "mouse callback: " + std::to_string(button) + ", action: " + std::to_string(action) + ", mods: " + std::to_string(mods));
  cBindings.onMouseCallback(button, action, mods);
}
void onMouseCallback(GLFWwindow* window, int button, int action, int mods){
  onMouse(button, action, mods);
}

void dispatchClick(int button, int action){
  std::cout << "dispatch click placeholder" << std::endl;
  onMouse(button, action,  0/*mods*/); // since this forces a click, could get into weird edge cases in behavior
}


void onSelectNullItem(){
  state.editor.selectedObjs = {};
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

void mouse_button_callback(engineState& state, int button, int action, int mods, void (*handleSerialization) (void)){
  if (state.inputMode != ENABLED && state.inputMode != CAMERA_ONLY){
    return;
  }
  if (state.inputMode == ENABLED && button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
    handleSerialization();
    onSelectNullItem();
  }

  if (button == GLFW_MOUSE_BUTTON_MIDDLE){
    if (action == GLFW_PRESS){
      state.isRotateSelection = true;
    }else if (action == GLFW_RELEASE){
      state.isRotateSelection = false;
    }

    if (state.inputMode == ENABLED){
      moveCursor(state.currentScreenWidth / 2, state.currentScreenHeight / 2);
    }
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
  modlog("joystick", "onJoystick");
  for (auto info : infos){
    modlog("joystick", std::to_string(info.index) + ", " + std::to_string(info.value));
  }
}

void onArrowKey(int key){
  if (key == 262){ // right
    std::cout << "next texture" << std::endl;
    setState("next_texture");
  }
  if (key == 263){ // left
    std::cout << "previous texture" << std::endl;
    setState("prev_texture");
  }

  std::cout << "key: " << key << std::endl;
}


void onScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
  cBindings.onScrollCallback(yoffset);

  for (auto &selectedIndex : state.editor.selectedObjs){
    if (idExists(world.sandbox, selectedIndex) && getLayerForId(selectedIndex).selectIndex != -2){ 
      maybeChangeTexture(selectedIndex);
    }
  }
 
  auto octreeId = getSelectedOctreeId();
  if (octreeId.has_value()){
    GameObjectOctree* octreeObject = getOctree(world.objectMapping, octreeId.value());
    if (octreeObject != NULL){
      auto isShiftHeld = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
      auto isCtrlHeld = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
      OctreeDimAxis axis = OCTREE_NOAXIS;
      if (state.manipulatorAxis == XAXIS){
        axis = OCTREE_XAXIS;
      }else if (state.manipulatorAxis == YAXIS){
        axis = OCTREE_YAXIS;
      }else if (state.manipulatorAxis == ZAXIS){
        axis = OCTREE_ZAXIS;
      }
      handleOctreeScroll(*octreeObject, octreeObject -> octree, yoffset > 0, createScopedLoadMesh(world, octreeId.value()), isShiftHeld, isCtrlHeld, axis);
      if (state.rebuildOctreePhysicsOnEdit){
        updatePhysicsBody(world, octreeId.value());
      }
    }
  }
}


void keyCharCallback(unsigned int codepoint){
  cBindings.onKeyCharCallback(codepoint);
  //std::cout << "Key is: " << codepoint << std::endl;
}
void keyCharCallback(GLFWwindow* window, unsigned int codepoint){
  keyCharCallback(codepoint);
}

void handleSnapEasy(objid id, bool left){
  if (getLayerForId(id).selectIndex == -2){
    return;
  }
  if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
    auto objPos = getGameObjectPosition(id, true, "handleSnapEasy");
    auto snapAmount = left ? snapTranslateDown(state.easyUse, state.snappingMode, objPos, state.manipulatorAxis) : snapTranslateUp(state.easyUse, state.snappingMode, objPos, state.manipulatorAxis);
    setGameObjectPosition(id, snapAmount, true, Hint { .hint = "handleSnapEasy position" });
  }else if (state.manipulatorMode == ROTATE){
    auto objRot = getGameObjectRotation(id, true, "handleSnapEasy rotate");
    auto snapAmount = left ? snapAngleDown(state.easyUse, state.snappingMode, objRot, state.manipulatorAxis) : snapAngleUp(state.easyUse, state.snappingMode, objRot, state.manipulatorAxis);
    setGameObjectRotation(id, snapAmount, false, Hint { .hint = "handleSnapEasy rotate" });
  }else if (state.manipulatorMode == SCALE){
    auto objScale = getGameObjectScale(id);
    auto snapAmount = left ? snapScaleDown(state.easyUse, state.snappingMode, objScale, state.manipulatorAxis) : snapScaleUp(state.easyUse, state.snappingMode, objScale, state.manipulatorAxis);
    setGameObjectScale(id, snapAmount, true);  // THis might be wrong
    std::cout << "WARNING: SNAP TRANSLATE SET INPUT HANDLING MIGHT BE WRONG (world vs local)" << std::endl;
  }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
  cBindings.onKeyCallback(getKeyRemapping(keyMapper, key), scancode, action, mods);

  if (state.inputMode != ENABLED){
    return;
  }
  if (state.printKeyStrokes){
    std::cout << "key: " << key << " action: " << action << std::endl;
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
    state.editor.selectedObjs = {};
  }

  if (key == 340){
    state.moveUp = (action == 1);
  }
}

float currentMouseDepth(){
  return state.currentCursorDepth.value();
}

glm::vec3 getMouseDirectionWorld(){
  float depth = currentMouseDepth();

  auto id = state.currentHoverIndex;
  bool objExists = idExists(world.sandbox, id);
  auto layer = objExists ? layerByName(world, getGameObject(world, id).layer) : layerByName(world, ""); // "" default layer
  modlog("layer is", layer.name);
  auto proj = projectionFromLayer(layer);
  auto rayDirection = getCursorInfoWorld(proj, view, state.cursorLeft, state.currentScreenHeight - state.cursorTop, state.currentScreenWidth, state.currentScreenHeight, depth).direction;
  return rayDirection;
}

glm::vec3 getMousePositionWorld(){
  float depth = currentMouseDepth();

  auto id = state.currentHoverIndex;
  bool objExists = idExists(world.sandbox, id);
  auto layer = objExists ? layerByName(world, getGameObject(world, id).layer) : layerByName(world, ""); // "" default layer
  modlog("layer is", layer.name);
  auto proj = projectionFromLayer(layer);
  auto rayDirection = getCursorInfoWorld(proj, view, state.cursorLeft, state.currentScreenHeight - state.cursorTop, state.currentScreenWidth, state.currentScreenHeight, depth).projectedPosition;
  return rayDirection;
}


void doOctreeRaycast(World& world, objid id, glm::vec3 fromPos, glm::vec3 toPos){
  if (!idExists(world.sandbox, id) || (!isOctree(world, id))){
    return;
  }
  auto octreeModelMatrix = fullModelTransform(world.sandbox, id);
  auto adjustedPosition = glm::inverse(octreeModelMatrix) * glm::vec4(fromPos.x, fromPos.y, fromPos.z, 1.f);
  auto adjustedToPos = glm::inverse(octreeModelMatrix) * glm::vec4(toPos.x, toPos.y, toPos.z, 1.f);
  auto adjustedDir = adjustedToPos - adjustedPosition;

  GameObjectOctree* octreeObj = getOctree(world.objectMapping, id);
  if (octreeObj){
    modassert(octreeObj, "draw selection grid onFrame not octree type");
    handleOctreeRaycast(octreeObj -> octree, adjustedPosition, adjustedDir, glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS, id);
    setSelectedOctreeId(id);       
  }
}

bool enableSelectionVisualization = false;
void onMouseButton(){ 
  glm::vec3 fromPos = defaultResources.defaultCamera.transformation.position;
  auto rayPosition = getMousePositionWorld();
  if (enableSelectionVisualization){
    static objid lineId = 0;
    if (lineId){
      freeLine(lineData, lineId);
    }
    lineId = addLineNextCycle(fromPos, rayPosition, true, -1, glm::vec4(1.f, 1.f, 0.f, 1.f), std::nullopt, std::nullopt);
  }
  doOctreeRaycast(world, state.currentHoverIndex, fromPos, rayPosition);
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
        .attr = {{ "clip", std::string(paths[i]) }}, 
      };
      std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
      makeObjectAttr(sceneId, "&" + objectName, attr, submodelAttributes);
    }else if (fileType == MODEL_EXTENSION){
      GameobjAttributes attr {
        .attr = {{ "mesh", std::string(paths[i]) }}, 
      };
      std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
      makeObjectAttr(sceneId, objectName, attr, submodelAttributes);
    }else if (fileType == UNKNOWN_EXTENSION){
      std::cout << "UNKNOWN file format, so doing nothing: " << paths[i] << std::endl;
    }
  }
}

void handleInput(GLFWwindow* window){
  if (state.escapeQuits && glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
    glfwSetWindowShouldClose(window, true);
  }
  processControllerInput(keyMapper, moveCamera, statistics.deltaTime, keyCharCallback, onJoystick);
  processKeyBindings(window, keyMapper);
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
  std::unordered_map<int, bool> lastFrameDown = {};
  for (auto inputFn : remapper.inputFns){
    if (state.inputMode == DISABLED && !inputFn.alwaysEnable){
      continue;
    }
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

bool enableDebugCommands(){
  return false;
}

float cameraSpeed = 1.f;
std::vector<InputDispatch> inputFns = {
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'B',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0,
    .hasPreq = false,
    .fn = []() -> void {
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
    .alwaysEnable = false,
    .sourceKey = 'N',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0,
    .hasPreq = false,
    .fn = []() -> void {
      if (!state.useDefaultCamera){
        nextCamera();
      }
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'M',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0,
    .hasPreq = false,
    .fn = []() -> void {
      auto ids = state.editor.selectedObjs;
      if (ids.size() > 0){
        state.cullingObject = ids.at(0);
      }else{
        state.cullingObject = std::nullopt;
      }
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'G',  // G 
    .sourceType = BUTTON_PRESS,
    //.prereqKey = 341,  // ctrl,
    .hasPreq = false,
    .fn = []() -> void {
      std::cout << "mode set to translate" << std::endl;
      state.manipulatorMode = TRANSLATE;
      sendAlert("mode: translate");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'R',  // R
    .sourceType = BUTTON_PRESS,
    //.prereqKey = 341,  // ctrl,
    .hasPreq = false,
    .fn = []() -> void {
      std::cout << "mode set to rotate" << std::endl;
      state.manipulatorMode = ROTATE;
      sendAlert("mode: rotate");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'T',  // T
    .sourceType = BUTTON_PRESS,
    //.prereqKey = 341,  // ctrl,
    .hasPreq = false,
    .fn = []() -> void {
      std::cout << "mode set to scale" << std::endl;
      state.manipulatorMode = SCALE;
      sendAlert("mode: scale");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '1', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = []() -> void {
      state.renderMode =  RENDER_FINAL;
      std::cout << "render mode: final" << std::endl;
    }
  }, 
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '2',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = []() -> void {
      state.renderMode = RENDER_DEPTH;
      std::cout << "render mode: depth" << std::endl;
    }
  }, 
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '3',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = []() -> void {
      state.renderMode = RENDER_PORTAL;
      std::cout << "render mode: portal" << std::endl;
    }
  }, 
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '4', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = []() -> void {
      state.renderMode = RENDER_PAINT;
      std::cout << "render mode: paint" << std::endl;
    }
  }, 
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '5', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = []() -> void {
      state.renderMode = RENDER_BLOOM;
      std::cout << "render mode: bloom" << std::endl;
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '6', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = []() -> void {
      state.renderMode = RENDER_GRAPHS;
      std::cout << "render mode: graph" << std::endl;
    }
  }, 
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '7', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = []() -> void {
      state.renderMode = RENDER_SELECTION;
      std::cout << "render mode: graph" << std::endl;
    }
  }, 
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '8', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = []() -> void {
      state.renderMode = RENDER_TEXTURE;
      std::cout << "render mode: graph" << std::endl;
    }
  }, 
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'C',  // 4
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,
    .hasPreq = true,
    .fn = []() -> void {
      handleClipboardSelect();
    }
  }, 
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 86,  // 4
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,
    .hasPreq = true,
    .fn = []() -> void {
      handleCopy();
    }
  }, 
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 340,  // 4
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.multiselect = true;
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 340,  // 4
    .sourceType = BUTTON_RELEASE,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.multiselect = false;
    }
  },  
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'W',  // w
    .sourceType = BUTTON_HOLD,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      if (state.inputMode != ENABLED && state.inputMode != CAMERA_ONLY){
        return;
      }
      auto speed = cameraSpeed * -40.0f * statistics.deltaTime;
      glm::vec3 moveVec = state.moveUp ? glm::vec3(0.0, -1 * speed, 0.f) : glm::vec3(0.0, 0.0, speed);
      moveCamera(moveVec);
    }
  },  
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'A',  // a
    .sourceType = BUTTON_HOLD,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      if (state.inputMode != ENABLED && state.inputMode != CAMERA_ONLY){
        return;
      }
      moveCamera(glm::vec3(cameraSpeed * -40.0 * statistics.deltaTime, 0.0, 0.0));
    }
  },  
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'S',  // s
    .sourceType = BUTTON_HOLD,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      if (state.inputMode != ENABLED && state.inputMode != CAMERA_ONLY){
        return;
      }
      auto speed = cameraSpeed * 40.0f * statistics.deltaTime;
      glm::vec3 moveVec = state.moveUp ? glm::vec3(0.0, -1 * speed, 0.f) : glm::vec3(0.0, 0.0, speed);
      moveCamera(moveVec);
    }
  },  
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'D',  // d
    .sourceType = BUTTON_HOLD,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      if (state.inputMode != ENABLED && state.inputMode != CAMERA_ONLY){
        return;
      }
      moveCamera(glm::vec3(cameraSpeed * 40.0f * statistics.deltaTime, 0.0, 0.0f));
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'A',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = []() -> void {
      std::cout << "setting snap absolute" << std::endl;
      state.snappingMode = SNAP_ABSOLUTE;
    }
  },   
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'C',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = []() -> void {
      std::cout << "setting snap continuous" << std::endl;
      state.snappingMode = SNAP_CONTINUOUS;
    }
  },   
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'R',  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 340,  // shift,
    .hasPreq = true,
    .fn = []() -> void {
      std::cout << "setting snap relative" << std::endl;
      state.snappingMode = SNAP_RELATIVE;
    }
  }, 
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 263,  // left arrow
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      for (auto id : state.editor.selectedObjs){
        handleSnapEasy(id, true);
      }
    }
  }, 
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 262,  // right arrow
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      for (auto id : state.editor.selectedObjs){
        handleSnapEasy(id, false);
      }
    }
  }, 
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 341,  // ctrl
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.cameraFast = !state.cameraFast;
      std::cout << "camera fast: " << state.cameraFast << std::endl;
      cameraSpeed = state.cameraFast ? 1.f : 0.1f;
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = GLFW_KEY_LEFT_ALT,  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.cameraFast = !state.cameraFast;
      std::cout << "camera fast: " << state.cameraFast << std::endl;
      cameraSpeed = state.cameraFast ? 1.f : 0.1f;
      sendAlert(std::string("camera speed: ") + (state.cameraFast ? "fast" : "slow"));
    }
  },  
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 341,  // ctrl
    .sourceType = BUTTON_RELEASE,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.cameraFast = !state.cameraFast;
      std::cout << "camera fast: " << state.cameraFast << std::endl;
      cameraSpeed = state.cameraFast ? 1.f : 0.1f;
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 261,  // delete
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      for (auto id : state.editor.selectedObjs){
        std::cout << "delete object id: " << id << std::endl;
        removeByGroupId(id);
      }
      state.editor.selectedObjs = {};
    }
  },
  InputDispatch{
    .alwaysEnable = false,
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
    .alwaysEnable = false,
    .sourceKey = 322,  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      snapCameraDown(setCameraRotation);
      sendAlert("snap camera: -y");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 322,  
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT, 
    .hasPreq = true,
    .fn = []() -> void {
      onManipulatorEvent(state.manipulatorState, tools, OBJECT_ORIENT_DOWN);
      sendAlert("set orientation: -y");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 324,  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      snapCameraLeft(setCameraRotation);
      sendAlert("snap camera: -x");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 324,  
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT, 
    .hasPreq = true,
    .fn = []() -> void {
      onManipulatorEvent(state.manipulatorState, tools, OBJECT_ORIENT_LEFT);
      sendAlert("set orientation: -x");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 326, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      snapCameraRight(setCameraRotation);
      sendAlert("snap camera: +x");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 326, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      onManipulatorEvent(state.manipulatorState, tools, OBJECT_ORIENT_RIGHT);
      sendAlert("set orientation: +x");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 327, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      snapCameraForward(setCameraRotation);
      sendAlert("snap camera: -z");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 327, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      onManipulatorEvent(state.manipulatorState, tools, OBJECT_ORIENT_FORWARD);
      sendAlert("set orientation: -z");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 328,  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      snapCameraUp(setCameraRotation);
      sendAlert("snap camera: +y");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 328, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      onManipulatorEvent(state.manipulatorState, tools, OBJECT_ORIENT_UP);
      sendAlert("set orientation: +y");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 329,  
    .sourceType = BUTTON_PRESS,
    .prereqKey = 341,  // ctrl,, 
    .hasPreq = true,
    .fn = []() -> void {
      snapCameraBackward(setCameraRotation);
      sendAlert("snap camera: +z");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 329, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT,
    .hasPreq = true,
    .fn = []() -> void {
      onManipulatorEvent(state.manipulatorState, tools, OBJECT_ORIENT_BACK);
      sendAlert("set orientation: +z");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '0', // zero 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      std::cout << dumpDebugInfo(false) << std::endl;
      modassert(false, "-");
      //std::cout << dumpProfiling() << std::endl;
      //sendAlert("*dumped debug data to console*");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
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
    .alwaysEnable = false,
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
    .alwaysEnable = false,
    .sourceKey = 'U', // u
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      if (state.manipulatorMode == TRANSLATE){
        if (state.manipulatorPositionMode == SNAP_CONTINUOUS){
          state.manipulatorPositionMode = SNAP_RELATIVE;
          sendAlert("snap positions: on - relative");
        }else if (state.manipulatorPositionMode == SNAP_RELATIVE){
          state.manipulatorPositionMode = SNAP_ABSOLUTE;
          sendAlert("snap positions: on - absolute");
        }else if (state.manipulatorPositionMode == SNAP_ABSOLUTE){
          state.manipulatorPositionMode = SNAP_CONTINUOUS;
          sendAlert("snap positions: off");
        }
      }else if (state.manipulatorMode == SCALE){
        state.snapManipulatorScales = !state.snapManipulatorScales;
        sendAlert(std::string("snap scales: ") + (state.snapManipulatorScales ? "enabled" : "disabled"));
      }else if (state.manipulatorMode == ROTATE){
        if (state.rotateMode == SNAP_CONTINUOUS){
          state.rotateMode = SNAP_RELATIVE;
          sendAlert("snap rotate: on - relative");
        }else if (state.rotateMode == SNAP_RELATIVE){
          state.rotateMode = SNAP_ABSOLUTE;
          sendAlert("snap rotate: on - absolute");
        }else if (state.rotateMode == SNAP_ABSOLUTE){
          state.rotateMode = SNAP_CONTINUOUS;
          sendAlert("snap rotate: off");
        }
      }
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '[',
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      if (state.manipulatorMode == TRANSLATE){
      }else if (state.manipulatorMode == SCALE){
        state.preserveRelativeScale = !state.preserveRelativeScale;
        sendAlert(std::string("scale preserve relative: ") + (state.preserveRelativeScale ? "enabled" : "disabled"));
      }else if (state.manipulatorMode == ROTATE){
      }
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = ']',
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      if (state.manipulatorMode == TRANSLATE){
      }else if (state.manipulatorMode == SCALE){
        if (state.scalingGroup == INDIVIDUAL_SCALING){
          state.scalingGroup = GROUP_SCALING;
          sendAlert("scaling grouping: group");
        }else if (state.scalingGroup == GROUP_SCALING){
          state.scalingGroup = INDIVIDUAL_SCALING;
          sendAlert("scaling grouping: individual");
        }
      }else if (state.manipulatorMode == ROTATE){
      }
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'H', // h
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.enableDiffuse = !state.enableDiffuse;
      std::cout << "diffuse: " << state.enableDiffuse << std::endl;
      sendAlert(std::string("diffuse: ") + (state.enableDiffuse ? "enabled" : "disabled"));
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'J', // j
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.enableSpecular = !state.enableSpecular;
      std::cout << "specular: " << state.enableSpecular << std::endl;
      sendAlert(std::string("specular: ") + (state.enableSpecular ? "enabled" : "disabled"));
    }
  },
  InputDispatch{
    .alwaysEnable = false,
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
    .alwaysEnable = false,
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
    .alwaysEnable = false,
    .sourceKey = 'X', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.manipulatorAxis = XAXIS;
      sendAlert("axis: XAXIS");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'Y', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.manipulatorAxis = YAXIS;
      sendAlert("axis: YAXIS");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'Z', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.manipulatorAxis = ZAXIS;
      sendAlert("axis: ZAXIS");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = GLFW_KEY_RIGHT, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      onArrowKey(GLFW_KEY_RIGHT);
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = GLFW_KEY_LEFT, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      onArrowKey(GLFW_KEY_LEFT);
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = GLFW_KEY_UP, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      onArrowKey(GLFW_KEY_UP);
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = GLFW_KEY_DOWN, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      onArrowKey(GLFW_KEY_DOWN);
    }
  },
  InputDispatch{
    .alwaysEnable = false,
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
    .alwaysEnable = false,
    .sourceKey = 'Q', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
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
    .alwaysEnable = false,
    .sourceKey = 'P', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      if (!enableDebugCommands()){
        return;
      }
      state.worldpaused = !state.worldpaused;
      std::cout << "worldpaused: " << (state.worldpaused ? "true" : "false") << std::endl;
    }
  },

  // Testing offline scene functions
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'N', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 'L', 
    .hasPreq = true,
    .fn = []() -> void {
      //offlineNewScene("./build/testscene.rawscene");

      printNavmeshDebug();

      return;
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
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'D', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 'L', 
    .hasPreq = true,
    .fn = []() -> void {
      //offlineDeleteScene("./build/testscene.rawscene");
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'C', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 'L', 
    .hasPreq = true,
    .fn = []() -> void {
      //offlineCopyScene("./build/testscene.rawscene", "./build/testscene2.rawscene", interface.readFile);
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'R', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 'L', 
    .hasPreq = true,
    .fn = []() -> void {
      //offlineRemoveElement("./build/testscene.rawscene", "someitem", interface.readFile);
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'A', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 'L', 
    .hasPreq = true,
    .fn = []() -> void {
     // offlineSetElementAttributes("./build/testscene.rawscene", "someitem", { {"one", "1" }, {"two", "2" }, {"3", "three"}}, interface.readFile);
    }
  },
  InputDispatch{
    .alwaysEnable = true,
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
    .alwaysEnable = false,
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
        sendAlert("capture cursor: cursor normal");
      }else if (state.cursorBehavior == CURSOR_CAPTURE){
        sendAlert("capture cursor: cursor capture");
      }else if (state.cursorBehavior == CURSOR_HIDDEN){
        sendAlert("capture cursor: cursor hidden");
      }
      
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 'R',
    .sourceType = BUTTON_PRESS,
    .prereqKey = GLFW_KEY_LEFT_ALT,
    .hasPreq = true,
    .fn = []() -> void {
      std::cout << renderStagesToString(renderStages) << std::endl;
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 258,
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0,
    .hasPreq = false,
    .fn = []() -> void {
      state.selectionDisabled = true;
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = 258,
    .sourceType = BUTTON_RELEASE,
    .prereqKey = 0,
    .hasPreq = false,
    .fn = []() -> void {
      state.selectionDisabled = false;
    }
  },

  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '-',
    .sourceType = BUTTON_RELEASE,
    .prereqKey = 0,
    .hasPreq = false,
    .fn = []() -> void {
      handleChangeSubdivisionLevel(getCurrentSubdivisionLevel() + 1);
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '=',
    .sourceType = BUTTON_RELEASE,
    .prereqKey = 0,
    .hasPreq = false,
    .fn = []() -> void {
      handleChangeSubdivisionLevel(getCurrentSubdivisionLevel() - 1);
    }
  },

  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = GLFW_KEY_RIGHT, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      auto isCtrlHeld = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
      if (isCtrlHeld){
        increaseSelectionSize(1, 0, 0);
      }else{
        handleMoveOctreeSelection(X_POS);
      }
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = GLFW_KEY_LEFT, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      auto isCtrlHeld = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
      if (isCtrlHeld){
        increaseSelectionSize(-1, 0, 0);
      }else{
        handleMoveOctreeSelection(X_NEG);
      }
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = GLFW_KEY_UP, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      auto isCtrlHeld = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
      if (isCtrlHeld){
        if (state.manipulatorAxis == ZAXIS){
          increaseSelectionSize(0, 0, 1);
        }else{
          increaseSelectionSize(0, 1, 0);
        }
      }else{
        if (state.manipulatorAxis == ZAXIS){
          handleMoveOctreeSelection(Z_POS);
        }else{
          handleMoveOctreeSelection(Y_POS);
        }         
      }
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = GLFW_KEY_DOWN, 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      auto isCtrlHeld = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
      if (isCtrlHeld){
        if (state.manipulatorAxis == ZAXIS){
          increaseSelectionSize(0, 0, -1);
        }else{
          increaseSelectionSize(0, -1, 0);
        }
      }else{
        if (state.manipulatorAxis == ZAXIS){
          handleMoveOctreeSelection(Z_NEG);
        }else{
          handleMoveOctreeSelection(Y_NEG);
        }         
      }
    }
  },

  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '8', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      for (auto &selectedIndex : state.editor.selectedObjs){
        GameObjectOctree* octreeObject = getOctree(world.objectMapping, selectedIndex);
        if (octreeObject != NULL){
          updatePhysicsBody(world, selectedIndex);
        }
      }
      //();
    }
  },

  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '7', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      auto isCtrlHeld = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
      for (auto &selectedIndex : state.editor.selectedObjs){
        GameObjectOctree* octreeObject = getOctree(world.objectMapping, selectedIndex);
        if (octreeObject != NULL){
          if (isCtrlHeld){
            deleteSelectedOctreeNodes(*octreeObject, octreeObject -> octree, createScopedLoadMesh(world, selectedIndex));
            if (state.rebuildOctreePhysicsOnEdit){
              updatePhysicsBody(world, selectedIndex);
            }
          }else{
            insertSelectedOctreeNodes(*octreeObject, octreeObject -> octree, createScopedLoadMesh(world, selectedIndex));
            if (state.rebuildOctreePhysicsOnEdit){
              updatePhysicsBody(world, selectedIndex);
            }
          }
        }
      }
      //();
    }
  },

  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '6', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      auto isCtrlHeld = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
      for (auto &selectedIndex : state.editor.selectedObjs){
        GameObjectOctree* octreeObject = getOctree(world.objectMapping, selectedIndex);
        modassert(octreeObject, "octree object is null");
        writeOctreeTexture(*octreeObject, octreeObject -> octree, createScopedLoadMesh(world, selectedIndex), isCtrlHeld, TEXTURE_UP);
      }
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '4', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      setOctreeTextureId(getOctreeTextureId() - 1);
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '5', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      setOctreeTextureId(getOctreeTextureId() + 1);
    }
  },

  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '2', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      for (auto &selectedIndex : state.editor.selectedObjs){
        GameObjectOctree* octreeObject = getOctree(world.objectMapping, selectedIndex);
        modassert(octreeObject, "octree object is null");
        loadOctree(
          *octreeObject, 
          [](std::string filepath) -> std::string {
            auto modpath = modlayerPath(filepath);
            return readFileOrPackage(modpath);
          }, 
          createScopedLoadMesh(world, selectedIndex)
        );
        updatePhysicsBody(world, selectedIndex);
      }
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '3', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      for (auto &selectedIndex : state.editor.selectedObjs){
        GameObjectOctree* octreeObject = getOctree(world.objectMapping, selectedIndex);
        modassert(octreeObject, "octree object null");
        saveOctree(*octreeObject, [](std::string filepath, std::string& data) -> void {
          auto modpath = modlayerPath(filepath);
          realfiles::saveFile(modpath, data);
        });
      }
    }
  },

  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '1', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      for (auto &selectedIndex : state.editor.selectedObjs){
        GameObjectOctree* octreeObject = getOctree(world.objectMapping, selectedIndex);
        modassert(octreeObject, "octree object null");
        makeOctreeCellRamp(*octreeObject, octreeObject -> octree, createScopedLoadMesh(world, selectedIndex), state.rampDirection);
        if (state.rebuildOctreePhysicsOnEdit){
          updatePhysicsBody(world, selectedIndex);
        }
      }
    }
  },

  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '[', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.rampDirection = (state.rampDirection ==  RAMP_FORWARD) ? RAMP_BACKWARD : RAMP_FORWARD;
      for (auto &selectedIndex : state.editor.selectedObjs){
        setTexture(selectedIndex, "./res/texturevideotexture");
      }
    }
  },
  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = ']', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      state.rampDirection = (state.rampDirection ==  RAMP_LEFT) ? RAMP_RIGHT : RAMP_LEFT;
    }
  },

  /////////// end octree stuff

  InputDispatch{
    .alwaysEnable = false,
    .sourceKey = '\'', 
    .sourceType = BUTTON_PRESS,
    .prereqKey = 0, 
    .hasPreq = false,
    .fn = []() -> void {
      setWorldState({
        ObjectValue {
          .object = "physics",
          .attribute = "debug",
          .value = state.enablePhysicsDebug ? "false" : "true",
        }
      });
    }
  },
};

    
    
    
    

//int getCurrentSubdivisionLevel(){
//  return subdivisionLevel;
//}
//void handleChangeSubdivisionLevel(int newSubdivisionLevel){