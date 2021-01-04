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
#include "./scene/animation/playback.h"
#include "./scene/animation/timeplayback.h"
#include "./scene/animation/recorder.h"
#include "./scene/types/sound.h"
#include "./network/servers.h"
#include "./network/activemanager.h"
#include "./common/sysinterface.h"
#include "./drawing.h"

struct Properties {
  Transformation transformation;
};
enum PacketType { CREATE, DELETE, UPDATE, SETUP, LOAD };
struct SetupPacket {
  char connectionHash[4000];
};
struct CreatePacket {
  int32_t id;
  objid sceneId;
  char serialobj[3000];
};
struct DeletePacket {
  int32_t id;
};
struct UpdatePacket {
  int32_t id;
  Properties properties;
};

// @TODO this should optimize so only send the size necessary, not max size since scnee file needs to be larger
struct LoadPacket {
  objid sceneId;
  char sceneData[4000]; // this makes every message 4k, which is probably way bigger than each packet needs to be, optimize this
};
union PacketPayload {
  SetupPacket setuppacket;
  CreatePacket createpacket;
  DeletePacket deletepacket;
  UpdatePacket updatepacket;
  LoadPacket loadpacket;
};
struct UdpPacket {
  PacketType type;
  PacketPayload payload;
};

NetworkPacket toNetworkPacket(UdpPacket& packet);

// This file is really just an extension of main.cpp (notice heavy usage of external scope) but organizes the business logic of the api functions
// These functions represent the functionality that individual language bindings use, but should not be coupled to any language in particular. 
// Should trim down when it becomes clear what the core api should.

void setActiveCamera(int32_t cameraId);
void setActiveCamera(std::string name);
void nextCamera();
void moveCamera(glm::vec3 offset);  
void rotateCamera(float xoffset, float yoffset);

void applyImpulse(int32_t index, glm::vec3 impulse);
void applyImpulseRel(int32_t index, glm::vec3 impulse);
void clearImpulse(int32_t index);

void loadScriptFromWorld(std::string script, int32_t id);
int32_t loadScene(std::string sceneFile);
int32_t loadSceneObj(std::string sceneFile, int32_t sceneId);
int32_t loadSceneData(std::string sceneData, objid sceneId);
void unloadScene(int32_t sceneId);
void unloadAllScenes();
void saveScene(bool includeIds);
std::vector<int32_t> listScenes();
void sendLoadScene(int32_t sceneId);

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2, glm::vec3 contactPos);
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2);

std::optional<objid> getGameObjectByName(std::string name);
std::vector<int32_t> getObjectsByType(std::string type);
std::string getGameObjectName(int32_t index);
std::map<std::string, std::string> getGameObjectAttr(int32_t id);
void setGameObjectAttr(int32_t id, std::map<std::string, std::string> attr);

glm::vec3 getGameObjectPosition(int32_t index, bool isWorld);
void setGameObjectPosition(int32_t index, glm::vec3 pos);
void setGameObjectPositionRelative(int32_t index, float x, float y, float z, bool xzPlaneOnly);

glm::vec3 getGameObjectScale(int32_t index);
void setGameObjectScale(int32_t index, glm::vec3 scale);

glm::quat getGameObjectRotation(int32_t index, bool isWorld);
void setGameObjectRotation(int32_t index, glm::quat rotation);
void setSelectionMode(bool enabled);

int32_t makeObject(std::string serializedobj, objid id, bool useObjId, objid sceneId = -1, bool useSceneId = false);
objid makeObjectAttr(std::string name, std::map<std::string, std::string> stringAttributes, std::map<std::string, double> numAttributes, std::map<std::string, glm::vec3> vecAttributes);
void removeObjectById(int32_t id);
void copyObject(int32_t id);

void drawText(std::string word, float left, float top, unsigned int fontSize);

struct AnimationState {
  std::map<int32_t, TimePlayback> playbacks;
};
std::vector<std::string> listAnimations(int32_t id);
void playAnimation(int32_t id, std::string animationToPlay);

std::vector<std::string> listSounds(int32_t id);
void playSound(std::string sound);

std::vector<std::string> listModels();

void sendEventMessage(std::string message);
void sendNotifyMessage(std::string message);

double timeSeconds();

void sendDataUdp(std::string data);
std::string connectServer(std::string server);

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

void copyStr(std::string& data, char* copyTo, int size);

void playSoundState(std::string source);

unsigned int activeTextureId();

#endif