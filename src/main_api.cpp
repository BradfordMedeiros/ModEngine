#include "./main_api.h"

extern World world;
extern AnimationState animations;
extern GameObject* activeCameraObj;
extern engineState state;
extern GameObject defaultCamera;
extern SchemeBindingCallbacks schemeBindings;
extern std::map<unsigned int, Mesh> fontMeshes;
extern unsigned int uiShaderProgram;
extern float initialTime;
extern std::vector<short> playbacksToRemove;
extern std::vector<std::string> channelMessages;
extern float now;
extern std::string rawSceneFile;
extern bool bootStrapperMode;
extern NetCode netcode;

NetworkPacket toNetworkPacket(UdpPacket& packet){
  NetworkPacket netpacket {
    .packet = &packet,
    .packetSize = sizeof(packet),
  };
  return netpacket;
}

void setActiveCamera(short cameraId){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(world.objectMapping);
  if (! (std::find(cameraIndexs.begin(), cameraIndexs.end(), cameraId) != cameraIndexs.end())){
    std::cout << "index: " << cameraId << " is not a valid index" << std::endl;
    return;
  }
  auto sceneId = world.idToScene.at(cameraId);
  activeCameraObj = &world.scenes.at(sceneId).idToGameObjects.at(cameraId);
  state.selectedIndex = cameraId;
}
void nextCamera(){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(world.objectMapping);
  if (cameraIndexs.size() == 0){  // if we do not have a camera in the scene, we use default
    activeCameraObj = NULL;
    return;
  }

  state.activeCamera = (state.activeCamera + 1) % cameraIndexs.size();
  short activeCameraId = cameraIndexs.at(state.activeCamera);
  setActiveCamera(activeCameraId);
  std::cout << "active camera is: " << state.activeCamera << std::endl;
}
void moveCamera(glm::vec3 offset){
  defaultCamera.transformation.position = moveRelative(defaultCamera.transformation.position, defaultCamera.transformation.rotation, glm::vec3(offset), false);
}
void rotateCamera(float xoffset, float yoffset){
  defaultCamera.transformation.rotation = setFrontDelta(defaultCamera.transformation.rotation, xoffset, yoffset, 0, 0.1);
}

void applyImpulse(short index, glm::vec3 impulse){
  applyImpulse(world.rigidbodys.at(index), impulse);
}
void applyImpulseRel(short index, glm::vec3 impulse){
  glm::vec3 relativeImpulse = calculateRelativeOffset(world.scenes.at(world.idToScene.at(index)).idToGameObjects.at(index).transformation.rotation, impulse, true);
  applyImpulse(world.rigidbodys.at(index), relativeImpulse);
}

void clearImpulse(short index){
  clearImpulse(world.rigidbodys.at(index));
}

void loadScriptFromWorld(std::string script, short id){
  auto name = world.scenes.at(world.idToScene.at(id)).idToGameObjects.at(id).name;
  std::cout << "gameobj: " << name << " wants to load script: (" << script << ")" << std::endl;
  loadScript(script);
}
short loadScene(std::string sceneFile){
  std::cout << "INFO: SCENE LOADING: loading " << sceneFile << std::endl;
  return addSceneToWorld(world, sceneFile, loadSoundState, loadScriptFromWorld);
}
short loadSceneData(std::string sceneData){
  std::cout << "INFO: SCENE LOADING: loading from scene data" << std::endl;
  return addSceneToWorldFromData(world, sceneData, loadSoundState, loadScriptFromWorld);
}

void unloadScene(short sceneId){  
  std::cout << "INFO: SCENE LOADING: unloading " << sceneId << std::endl;
  removeSceneFromWorld(world, sceneId, unloadSoundState);
}
void unloadAllScenes(){
  removeAllScenesFromWorld(world, unloadSoundState);
}

// @TODO - save all the scenes in the world
void saveScene(bool includeIds){
  auto id = 0;
  auto fileToSave = rawSceneFile;
  std::cout << "saving scene id: " << id << " to file: " << fileToSave << std::endl;
  saveFile(fileToSave, serializeScene(world, id, includeIds));
}

std::vector<short> listScenes(){
  std::vector<short> sceneIds;
  for (auto &[id, _] : world.scenes){
    sceneIds.push_back(id);
  }
  return sceneIds;
}

void sendLoadScene(short id){
  if (!bootStrapperMode){
    std::cout << "ERROR: cannot send load scene in not-server mode" << std::endl;
    assert(false);
  }

  std::string sceneData = serializeScene(world, id, true);
  UdpPacket packet { .type = LOAD };
  auto data = sceneData.c_str();
  LoadPacket loadpacket {};
  assert((sizeof(data) + 1 ) < sizeof(loadpacket.sceneData));
  strncpy(loadpacket.sceneData, data, sizeof(loadpacket.sceneData));
  assert(loadpacket.sceneData[sizeof(loadpacket.sceneData) -1] == '\0');
  packet.payload.loadpacket = loadpacket; 
  sendUdpPacketToAllUdpClients(netcode, toNetworkPacket(packet));
}

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2, glm::vec3 contactPos){
  schemeBindings.onCollisionEnter(getIdForCollisionObject(world, obj1), getIdForCollisionObject(world, obj2), contactPos);
}
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2){
  schemeBindings.onCollisionExit(getIdForCollisionObject(world, obj1), getIdForCollisionObject(world, obj2));
}

short getGameObjectByName(std::string name){    // @todo : odd behavior: currently these names do not have to be unique in different scenes.  this just finds first instance of that name.
  return getGameObjectByName(world, name);
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
  return world.scenes.at(sceneId).idToGameObjects.at(index).name;
}

std::map<std::string, std::string> getGameObjectAttr(short id){
  return getAttributes(world, id);
}
void setGameObjectAttr(short id, std::map<std::string, std::string> attr){
  setAttributes(world, id, attr);
}

glm::vec3 getGameObjectPosition(short index){
  auto sceneId = world.idToScene.at(index);
  return world.scenes.at(sceneId).idToGameObjects.at(index).transformation.position;
}
void setGameObjectPosition(short index, glm::vec3 pos){
  physicsTranslateSet(world, index, pos);
}
void setGameObjectPositionRelative(short index, float x, float y, float z, bool xzPlaneOnly){
  auto sceneId = world.idToScene.at(index);
  auto transformation = world.scenes.at(sceneId).idToGameObjects.at(index).transformation;
  glm::vec3 pos = moveRelative(transformation.position, transformation.rotation, glm::vec3(x, y, z), xzPlaneOnly);
  physicsTranslateSet(world, index, pos);
}
glm::vec3 getGameObjectScale(short index){
  auto sceneId = world.idToScene.at(index);
  return world.scenes.at(sceneId).idToGameObjects.at(index).transformation.scale;
}
void setGameObjectScale(short index, glm::vec3 scale){
  physicsScaleSet(world, index, scale);
}
void setGameObjectRotation(short index, glm::quat rotation){
  auto sceneId = world.idToScene.at(index);
  physicsRotateSet(world, index, rotation);
}
glm::quat getGameObjectRotation(short index){
  auto sceneId = world.idToScene.at(index);
  return world.scenes.at(sceneId).idToGameObjects.at(index).transformation.rotation;
}

void setSelectionMode(bool enabled){
  state.isSelectionMode = enabled;
}

short makeObject(std::string name, std::string meshName, float x, float y, float z, objid id, bool useObjId){
  return addObjectToScene(world, 0, name, meshName, glm::vec3(x, y, z), id, useObjId, loadSoundState, loadScriptFromWorld);
}
short makeObject(std::string serializedobj, objid id, bool useObjId){
  return addObjectToScene(world, 0, serializedobj, id, useObjId, loadSoundState, loadScriptFromWorld);
}
void removeObjectById(short id){
  removeObjectFromScene(world, id, unloadSoundState);
}

void drawText(std::string word, float left, float top, unsigned int fontSize){
  drawWords(uiShaderProgram, fontMeshes, word, left, top, fontSize);
}

std::vector<std::string> listAnimations(short id){
  std::vector<std::string> animationNames;
  auto groupId =  world.scenes.at(world.idToScene.at(id)).idToGameObjectsH.at(id).groupId;
  if (world.animations.find(groupId) == world.animations.end()){
    return animationNames;
  }
  auto animations = world.animations.at(groupId);
  for (auto animation : animations){
    animationNames.push_back(animation.name);
  }
  return animationNames;
}
Animation getAnimation(World& world, short groupId, std::string animationToPlay){  
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

void addAnimation(AnimationState& animationState, short groupId, std::string animationToPlay){
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

void playAnimation(short id, std::string animationToPlay){
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
      channelMessages.push_back(to);
    }
  }
}

void attachToRail(short id, std::string rail){
  addEntity(world.rails, id, rail);
}

void unattachFromRail(short id){
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

void connectServer(std::string data){
  UdpPacket setup = {
    .type = SETUP,
  };  

  SetupPacket setupPacket {};
  auto packet = toNetworkPacket(setup);

  connectServer(data, [&setup, &setupPacket, &packet](std::string connectionHash) -> NetworkPacket {
    auto data = connectionHash.c_str();
    assert((sizeof(data) + 1 ) < sizeof(setupPacket.connectionHash));
    strncpy(setupPacket.connectionHash, data, sizeof(setupPacket.connectionHash));
    assert(setupPacket.connectionHash[sizeof(setupPacket.connectionHash) -1] == '\0');
    setup.payload.setuppacket = setupPacket;
    return packet;
  });
}