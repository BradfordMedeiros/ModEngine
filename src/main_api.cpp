#include "./main_api.h"

extern World world;
extern SysInterface interface;
extern AnimationState animations;
extern GameObject* activeCameraObj;
extern engineState state;
extern GameObject defaultCamera;
extern std::map<unsigned int, Mesh> fontMeshes;
extern unsigned int uiShaderProgram;
extern float initialTime;
extern std::vector<int32_t> playbacksToRemove;
extern std::queue<StringString> channelMessages;

extern float now;
extern std::string rawSceneFile;
extern bool bootStrapperMode;
extern NetCode netcode;
extern DrawingParams drawParams;
extern DynamicLoading dynamicLoading;


NetworkPacket toNetworkPacket(UdpPacket& packet){
  NetworkPacket netpacket {
    .packet = &packet,
    .packetSize = sizeof(packet),
  };
  return netpacket;
}

std::optional<objid> getGameObjectByName(std::string name, objid sceneId){    // @todo : odd behavior: currently these names do not have to be unique in different scenes.  this just finds first instance of that name.
  return getGameObjectByName(world, name, sceneId);
}

void applyImpulse(int32_t index, glm::vec3 impulse){
  applyImpulse(world.rigidbodys.at(index), impulse);
}
void applyImpulseRel(int32_t index, glm::vec3 impulse){
  glm::vec3 relativeImpulse = calculateRelativeOffset(getGameObject(world, index).transformation.rotation, impulse, true);
  applyImpulse(world.rigidbodys.at(index), relativeImpulse);
}

void clearImpulse(int32_t index){
  clearImpulse(world.rigidbodys.at(index));
}

void loadScriptFromWorld(std::string script, int32_t id){
  auto name = getGameObject(world, id).name;
  std::cout << "gameobj: " << name << " wants to load script: (" << script << ")" << std::endl;
  loadScript(script, id, bootStrapperMode);
}
int32_t loadScene(std::string sceneFile){
  std::cout << "INFO: SCENE LOADING: loading " << sceneFile << std::endl;
  return addSceneToWorld(world, sceneFile, interface);
}
int32_t loadSceneParentOffset(std::string sceneFile, glm::vec3 offset, std::string parentNodeName){
  auto name = std::to_string(getUniqueObjId()) + parentNodeName;

  auto nodeOffsetId = makeObjectAttr(world.sandbox.mainScene.rootId, name, {}, {}, {{"position", offset}});
  std::cout << "load scene offset: " << print(offset) << std::endl;
  auto sceneId = loadScene(sceneFile);
  auto rootId = rootIdForScene(world.sandbox, sceneId);
  makeParent(world.sandbox, rootId, nodeOffsetId);
  return nodeOffsetId;
}

int32_t loadSceneData(std::string sceneData, objid sceneId){
  std::cout << "INFO: SCENE LOADING: loading from scene data" << std::endl;
  return addSceneToWorldFromData(world, sceneId, sceneData, interface);
}

void unloadScene(int32_t sceneId){  
  std::cout << "INFO: SCENE LOADING: unloading " << sceneId << std::endl;
  removeSceneFromWorld(world, sceneId, interface);
}
void unloadAllScenes(){
  removeAllScenesFromWorld(world, interface);
}

// @TODO - save all the scenes in the world
void saveScene(bool includeIds){
  auto sceneIds = allSceneIds(world.sandbox);
  auto id = sceneIds.at(0);
  auto fileToSave = rawSceneFile;
  std::cout << "saving scene id: " << id << " to file: " << fileToSave << std::endl;
  saveFile(fileToSave, serializeScene(world, id, includeIds));
}

std::vector<int32_t> listScenes(){
  return allSceneIds(world.sandbox);
}

void sendLoadScene(int32_t id){
  if (!bootStrapperMode){
    std::cout << "ERROR: cannot send load scene in not-server mode" << std::endl;
    assert(false);
  }

  std::string sceneData = serializeScene(world, id, true);
  UdpPacket packet { .type = LOAD };
  auto data = sceneData.c_str();
  LoadPacket loadpacket {
    .sceneId = id,
  };
  assert((sizeof(data) + 1 ) < sizeof(loadpacket.sceneData));
  strncpy(loadpacket.sceneData, data, sizeof(loadpacket.sceneData));
  assert(loadpacket.sceneData[sizeof(loadpacket.sceneData) -1] == '\0');
  packet.payload.loadpacket = loadpacket; 
  sendUdpPacketToAllUdpClients(netcode, toNetworkPacket(packet));
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
std::string getGameObjectName(int32_t index){
  return getGameObject(world, index).name;
}

std::map<std::string, std::string> getGameObjectAttr(int32_t id){
  return getAttributes(world, id);
}
void setGameObjectAttr(int32_t id, std::map<std::string, std::string> attr){
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
void setGameObjectPosition(int32_t index, glm::vec3 pos){
  physicsTranslateSet(world, index, pos);
}
void setGameObjectPositionRelative(int32_t index, float x, float y, float z, bool xzPlaneOnly){
  auto transformation = getGameObject(world, index).transformation;
  glm::vec3 pos = moveRelative(transformation.position, transformation.rotation, glm::vec3(x, y, z), xzPlaneOnly);
  physicsTranslateSet(world, index, pos);
}
glm::vec3 getGameObjectScale(int32_t index){
  return getGameObject(world, index).transformation.scale;
}
void setGameObjectScale(int32_t index, glm::vec3 scale){
  physicsScaleSet(world, index, scale);
}
void setGameObjectRotation(int32_t index, glm::quat rotation){
  physicsRotateSet(world, index, rotation);
}
glm::quat getGameObjectRotation(int32_t index, bool isWorld){
  if (isWorld){
    return fullTransformation(world.sandbox, index).rotation;
  }
  return getGameObject(world, index).transformation.rotation;
}

void setSelectionMode(bool enabled){
  // todo allow toggle for this 
}

int32_t makeObject(std::string serializedobj, objid id, bool useObjId, objid sceneId, bool useSceneId){
  auto firstSceneId = allSceneIds(world.sandbox).at(0);
  return addObjectToScene(world, useSceneId ? sceneId : firstSceneId, serializedobj, id, useObjId, interface);
}
objid makeObjectAttr(objid sceneId, std::string name, std::map<std::string, std::string> stringAttributes, std::map<std::string, double> numAttributes, std::map<std::string, glm::vec3> vecAttributes){
  GameobjAttributes attributes {
    .stringAttributes = stringAttributes,
    .numAttributes = numAttributes,
    .vecAttributes = vecAttributes,
    .additionalFields = stringAttributes,
  };
  return addObjectToScene(world, sceneId, name, attributes, interface);
}

void copyObject(int32_t id){
  copyObjectToScene(world, id, interface);
}

void drawText(std::string word, float left, float top, unsigned int fontSize){
  drawWords(uiShaderProgram, fontMeshes, word, left, top, fontSize);  
}

void drawWord(GLint shaderProgram, objid id, std::string word, unsigned int fontSize, float offsetDelta){
  drawWordsRelative(shaderProgram, fontMeshes, fullModelTransform(world.sandbox, id), word, 0, 0, fontSize, true, offsetDelta);
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
Animation getAnimation(World& world, int32_t groupId, std::string animationToPlay){  
  Animation noAnimation { };
  for (auto animation :  world.animations.at(groupId)){
    if (animation.name == animationToPlay){
      return animation;
    }
  }
  std::cout << "no animation found" << std::endl;
  assert(false);
  return  noAnimation;  // @todo probably use optional here.
}

void addAnimation(AnimationState& animationState, int32_t groupId, objid sceneId, std::string animationToPlay){
  auto animation = getAnimation(world, groupId, animationToPlay);
  TimePlayback playback(
    initialTime, 
    [animation, groupId, sceneId](float currentTime, float elapsedTime) -> void { 
      auto meshNameToMeshes = getMeshesForGroupId(world, groupId);  
      playbackAnimation(animation, meshNameToMeshes, currentTime, elapsedTime,
        [sceneId](std::string name, std::string skeletonRoot) -> glm::mat4 {
          auto gameobj =  maybeGetGameObjectByName(world.sandbox, name, sceneId);
          if (gameobj.has_value()){
            return armatureTransform(world.sandbox, gameobj.value() -> id, skeletonRoot, sceneId);
          }
          std::cout << "no value: " << name << std::endl;
          assert(false);
          return glm::mat4(1.f);  
        },
        [&world, sceneId](std::string name, glm::mat4 pose) -> void {
          auto gameobj =  maybeGetGameObjectByName(world.sandbox, name, sceneId);
          if (gameobj.has_value()){
            gameobj.value() -> transformation = getTransformationFromMatrix(pose);
          }else{
            std::cout << "warning no bone node named: " << name << std::endl;
            assert(false);
          }
      });
    }, 
    [groupId, &animationState]() -> void { 
      playbacksToRemove.push_back(groupId);
    },
    animation.duration,
    PAUSE
  );  
  animationState.playbacks[groupId] = playback;
}

void playAnimation(int32_t id, std::string animationToPlay){
  auto groupId = getGroupId(world.sandbox, id);
  auto idForScene = sceneId(world.sandbox, id);
  if (animations.playbacks.find(groupId) != animations.playbacks.end()){
    animations.playbacks.erase(groupId);
  }
  addAnimation(animations, groupId, idForScene, animationToPlay);
}

void stopAnimation(int32_t id){
  auto groupId = getGroupId(world.sandbox, id);
  playbacksToRemove.push_back(groupId);
}

void removeObjectById(objid id){
  removeObjectFromScene(world, id, interface);
}

std::vector<std::string> listModels(){
  return listFilesWithExtensions("./res/models", { "obj", "dae" });
}

void sendEventMessage(std::string message){
  auto channelMapping = getChannelMapping(world.objectMapping);
  if (channelMapping.find(message) != channelMapping.end()){
    for (auto to : channelMapping.at(message)){
      std::cout << "SYSTEM INFO: channel: (" << message << ", " << to << ")" << std::endl;
      channelMessages.push(StringString {
        .strTopic = to,
        .strValue = "",
      });
    }
  }
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

void sendDataUdp(std::string data){
  UdpPacket packet {
    .type = CREATE,
  };
  sendDataOnUdpSocket(toNetworkPacket(packet));
}

void copyStr(std::string& data, char* copyTo, int size){
  auto strdata = data.c_str();
  assert((sizeof(strdata) + 1 ) < size);
  strncpy(copyTo, strdata, size);
  assert(copyTo[size -1] == '\0');
}

std::string connectServer(std::string data){
  UdpPacket setup = {
    .type = SETUP,
  };  

  SetupPacket setupPacket {};
  auto packet = toNetworkPacket(setup);

  return connectServer(data, [&setup, &setupPacket, &packet](std::string connectionHash) -> NetworkPacket {
    auto data = connectionHash.c_str();
    assert((sizeof(data) + 1 ) < sizeof(setupPacket.connectionHash));
    strncpy(setupPacket.connectionHash, data, sizeof(setupPacket.connectionHash));
    assert(setupPacket.connectionHash[sizeof(setupPacket.connectionHash) -1] == '\0');
    setup.payload.setuppacket = setupPacket;
    return packet;
  });
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
    auto gameobj = getGameObject(world, id);
    auto interpolatedProperties = recordingPropertiesInterpolated(recording, time, interpolateAttribute);
    setProperty(world, gameobj.id, interpolatedProperties);
  }
}


std::vector<HitObject> raycastW(glm::vec3 pos, glm::quat direction, float maxDistance){
  return raycast(world, pos, direction, maxDistance);
}

glm::vec3 moveRelative(glm::vec3 posFrom, glm::quat orientation, float distance){
  return moveRelative(posFrom, orientation, glm::vec3(0.f, 0.f, -1 * distance), false);
}

void nextTexture(){
  nextTexture(drawParams, world.textures.size());
}
void previousTexture(){
  previousTexture(drawParams);
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

void setTexture(objid index, std::string textureName){
  if (world.textures.find(textureName) == world.textures.end()){
    loadTextureWorld(world, textureName);
  }

  auto textureId = world.textures.at(textureName).textureId;
  for (auto id : getIdsInGroup(world.sandbox, index)){
    GameObjectMesh* meshObj = std::get_if<GameObjectMesh>(&world.objectMapping.at(id));
    if (meshObj != NULL){
      meshObj -> texture.textureOverloadName = textureName;
      meshObj -> texture.textureOverloadId = textureId;       
    }

    GameObjectUIButton* buttonObj = std::get_if<GameObjectUIButton>(&world.objectMapping.at(id));
    if (buttonObj != NULL){
      buttonObj -> onTextureString = textureName;
      buttonObj -> onTexture = textureId;
      buttonObj -> offTextureString = textureName;
      buttonObj -> offTexture = textureId;
    }
  }
}
void maybeChangeTexture(int index){
  auto textureName = worldTextures(world).at(drawParams.activeTextureIndex).textureName;
  setTexture(index, textureName);
}

void setState(std::string stateName){
  if (stateName == "diffuse_on"){
    state.enableDiffuse = true;
  }else if (stateName == "diffuse_off"){
    state.enableDiffuse = false;
  }else if (stateName == "specular_on"){
    state.enableSpecular = true;
  }else if (stateName == "specular_off"){
    state.enableSpecular = false;
  }else if (stateName == "paint_on"){
    state.shouldPaint = true;
  }else if (stateName == "paint_off"){
    state.shouldPaint = false;
  }else if (stateName == "bloom_on"){
    state.enableBloom = true;
  }else if (stateName == "bloom_off"){
    state.enableBloom = false;
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
void setActiveCamera(int32_t cameraId){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(world.objectMapping);
  if (! (std::find(cameraIndexs.begin(), cameraIndexs.end(), cameraId) != cameraIndexs.end())){
    std::cout << "index: " << cameraId << " is not a valid index" << std::endl;
    return;
  }
  activeCameraObj = &getGameObject(world, cameraId);
  setSelectedIndex(state.editor, cameraId, activeCameraObj -> name, true);
}
void setActiveCamera(std::string name, objid sceneId){
  auto object = getGameObjectByName(name, sceneId);
  if (!object.has_value()){
    std::cout << "ERROR SETTING CAMERA: does the camera: " << name << " exist?" << std::endl;
    assert(false);
  }
  setActiveCamera(object.value());
}

void nextCamera(){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(world.objectMapping);
  if (cameraIndexs.size() == 0){  // if we do not have a camera in the scene, we use default
    activeCameraObj = NULL;
    return;
  }

  state.activeCamera = (state.activeCamera + 1) % cameraIndexs.size();
  int32_t activeCameraId = cameraIndexs.at(state.activeCamera);
  setActiveCamera(activeCameraId);
}
void moveCamera(glm::vec3 offset){
  if (activeCameraObj == NULL){
    defaultCamera.transformation.position = moveRelative(defaultCamera.transformation.position, defaultCamera.transformation.rotation, glm::vec3(offset), false);
  }else{
    setGameObjectPosition(activeCameraObj ->id, moveRelative(activeCameraObj -> transformation.position, activeCameraObj -> transformation.rotation, glm::vec3(offset), false));
  }
}
void rotateCamera(float xoffset, float yoffset){
  if (activeCameraObj == NULL){
    defaultCamera.transformation.rotation = setFrontDelta(defaultCamera.transformation.rotation, xoffset, yoffset, 0, 0.1);
  }else{
    setGameObjectRotation(activeCameraObj ->id, setFrontDelta(activeCameraObj -> transformation.rotation, xoffset, yoffset, 0, 0.1));
  }
}
void setCameraRotation(glm::quat orientation){
  if (activeCameraObj == NULL){
    defaultCamera.transformation.rotation = orientation;
  }else{
    setGameObjectRotation(activeCameraObj ->id, orientation);
  }
}

void playSoundState(std::string source, objid sceneId){
  auto gameobj = getGameObjectByName(world, source, sceneId);
  if (gameobj.has_value()){
    playSoundState(world.objectMapping, gameobj.value()); 
  }
}

unsigned int activeTextureId(){
  return world.textures.at(activeTextureName(drawParams, world.textures)).textureId;
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

void scmEmit(objid id){
  emit(world, id);
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
