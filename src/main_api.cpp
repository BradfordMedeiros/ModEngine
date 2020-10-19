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
extern std::queue<std::string> channelMessages;

extern float now;
extern std::string rawSceneFile;
extern bool bootStrapperMode;
extern NetCode netcode;
extern DrawingParams drawParams;

NetworkPacket toNetworkPacket(UdpPacket& packet){
  NetworkPacket netpacket {
    .packet = &packet,
    .packetSize = sizeof(packet),
  };
  return netpacket;
}

std::optional<objid> getGameObjectByName(std::string name){    // @todo : odd behavior: currently these names do not have to be unique in different scenes.  this just finds first instance of that name.
  return getGameObjectByName(world, name);
}

void setActiveCamera(int32_t cameraId){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(world.objectMapping);
  if (! (std::find(cameraIndexs.begin(), cameraIndexs.end(), cameraId) != cameraIndexs.end())){
    std::cout << "index: " << cameraId << " is not a valid index" << std::endl;
    return;
  }
  auto sceneId = world.idToScene.at(cameraId);
  activeCameraObj = &getGameObject(world, cameraId);
  state.selectedIndex = cameraId;
}
void setActiveCamera(std::string name){
  auto object = getGameObjectByName(name);
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
    std::cout << "default camera!" << std::endl;
    defaultCamera.transformation.position = moveRelative(defaultCamera.transformation.position, defaultCamera.transformation.rotation, glm::vec3(offset), false);
  }else{
    activeCameraObj -> transformation.position = moveRelative(activeCameraObj -> transformation.position, activeCameraObj -> transformation.rotation, glm::vec3(offset), false);
  }
}
void rotateCamera(float xoffset, float yoffset){
  if (activeCameraObj == NULL){
    defaultCamera.transformation.rotation = setFrontDelta(defaultCamera.transformation.rotation, xoffset, yoffset, 0, 0.1);
  }else{
    activeCameraObj -> transformation.rotation = setFrontDelta(activeCameraObj -> transformation.rotation, xoffset, yoffset, 0, 0.1);
  }
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
int32_t loadSceneObj(std::string sceneFile, int32_t sceneId){
  std::cout << "INFO: SCENE LOADING: loading subscene" << sceneFile << std::endl;
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
  auto id = world.scenes.begin() -> first;
  auto fileToSave = rawSceneFile;
  std::cout << "saving scene id: " << id << " to file: " << fileToSave << std::endl;
  saveFile(fileToSave, serializeScene(world, id, includeIds));
}

std::vector<int32_t> listScenes(){
  std::vector<int32_t> sceneIds;
  for (auto &[id, _] : world.scenes){
    sceneIds.push_back(id);
  }
  return sceneIds;
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
    return fullTransformation(world, index).position;
  }
  return getGameObject(world, index).transformation.position;
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
    return fullTransformation(world, index).rotation;
  }
  return getGameObject(world, index).transformation.rotation;
}

void setSelectionMode(bool enabled){
  state.isSelectionMode = enabled;
}

int32_t makeObject(std::string serializedobj, objid id, bool useObjId, objid sceneId, bool useSceneId){
  return addObjectToScene(world, useSceneId ? sceneId : world.scenes.begin() -> first, serializedobj, id, useObjId, interface);
}
objid makeObjectAttr(std::string name, std::map<std::string, std::string> stringAttributes, std::map<std::string, double> numAttributes, std::map<std::string, glm::vec3> vecAttributes){
  assert(world.scenes.size() > 0); 

  GameobjAttributes attributes {
    .stringAttributes = stringAttributes,
    .numAttributes = numAttributes,
    .vecAttributes = vecAttributes,
  };
  return addObjectToScene(world, world.scenes.begin() -> first, name, attributes, interface);
}

void removeObjectById(int32_t id){
  removeObjectFromScene(world, id, interface);
}

void drawText(std::string word, float left, float top, unsigned int fontSize){
  drawWords(uiShaderProgram, fontMeshes, word, left, top, fontSize);
}

std::vector<std::string> listAnimations(int32_t id){
  std::vector<std::string> animationNames;
  auto groupId = getGroupId(world, id);
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

void addAnimation(AnimationState& animationState, int32_t groupId, std::string animationToPlay){
  auto animation = getAnimation(world, groupId, animationToPlay);
  auto meshNameToMeshes = getMeshesForGroupId(world, groupId);  
  TimePlayback playback(
    initialTime, 
    [animation, meshNameToMeshes](float currentTime, float elapsedTime) -> void { 
      playbackAnimation(animation, world.meshnameToBoneToParent, meshNameToMeshes, currentTime, elapsedTime);  
    }, 
    [groupId, &animationState]() -> void { 
      playbacksToRemove.push_back(groupId);
    },
    animation.duration
  );  
  animationState.playbacks[groupId] = playback;
}

void playAnimation(int32_t id, std::string animationToPlay){
  auto groupId =  world.scenes.at(world.idToScene.at(id)).idToGameObjectsH.at(id).groupId;
  if (animations.playbacks.find(groupId) != animations.playbacks.end()){
    animations.playbacks.erase(groupId);
  }
  addAnimation(animations, groupId, animationToPlay);
}

std::vector<std::string> listModels(){
  return listFilesWithExtensions("../gameresources", { "obj", "dae" });
}

void sendEventMessage(std::string message){
  auto channelMapping = getChannelMapping(world.objectMapping);
  if (channelMapping.find(message) != channelMapping.end()){
    for (auto to : channelMapping.at(message)){
      std::cout << "SYSTEM INFO: channel: (" << message << ", " << to << ")" << std::endl;
      channelMessages.push(to);
    }
  }
}

void sendNotifyMessage(std::string message){
  channelMessages.push(message);
}

void attachToRail(int32_t id, std::string rail){
  addEntity(world.rails, id, rail);
}

void unattachFromRail(int32_t id){
  removeEntity(world.rails, id);
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

void startRecording(objid id, std::string recordingPath){
  std::cout << "start recording placeholder" << std::endl;
}
void playRecording(objid id, std::string recordingPath){
  std::cout << "play recording placeholder" << std::endl;
}

std::vector<objid> raycast(glm::vec3 pos, glm::quat direction, float maxDistance){
  std::vector<objid> ids;
  auto hitobjects = raycast(world, pos, direction, maxDistance);
  for (auto hitobject : hitobjects){
    ids.push_back(hitobject.id);
  }
  return ids;
}

glm::vec3 moveRelative(glm::vec3 posFrom, glm::quat orientation, float distance){
  return moveRelative(posFrom, orientation, glm::vec3(0.f, 0.f, -1 * distance), false);
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
  }
}

void setFloatState(std::string stateName, float value){
  if (stateName == "opacity"){
    drawParams.opacity = value;
  }
  if (stateName == "drawcolor-r"){
    drawParams.tint.x = value;
  }
  if (stateName == "drawcolor-g"){
    drawParams.tint.y = value;
  }
  if (stateName == "drawcolor-b"){
    drawParams.tint.z = value;
  }
}