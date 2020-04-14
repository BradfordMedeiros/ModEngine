#include "./main_api.h"

extern World world;
extern GameObject* activeCameraObj;
extern engineState state;
extern GameObject defaultCamera;
extern SchemeBindingCallbacks schemeBindings;
extern std::map<unsigned int, Mesh> fontMeshes;
extern unsigned int uiShaderProgram;

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
void clearImpulse(short index){
  clearImpulse(world.rigidbodys.at(index));
}

short loadScene(std::string sceneFile){
  std::cout << "INFO: SCENE LOADING: loading " << sceneFile << std::endl;
  return addSceneToWorld(world, sceneFile);
}
void unloadScene(short sceneId){  
  std::cout << "INFO: SCENE LOADING: unloading " << sceneId << std::endl;
  removeSceneFromWorld(world, sceneId);
}
std::vector<short> listScenes(){
  std::vector<short> sceneIds;
  for (auto &[id, _] : world.scenes){
    sceneIds.push_back(id);
  }
  return sceneIds;
}

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2){
  schemeBindings.onCollisionEnter(getIdForCollisionObject(world, obj1), getIdForCollisionObject(world, obj2));
}
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2){
  schemeBindings.onCollisionExit(getIdForCollisionObject(world, obj1), getIdForCollisionObject(world, obj2));
}

short getGameObjectByName(std::string name){    // @todo : odd behavior: currently these names do not have to be unique in different scenes.  this just finds first instance of that name.
  for (int i = 0; i < world.scenes.size(); i++){
    for (auto [id, gameObj]: world.scenes.at(i).idToGameObjects){
      if (gameObj.name == name){
        return id;
      }
    }
  }
  return -1;
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
glm::vec3 getGameObjectPosition(short index){
  auto sceneId = world.idToScene.at(index);
  return world.scenes.at(sceneId).idToGameObjects.at(index).transformation.position;
}
void setGameObjectPosition(short index, glm::vec3 pos){
  auto sceneId = world.idToScene.at(index);
  physicsTranslateSet(world.scenes.at(sceneId), world.rigidbodys.at(index), pos, index);
}
void setGameObjectPositionRelative(short index, float x, float y, float z, bool xzPlaneOnly){
  auto sceneId = world.idToScene.at(index);
  auto transformation = world.scenes.at(sceneId).idToGameObjects.at(index).transformation;
  glm::vec3 pos = moveRelative(transformation.position, transformation.rotation, glm::vec3(x, y, z), xzPlaneOnly);
  physicsTranslateSet(world.scenes.at(sceneId), world.rigidbodys.at(index), pos, index);
}

void setGameObjectRotation(short index, glm::quat rotation){
  auto sceneId = world.idToScene.at(index);
  physicsRotateSet(world.scenes.at(sceneId), world.rigidbodys.at(index), rotation,  index);
}
glm::quat getGameObjectRotation(short index){
  auto sceneId = world.idToScene.at(index);
  return world.scenes.at(sceneId).idToGameObjects.at(index).transformation.rotation;
}

void setSelectionMode(bool enabled){
  state.isSelectionMode = enabled;
}

void makeObject(std::string name, std::string meshName, float x, float y, float z){
  //addObjectToFullScene(world, 0, name, meshName, glm::vec3(x,y,z));
  std::cout << "make object called -- this doesn't work anymore with mutli scene" << std::endl;
}
void removeObjectById(short id){
  std::cout << "removing object by id: " << id << std::endl;
  removeObject(world.objectMapping, id);
  auto sceneId = world.idToScene.at(id);
  removeObjectFromScene(world.scenes.at(sceneId), id);
}

void drawText(std::string word, float left, float top, unsigned int fontSize){
  drawWords(uiShaderProgram, fontMeshes, word, left, top, fontSize);
}

std::vector<std::string> getAnimationsById(short id){
  std::vector<std::string> animationNames;
  animationNames.push_back("test animation");
  return animationNames;
}
void playAnimation(short id, std::string animationToPlay){
  auto animations = getAnimationsById(id);
  bool isValidAnimation = false;
  for (auto animation : animations){
    if (animation == animationToPlay){
      isValidAnimation = true;
      break;
    }
  }
  assert(isValidAnimation);
  std::cout << "play animation placeholder" << std::endl; // -- should add it to animation, maybe this should restart it, potentially replace the current animation playing" << std::endl;
}