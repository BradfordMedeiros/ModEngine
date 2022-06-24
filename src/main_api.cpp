#include "./main_api.h"

extern World world;
extern RenderStages renderStages;
extern SysInterface interface;
extern WorldTiming timings;
extern engineState state;
extern GameObject defaultCamera;
extern std::map<unsigned int, Mesh> fontMeshes;
extern Mesh* crosshairSprite;
extern Mesh defaultCrosshairSprite;
extern unsigned int uiShaderProgram;
extern float initialTime;
extern std::queue<StringString> channelMessages;

extern float now;
extern float deltaTime;

extern std::string rawSceneFile;
extern bool bootStrapperMode;
extern NetCode netcode;
extern DrawingParams drawParams;
extern DynamicLoading dynamicLoading;
extern std::map<std::string, objid> activeLocks;
extern CScriptBindingCallbacks cBindings;
extern LineData lineData;

std::optional<objid> getGameObjectByName(std::string name, objid sceneId, bool sceneIdExplicit){    // @todo : odd behavior: currently these names do not have to be unique in different scenes.  this just finds first instance of that name.
  return getGameObjectByNamePrefix(world, name, sceneId, sceneIdExplicit);
}


btRigidBody* getRigidBody(int32_t index){
  return world.rigidbodys.find(index) == world.rigidbodys.end() ? NULL : world.rigidbodys.at(index).body;
}
void applyImpulse(int32_t index, glm::vec3 impulse){
  auto rigidBody = getRigidBody(index);
  if (rigidBody != NULL){
    applyImpulse(rigidBody, impulse);
  }
}
void applyImpulseRel(int32_t index, glm::vec3 impulse){
  auto rigidBody = getRigidBody(index);
  if (rigidBody != NULL){
    glm::vec3 relativeImpulse = calculateRelativeOffset(getGameObject(world, index).transformation.rotation, impulse, true);
    applyImpulse(rigidBody, relativeImpulse);
  }
}

void clearImpulse(int32_t index){
  auto rigidBody = getRigidBody(index);
  if (rigidBody != NULL){
    clearImpulse(rigidBody);
  }
}

std::vector<std::string> listSceneFiles(){
  return listFilesWithExtensions("./res/scenes", { "rawscene" });
}

bool parentScene(objid sceneId, objid* _parentSceneId){
  return parentSceneId(world.sandbox, sceneId, _parentSceneId);
}

std::vector<objid> childScenes(objid sceneId){
  return childSceneIds(world.sandbox, sceneId);
}

int32_t loadScene(std::string sceneFile, std::vector<std::vector<std::string>> additionalTokens, std::optional<std::string> name){
  std::cout << "INFO: SCENE LOADING: loading " << sceneFile << std::endl;
  std::vector<Token> addedTokens;
  for (auto &tokens : additionalTokens){
    addedTokens.push_back(Token{
      .target = tokens.at(0),
      .attribute = tokens.at(1),
      .payload = tokens.at(2),
    });
  }
  return addSceneToWorld(world, sceneFile, interface, addedTokens, name);
}
int32_t loadSceneParentOffset(std::string sceneFile, glm::vec3 offset, std::string parentNodeName){
  auto name = std::to_string(getUniqueObjId()) + parentNodeName;

  auto nodeOffsetId = makeObjectAttr(world.sandbox.mainScene.rootId, name, {}, {}, vectorAttributes { .vec3 = {{"position", offset}}, .vec4 = {} } );
  std::cout << "load scene offset: " << print(offset) << std::endl;
  auto sceneId = loadScene(sceneFile, {}, std::nullopt);
  auto rootId = rootIdForScene(world.sandbox, sceneId);
  makeParent(world.sandbox, rootId, nodeOffsetId);
  return nodeOffsetId;
}

std::optional<objid> sceneIdByName(std::string name){
  return sceneIdByName(world.sandbox, name);
}

objid rootIdForScene(objid sceneId){
  return rootIdForScene(world.sandbox, sceneId);
}

void unloadScene(int32_t sceneId){  
  std::cout << "INFO: SCENE LOADING: unloading " << sceneId << std::endl;
  removeSceneFromWorld(world, sceneId, interface);
}
void unloadAllScenes(){
  removeAllScenesFromWorld(world, interface);
}

// @TODO - save all the scenes in the world
void saveScene(bool includeIds, objid sceneId){
  auto fileToSave = sceneFileForSceneId(world, sceneId);    // MAYBE SHOULD CREATE A CACHE OF WHAT FILE WAS WHAT SCENE?
  std::cout << "saving scene id: " << sceneId << " to file: " << fileToSave << std::endl;
  auto fileExtension = getPreExtension(fileToSave);

  auto allowedToSave = true;
  if (fileExtension.has_value()){
    auto extension = fileExtension.value();
    std::cout << "extension is: " << extension << std::endl;
    allowedToSave = extension != "p";
  }
  if (allowedToSave){
    saveFile(fileToSave, serializeScene(world, sceneId, includeIds));
  }else{
    std::cout << "WARNING: CANNOT SAVE: " << fileToSave << " because is a protected file" << std::endl;
  }

}

std::vector<int32_t> listScenes(){
  return allSceneIds(world.sandbox);
}

std::vector<StringPairVec2> scenegraph(){
  std::vector<StringPairVec2> parentToChild;
  auto dotRelations = getDotRelations(world.sandbox, world.objectMapping);
  for (auto &dotRelation : dotRelations){
    if (dotRelation.parent.has_value()){
      parentToChild.push_back(StringPairVec2{
        .key = dotRelation.parent.value().name,
        .value = dotRelation.child.name,
        .vec = glm::ivec2(dotRelation.parent.value().sceneId, dotRelation.child.sceneId),
      });
    }
  }

  std::sort(parentToChild.begin(), parentToChild.end(), [](StringPairVec2 one, StringPairVec2 two) {
    int value = one.key.compare(two.key);
    if (value != 0){
      return value < 0;
    }
    return one.value.compare(two.value) < 0;
  });
  return parentToChild;
}

void sendLoadScene(int32_t id){
  sendLoadScene(world, netcode, bootStrapperMode, id);
}

void createScene(std::string scenename){
  auto extension = getExtension(scenename);
  bool canSave = extension.has_value() && extension == "rawscene";
  assert(canSave);
  saveFile(scenename, "");
}


std::vector<int32_t> getObjectsByType(std::string type){
  if (type == "mesh"){
    std::vector indexes = getGameObjectsIndex<GameObjectMesh>(world.objectMapping);
    return indexes;
  }else if (type == "camera"){
    std::vector indexes = getGameObjectsIndex<GameObjectCamera>(world.objectMapping);
    return indexes;
  }
  return getGameObjectsIndex(world.objectMapping);
}
std::vector<int32_t> getObjectsByAttr(std::string type, std::optional<AttributeValue> value, int32_t sceneId){
  auto objIds = listObjInScene(world.sandbox, sceneId);
  std::vector<objid> idsWithAttrs;
  for (auto id : objIds){
    if (objectHasAttribute(world, id, type, value)){
      idsWithAttrs.push_back(id);
    }
  }
  return idsWithAttrs;
}

std::string getGameObjectName(int32_t index){
  return getGameObject(world, index).name;
}

GameobjAttributes getGameObjectAttr(int32_t id){
  return objectAttributes(world, id);
}

void setGameObjectAttr(int32_t id, GameobjAttributes& attr){
  setAttributes(world, id, attr);
}

glm::vec3 getGameObjectPosition(int32_t index, bool isWorld){
  if (isWorld){
    return fullTransformation(world.sandbox, index).position;
  }
  return getGameObject(world, index).transformation.position;
}
glm::vec3 getGameObjectPos(int32_t index){
  return getGameObjectPosition(index, true);
}
void setGameObjectPosition(int32_t index, glm::vec3 pos){ // sets the absolutePosition
  physicsTranslateSet(world, index, pos, false);
}
void setGameObjectPositionRelative(int32_t index, glm::vec3 pos){
  physicsTranslateSet(world, index, pos, true);
}
glm::vec3 getGameObjectScale(int32_t index){
  return getGameObject(world, index).transformation.scale;
}
void setGameObjectScale(int32_t index, glm::vec3 scale){
  physicsScaleSet(world, index, scale);
}
void setGameObjectRotation(int32_t index, glm::quat rotation){
  physicsRotateSet(world, index, rotation, false);
}

void setGameObjectRotationRelative(int32_t index, glm::quat rotation){
  physicsRotateSet(world, index, rotation, true);
}
glm::quat getGameObjectRotation(int32_t index, bool isWorld){
  if (isWorld){
    return fullTransformation(world.sandbox, index).rotation;
  }
  return getGameObject(world, index).transformation.rotation;
}
glm::quat getGameObjectRotationRelative(int32_t index){
  return getGameObjectRotation(index, false);
}

objid makeObjectAttr(objid sceneId, std::string name, std::map<std::string, std::string> stringAttributes, std::map<std::string, double> numAttributes, vectorAttributes vecAttr){
  GameobjAttributes attributes {
    .stringAttributes = stringAttributes,
    .numAttributes = numAttributes,
    .vecAttr = vecAttr,
  };
  return addObjectToScene(world, sceneId, name, attributes, interface);
}

void copyObject(int32_t id){
  copyObjectToScene(world, id, interface);
}

void drawText(std::string word, float left, float top, unsigned int fontSize, bool permatext, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId){
  //std::cout << "draw text: " << word << ": perma? " << permatext << std::endl;
  addTextData(lineData, TextDrawingOptions{
    .word = word,
    .left = left,
    .top = top,
    .fontSize = fontSize,
    .textureId = textureId,
    .permaText = permatext,
    .tint = tint.has_value() ? tint.value() : glm::vec4(1.f, 1.f, 1.f, 1.f),
  });
}
void drawText(std::string word, float left, float top, unsigned int fontSize){
  drawText(word, left, top, fontSize, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);  
}

int drawWord(GLint shaderProgram, objid id, std::string word, unsigned int fontSize, float offsetDelta, AlignType align, TextWrap wrap, TextVirtualization virtualization){
  return drawWordsRelative(shaderProgram, fontMeshes, fullModelTransform(world.sandbox, id), word, 0, 0, fontSize, offsetDelta, align, wrap, virtualization);
}

std::vector<std::string> listAnimations(int32_t id){
  std::vector<std::string> animationNames;
  auto groupId = getGroupId(world.sandbox, id);
  if (world.animations.find(groupId) == world.animations.end()){
    return animationNames;
  }
  auto animations = world.animations.at(groupId);
  for (auto animation : animations){
    animationNames.push_back(animation.name);
  }
  return animationNames;
}

void playAnimation(int32_t id, std::string animationToPlay){
  addAnimation(world, timings, id, animationToPlay);
}

void stopAnimation(int32_t id){
  removeAnimation(world, timings, id);
}

void removeObjectById(objid id){
  removeObjectFromScene(world, id, interface);
}

std::vector<std::string> listModels(){
  return listFilesWithExtensions("./res/models", { "obj", "dae" });
}

void sendNotifyMessage(std::string message, std::string value){
  channelMessages.push(StringString {
    .strTopic = message,
    .strValue = value,
  });
}

double timeSeconds(){
  return now;
}

double timeElapsed(){
  return deltaTime;
}

struct ActiveRecording {
  objid targetObj;
  Recording recording;
};

std::map<objid, ActiveRecording> activeRecordings;
objid createRecording(objid id){  
  auto recordingId = getUniqueObjId();
  assert(activeRecordings.find(recordingId) == activeRecordings.end());
  activeRecordings[recordingId] = ActiveRecording{
    .targetObj = id,
    .recording = createRecording(),
  };
  return recordingId;
}
void saveRecording(objid recordingId, std::string filepath){
  auto recording = activeRecordings.at(recordingId);
  std::cout << "SAVING RECORDING STARTED - " << filepath << std::endl;
  saveRecording(filepath, recording.recording, serializePropertySuffix);
  activeRecordings.erase(recordingId);
  std::cout << "SAVING RECORDING COMPLETE - " << filepath << std::endl;
}

std::map<objid, Recording> playingRecordings;
void playRecording(objid id, std::string recordingPath){
  assert(playingRecordings.find(id) == playingRecordings.end());
  playingRecordings[id] = loadRecording(recordingPath, parsePropertySuffix);
}
void stopRecording(objid id, std::string recordingPath){
  assert(playingRecordings.find(id) != playingRecordings.end());
  playingRecordings.erase(id);
}

void tickRecordings(float time){
  for (auto &[id, activeRecording] : activeRecordings){
    auto gameobject = getGameObject(world, activeRecording.targetObj);
    saveRecordingIndex(activeRecording.recording, "position", gameobject.transformation.position, time);
  } 

  for (auto &[id, recording] : playingRecordings){
    auto interpolatedProperties = recordingPropertiesInterpolated(recording, time, interpolateAttribute);
    setProperty(world, id, interpolatedProperties);
  }
}


std::vector<HitObject> raycastW(glm::vec3 pos, glm::quat direction, float maxDistance){
  return raycast(world, pos, direction, maxDistance);
}

glm::vec3 moveRelative(glm::vec3 posFrom, glm::quat orientation, float distance){
  return moveRelative(posFrom, orientation, glm::vec3(0.f, 0.f, -1 * distance), false);
}
glm::vec3 moveRelative(glm::vec3 posFrom, glm::quat orientation, glm::vec3 vec){
  return moveRelative(posFrom, orientation, vec, false);
}

void nextTexture(){
  nextTexture(drawParams, world.textures.size());
}
void previousTexture(){
  previousTexture(drawParams);
}

void setTexture(objid index, std::string textureName){
  setTexture(world, index, textureName);
}
void maybeChangeTexture(int index){
  auto textureName = worldTextures(world).at(drawParams.activeTextureIndex).textureName;
  setTexture(world, index, textureName);
}

void setState(std::string stateName){
  if (stateName == "paint_on"){
    state.shouldPaint = true;
  }else if (stateName == "paint_off"){
    state.shouldPaint = false;
  }else if (stateName == "highlight_on"){
    state.highlight = true;
  }else if (stateName == "highlight_off"){
    state.highlight = false;
  }else if (stateName == "translate"){
    state.manipulatorMode = TRANSLATE;
  }else if (stateName == "scale"){
    state.manipulatorMode = SCALE;
  }else if (stateName == "rotate"){
    state.manipulatorMode = ROTATE;  
  }else if (stateName == "next_texture"){
    nextTexture();
  }else if (stateName == "prev_texture"){
    previousTexture();
  }
}

void  setCrosshairSprite(){
  if (state.crosshair == ""){
    crosshairSprite = &world.meshes.at("./res/textures/crosshairs/crosshair008.png").mesh;
    return;
  }
  crosshairSprite = &world.meshes.at(state.crosshair).mesh;
}

void setWorldState(std::vector<ObjectValue> values){
  std::vector<ObjectValue> renderStagesValues;
  std::vector<ObjectValue> otherValues;
  for (auto &value: values){
    bool isRenderValue = value.object.at(0) == '$';
    if (isRenderValue){
      auto translatedObject = value.object.substr(1, value.object.size());
      value.object = translatedObject;
      renderStagesValues.push_back(value);
    }else{
      otherValues.push_back(value);
    }
  }

  for (auto &renderStagesValue : renderStagesValues){
    setRenderStageState(renderStages, renderStagesValue);
  }
  setState(state, otherValues, now);

  //// todo add updates for each state ... should check if it was changed in state maybe?
  setCrosshairSprite();
}

void setLayerState(std::vector<StrValues> values){
  setLayerOptions(world.sandbox.layers, values);
}

void setFloatState(std::string stateName, float value){
  if (stateName == "opacity"){
    drawParams.opacity = value;
  }
  else if (stateName == "drawsize"){
    drawParams.scale = glm::vec3(1.f, 1.f, 1.f) * value;
  }
  else if (stateName == "drawcolor-r"){
    drawParams.tint.x = value;
  }
  else if (stateName == "drawcolor-g"){
    drawParams.tint.y = value;
  }
  else if (stateName == "drawcolor-b"){
    drawParams.tint.z = value;
  }
}

void setIntState(std::string stateName, int value){
  if (stateName == "set_texture"){
    std::cout << "value is: " << value << std::endl;
    maybeChangeTexture(value);
  }  
}

objid listSceneId(int32_t id){
  return sceneId(world.sandbox, id);
}

Transformation getCameraTransform(){
  if (state.useDefaultCamera || state.activeCameraObj == NULL){
    return defaultCamera.transformation;
  }
  if (state.cameraInterp.shouldInterpolate){
    auto lerpAmount = (now - state.cameraInterp.startingTime) / state.cameraInterp.length;
    if (lerpAmount >= 1){
      state.cameraInterp.shouldInterpolate = false;
      state.activeCameraObj = &getGameObject(world, state.cameraInterp.targetCam);
      state.activeCameraData = &getCamera(world, state.activeCameraObj -> id);
      cBindings.onCameraSystemChange(state.activeCameraObj -> name, state.useDefaultCamera);
    }
    auto oldCameraPosition = fullTransformation(world.sandbox, state.activeCameraObj -> id);
    auto newCameraPosition = fullTransformation(world.sandbox, state.cameraInterp.targetCam);
    return interpolate(oldCameraPosition, newCameraPosition, lerpAmount, lerpAmount, lerpAmount);
  }
  return fullTransformation(world.sandbox, state.activeCameraObj -> id);
}
void maybeResetCamera(int32_t id){
  if (state.activeCameraObj != NULL &&  id == state.activeCameraObj -> id){
    state.activeCameraObj = NULL;
    state.activeCameraData = NULL;
    std::cout << "active camera reset" << std::endl;
  }
  if (state.cameraInterp.targetCam == id){
    state.cameraInterp.shouldInterpolate = false;
  }
}
void setActiveCamera(int32_t cameraId, float interpolationTime){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(world.objectMapping);
  if (! (std::find(cameraIndexs.begin(), cameraIndexs.end(), cameraId) != cameraIndexs.end())){
    std::cout << "index: " << cameraId << " is not a valid index" << std::endl;
    return;
  }

  if (interpolationTime > 0){
    state.cameraInterp = CamInterpolation {
      .shouldInterpolate = true,
      .startingTime = now,
      .length = interpolationTime,
      .targetCam = cameraId,
    };
    return;
  }

  state.activeCameraObj = &getGameObject(world, cameraId);
  state.activeCameraData = &getCamera(world, cameraId);
  setSelectedIndex(state.editor, cameraId, state.activeCameraObj -> name, true);
  cBindings.onCameraSystemChange(state.activeCameraObj -> name, state.useDefaultCamera);
  std::cout << "set active camera to id: " << cameraId << std::endl;
  std::cout << "camera data: " << state.activeCameraData -> enableDof << ", " << state.activeCameraData -> minBlurDistance << ", " << state.activeCameraData -> maxBlurDistance << std::endl;
}
void setActiveCamera(std::string name, objid sceneId){
  auto object = getGameObjectByName(name, sceneId, false);
  if (!object.has_value()){
    std::cout << "ERROR SETTING CAMERA: does the camera: " << name << " exist?" << std::endl;
    assert(false);
  }
  setActiveCamera(object.value(), -1);
}

void nextCamera(){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(world.objectMapping);
  if (cameraIndexs.size() == 0){  // if we do not have a camera in the scene, we use default
    state.activeCameraObj = NULL;
    state.activeCameraData = NULL;
    return;
  }

  state.activeCamera = (state.activeCamera + 1) % cameraIndexs.size();
  int32_t activeCameraId = cameraIndexs.at(state.activeCamera);
  setActiveCamera(activeCameraId, -1);
}
void moveCamera(glm::vec3 offset){
  if (state.activeCameraObj == NULL){
    defaultCamera.transformation.position = moveRelative(defaultCamera.transformation.position, defaultCamera.transformation.rotation, glm::vec3(offset), false);
  }else{
    setGameObjectPosition(state.activeCameraObj ->id, moveRelative(state.activeCameraObj -> transformation.position, state.activeCameraObj -> transformation.rotation, glm::vec3(offset), false));
  }
}
void rotateCamera(float xoffset, float yoffset){
  if (state.activeCameraObj == NULL){
    defaultCamera.transformation.rotation = setFrontDelta(defaultCamera.transformation.rotation, xoffset, yoffset, 0, 0.1);
  }else{
    setGameObjectRotation(state.activeCameraObj ->id, setFrontDelta(state.activeCameraObj -> transformation.rotation, xoffset, yoffset, 0, 0.1));
  }
}
void setCameraRotation(glm::quat orientation){
  if (state.activeCameraObj == NULL){
    defaultCamera.transformation.rotation = orientation;
  }else{
    setGameObjectRotation(state.activeCameraObj ->id, orientation);
  }
}

void playSoundState(std::string source, objid sceneId){
  std::cout << "Info: play sound: " << source << std::endl;
  auto gameobj = getGameObjectByName(source, sceneId, false);
  if (gameobj.has_value()){
    playSoundState(world.objectMapping, gameobj.value()); 
  }else{
    std::cout << "ERROR: no source named: " << source << " in scene: " << sceneId << std::endl;
    assert(false);
  }
}

unsigned int activeTextureId(){
  return world.textures.at(activeTextureName(drawParams, world.textures)).texture.textureId;
}

void emit(objid id, std::optional<glm::vec3> initPosition, std::optional<glm::quat> initOrientation, std::optional<glm::vec3> initVelocity, std::optional<glm::vec3> initAvelocity){
  emit(world, id, NewParticleOptions {
    .position = initPosition,
    .orientation = initOrientation,
    .velocity = initVelocity,
    .angularVelocity = initAvelocity, 
  });
}

objid addLoadingAround(objid id){
  return addLoadingAround(dynamicLoading, id);
}
void removeLoadingAround(objid id){
  removeLoadingAround(dynamicLoading, id);
}

void makeParent(objid child, objid parent){
  makeParent(world.sandbox, child, parent);
}

void createGeneratedMesh(std::vector<glm::vec3> face, std::vector<glm::vec3> points, std::string destMesh){
  createGeneratedMesh(world, face, points, destMesh);
}

glm::vec3 navPosition(objid id, glm::vec3 target){
  return aiNavigate(world, id, target);
}

std::vector<VoxelQueryData> getSelectedVoxels(){
  std::vector<VoxelQueryData> voxels;
  for (auto id : selectedIds(state.editor)){
    auto maybeVoxel = getVoxel(world, id);
    if (maybeVoxel.has_value()){
      voxels.push_back(VoxelQueryData{
        .index = id,
        .voxelPtr = maybeVoxel.value(),
      });
    }
  }
  return voxels;
}

bool lock(std::string key, objid owner){
  std::cout << "lock: (" << key << ", " << owner << ")" << std::endl;
  auto canLock = activeLocks.find(key) == activeLocks.end();
  if (!canLock){
    return false;
  }
  activeLocks[key] = owner;
  std::cout << "lock: " << key << std::endl;
  return true;
}
bool unlock(std::string key, objid owner){
  std::cout << "unlock: (" << key << ", " << owner << ")" << std::endl;
  auto lockExists = activeLocks.find(key) != activeLocks.end();
  auto ownerOwnsKey = lockExists && activeLocks.at(key) == owner;
  if (!lockExists){
    return true;
  }  
  if (ownerOwnsKey){
    activeLocks.erase(key);
    std::cout << "unlock: " << key << std::endl;
    return true;
  }
  std::cout << "ERROR: tried to unlock a key that did not own (" << key << "," << owner << ")" << std::endl;
  return false;
}

void removeLocks(objid owner){
  std::vector<std::string> locksOwnedByOwner;
  for (auto &[lockname, id] : activeLocks){
    if (id == owner){
      locksOwnedByOwner.push_back(lockname);
    }
  }
  for (auto lockname : locksOwnedByOwner){
    activeLocks.erase(lockname);
  }
}

void enforceLayout(objid layoutId){
  enforceLayout(world, layoutId);
}

void takeScreenshot(std::string filepath){
  state.takeScreenshot = true;
  state.screenshotPath = filepath;
}


std::vector<UserTexture> userTextures;
std::vector<UserTexture> textureIdsToRender(){
  return userTextures;
}

UserTexture* userTextureById(unsigned int id){
  for (auto &userTexture : userTextures){
    if (userTexture.id == id){
      return &userTexture;
    }
  }
  assert(false);
  return NULL;
}
unsigned int createTexture(std::string name, unsigned int width, unsigned int height, objid ownerId){
  MODTODO("create texture -> use ownership id of the script being used");
  auto textureID = loadTextureWorldEmpty(world, name, ownerId, width, height).textureId;
  userTextures.push_back(UserTexture{
    .id = textureID,
    .autoclear = false,
    .shouldClear = true,
    .clearTextureId = std::nullopt, //world.textures.at("./res/textures/wood.jpg").texture.textureId,//,
    .clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
  });
  return textureID;
}

void freeTexture(std::string name, objid ownerId){
  MODTODO("delete texture -> use ownership id of the script being used");
  auto textureId = world.textures.at(name).texture.textureId;

  std::vector<UserTexture> remainingTextures;
  for (auto userTexture : userTextures){
    if (userTexture.id != textureId){
      remainingTextures.push_back(userTexture);
    }
  }
  userTextures = remainingTextures;
  freeTextureRefsIdByOwner(world, ownerId, textureId);
}

void clearTexture(unsigned int textureId, std::optional<bool> autoclear, std::optional<glm::vec4> color, std::optional<std::string> texture){
  UserTexture& userTex = *userTextureById(textureId);
  std::optional<unsigned int> clearTextureId = std::nullopt;
  if (texture.has_value()){
    std::cout << "texture: " << texture.value() << std::endl;
    clearTextureId = world.textures.at(texture.value()).texture.textureId;
  }
  userTex.clearTextureId = clearTextureId;
  
  if (color.has_value()){
    userTex.clearColor = color.value();
  }
  if (!autoclear.has_value()){
    userTex.shouldClear = true;
    return;
  }
  userTex.autoclear = autoclear.value();
}

void markUserTexturesCleared(){
  for (auto &userTexture : userTextures){
    userTexture.shouldClear = false;
  }
}