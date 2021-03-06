#ifndef MOD_MAINAPI
#define MOD_MAINAPI

#include <GLFW/glfw3.h>
#include <queue>          
#include "./scene/scene.h"
#include "./scene/scene_object.h"
#include "./state.h"
#include "./scene/physics.h"
#include "./scheme/scriptmanager.h"
#include "./scene/sprites/sprites.h"
#include "./scene/types/sound.h"
#include "./network/servers.h"
#include "./network/activemanager.h"
#include "./common/sysinterface.h"
#include "./drawing.h"
#include "./worldloader.h"
#include "./netscene.h"
#include "./worldtiming.h"

NetworkPacket toNetworkPacket(UdpPacket& packet);

// This file is really just an extension of main.cpp (notice heavy usage of external scope) but organizes the business logic of the api functions
// These functions represent the functionality that individual language bindings use, but should not be coupled to any language in particular. 
// Should trim down when it becomes clear what the core api should.

objid listSceneId(int32_t id);

void setActiveCamera(int32_t cameraId);
void setActiveCamera(std::string name, objid sceneId);
void nextCamera();
void moveCamera(glm::vec3 offset);  
void rotateCamera(float xoffset, float yoffset);
void setCameraRotation(glm::quat orientation);

void applyImpulse(int32_t index, glm::vec3 impulse);
void applyImpulseRel(int32_t index, glm::vec3 impulse);
void clearImpulse(int32_t index);

void loadScriptFromWorld(std::string script, objid id, objid sceneId);

std::vector<std::string> listSceneFiles();
int32_t loadScene(std::string sceneFile);
int32_t loadSceneParentOffset(std::string sceneFile, glm::vec3 offset, std::string parentNodeName);
void unloadScene(int32_t sceneId);
void unloadAllScenes();
void saveScene(bool includeIds, objid sceneId);
std::vector<int32_t> listScenes();
void sendLoadScene(int32_t sceneId);
void createScene(std::string scenename);

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2, glm::vec3 contactPos);
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2);

std::optional<objid> getGameObjectByName(std::string name, objid sceneId);
std::vector<int32_t> getObjectsByType(std::string type);
std::string getGameObjectName(int32_t index);
GameobjAttributes getGameObjectAttr(int32_t id);
void setGameObjectAttr(int32_t id, GameobjAttributes& attr);

glm::vec3 getGameObjectPosition(int32_t index, bool isWorld);
glm::vec3 getGameObjectPos(int32_t index);
void setGameObjectPosition(int32_t index, glm::vec3 pos);
void setGameObjectPositionRelative(int32_t index, float x, float y, float z, bool xzPlaneOnly);

glm::vec3 getGameObjectScale(int32_t index);
void setGameObjectScale(int32_t index, glm::vec3 scale);

glm::quat getGameObjectRotation(int32_t index, bool isWorld);
void setGameObjectRotation(int32_t index, glm::quat rotation);
void setSelectionMode(bool enabled);

objid makeObjectAttr(objid sceneId, std::string name, std::map<std::string, std::string> stringAttributes, std::map<std::string, double> numAttributes, std::map<std::string, glm::vec3> vecAttributes);
void removeObjectById(int32_t id);
void copyObject(int32_t id);

void drawText(std::string word, float left, float top, unsigned int fontSize);
void drawWord(GLint shaderProgram, objid id, std::string word, unsigned int fontSize, float offsetDelta);

std::vector<std::string> listAnimations(int32_t id);
void playAnimation(int32_t id, std::string animationToPlay);
void stopAnimation(int32_t id);

std::vector<std::string> listSounds(int32_t id);
void playSound(std::string sound);

std::vector<std::string> listModels();

void sendEventMessage(std::string message);
void sendNotifyMessage(std::string message, std::string value);

double timeSeconds();

void playRecording(objid id, std::string recordingPath);
void stopRecording(objid id, std::string recordingPath);

void tickRecording(float time, GameObject& gameobject);
void tickRecordings(float time);

objid createRecording(objid id);
void saveRecording(objid recordingId, std::string filepath);

std::vector<HitObject> raycastW(glm::vec3 pos, glm::quat direction, float maxDistance);
glm::vec3 moveRelative(glm::vec3, glm::quat orientation, float distance);

void nextTexture();
void previousTexture();
void setTexture(objid index, std::string textureName);
void maybeChangeTexture(int index);

void setState(std::string stateName);
void setFloatState(std::string stateName, float value);
void setIntState(std::string stateName, int value);

void playSoundState(std::string source, objid sceneId);

unsigned int activeTextureId();

struct VoxelQueryData {
  int index;
  GameObjectVoxel* voxelPtr;
};
std::vector<VoxelQueryData> getSelectedVoxels();

void scmEmit(objid id);

objid addLoadingAround(objid id);
void removeLoadingAround(objid id);

void makeParent(objid child, objid parent);

void createGeneratedMesh(std::vector<glm::vec3> face, std::vector<glm::vec3> points, std::string destMesh);

glm::vec3 navPosition(objid id, glm::vec3 target);


#endif