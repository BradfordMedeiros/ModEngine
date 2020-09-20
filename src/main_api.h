#ifndef MOD_MAINAPI
#define MOD_MAINAPI

#include <GLFW/glfw3.h>
#include <queue>          
#include "./scene/scene.h"
#include "./state.h"
#include "./scene/physics.h"
#include "./scheme/scriptmanager.h"
#include "./scene/sprites/sprites.h"
#include "./scene/animation/playback.h"
#include "./scene/animation/timeplayback.h"
#include "./sounds/soundmanager.h"
#include "./network/servers.h"
#include "./network/activemanager.h"
#include "./common/sysinterface.h"

enum PacketType { CREATE, DELETE, UPDATE, SETUP, LOAD };
struct SetupPacket {
  char connectionHash[4000];
};
struct CreatePacket {
  short id;
  objid sceneId;
  char serialobj[3000];
};
struct DeletePacket {
  short id;
};
struct UpdatePacket {
  short id;
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

void setActiveCamera(short cameraId);
void nextCamera();
void moveCamera(glm::vec3 offset);  
void rotateCamera(float xoffset, float yoffset);

void applyImpulse(short index, glm::vec3 impulse);
void applyImpulseRel(short index, glm::vec3 impulse);
void clearImpulse(short index);

void loadScriptFromWorld(std::string script, short id);
short loadScene(std::string sceneFile);
short loadSceneObj(std::string sceneFile, short sceneId);
short loadSceneData(std::string sceneData, objid sceneId);
void unloadScene(short sceneId);
void unloadAllScenes();
void saveScene(bool includeIds);
std::vector<short> listScenes();
void sendLoadScene(short sceneId);

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2, glm::vec3 contactPos);
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2);

std::optional<objid> getGameObjectByName(std::string name);
std::vector<short> getObjectsByType(std::string type);
std::string getGameObjectName(short index);
std::map<std::string, std::string> getGameObjectAttr(short id);
void setGameObjectAttr(short id, std::map<std::string, std::string> attr);

glm::vec3 getGameObjectPosition(short index, bool isWorld);
void setGameObjectPosition(short index, glm::vec3 pos);
void setGameObjectPositionRelative(short index, float x, float y, float z, bool xzPlaneOnly);

glm::vec3 getGameObjectScale(short index);
void setGameObjectScale(short index, glm::vec3 scale);

glm::quat getGameObjectRotation(short index, bool isWorld);
void setGameObjectRotation(short index, glm::quat rotation);
void setSelectionMode(bool enabled);

short makeObject(std::string serializedobj, objid id, bool useObjId, objid sceneId = -1, bool useSceneId = false);
objid makeObjectAttr(std::string name, std::map<std::string, std::string> stringAttributes, std::map<std::string, double> numAttributes, std::map<std::string, glm::vec3> vecAttributes);

void removeObjectById(short id);

void drawText(std::string word, float left, float top, unsigned int fontSize);

struct AnimationState {
  std::map<short, TimePlayback> playbacks;
};
std::vector<std::string> listAnimations(short id);
void playAnimation(short id, std::string animationToPlay);

std::vector<std::string> listSounds(short id);
void playSound(std::string sound);

std::vector<std::string> listModels();

void sendEventMessage(std::string message);
void sendNotifyMessage(std::string message);

void attachToRail(short id, std::string rail);
void unattachFromRail(short id);
double timeSeconds();

void sendDataUdp(std::string data);
std::string connectServer(std::string server);

void startRecording(objid id, std::string recordingPath);
void playRecording(objid id, std::string recordingPath);
std::vector<objid> raycast(glm::vec3 pos, glm::quat direction, float maxDistance);
glm::vec3 moveRelative(glm::vec3, glm::quat orientation, float distance);

void copyStr(std::string& data, char* copyTo, int size);

#endif