#ifndef MOD_SCHEMEBINDINGS
#define MOD_SCHEMEBINDINGS

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <libguile.h>
#include <limits>       
#include <map>
#include "../common/util.h"
#include "../sequencer.h"

void createStaticSchemeBindings(
  short (*loadScene)(std::string),  
  void (*unloadScene)(short id),  
  void (*unloadAllScenes)(),
  std::vector<short> (*listScenes)(),  
  void (*sendLoadScene)(short id),
  void (*moveCamera)(glm::vec3),  
  void (*rotateCamera)(float xoffset, float yoffset),
  void (*removeObjectById)(short id),
  std::vector<short> (*getObjectsByType)(std::string),
  void (*setActiveCamera)(short cameraId),
  void (*drawText)(std::string word, float left, float top, unsigned int fontSize),
  void (*drawLine)(glm::vec3 posFrom, glm::vec3 posTo),  
  std::string (*getGameObjectNameForId)(short id),
  std::map<std::string, std::string> getGameObjectAttr(short id),
  void (*setGameObjectAttr)(short id, std::map<std::string, std::string> attr),
  glm::vec3 (*getGameObjectPos)(short index, bool world),
  void (*setGameObjectPos)(short index, glm::vec3 pos),
  void (*setGameObjectPosRelative)(short index, float x, float y, float z, bool xzPlaneOnly),
  glm::quat (*getGameObjectRotation)(short index, bool world),
  void (*setGameObjectRot)(short index, glm::quat rotation),
  glm::quat (*setFrontDelta)(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta),
  glm::vec3 (*moveRelative)(glm::vec3 pos, glm::quat orientation, float distance),
  short (*getGameObjectByName)(std::string name),
  void (*setSelectionMode)(bool enabled),
  void (*applyImpulse)(short index, glm::vec3 impulse),
  void (*applyImpulseRel)(short index, glm::vec3 impulse),
  void (*clearImpulse)(short index),
  std::vector<std::string> (*listAnimations)(short id),
  void playAnimation(short id, std::string animationToPlay),
  std::vector<std::string>(*listClips)(),
  void (*playClip)(std::string),
  std::vector<std::string> (*listModels)(),
  void (*sendEventMessage)(std::string message),
  void (*attachToRail)(short id, std::string rail),
  void (*unattachFromRail)(short id),
  double (*timeSeconds)(),
  void (*saveScene)(bool includeIds),
  std::map<std::string, std::string> (*listServers)(),
  void (*connectServer)(std::string server),
  void (*disconnectServer)(),
  void (*sendMessageTcp)(std::string data),
  void (*sendMessageUdp)(std::string data),
  Track (*createTrack)(std::string, std::vector<std::function<void()>> fns),
  void (*playbackTrack)(Track& track),
  StateMachine (*createStateMachine)(std::vector<State> states),
  void (*playStateMachine)(StateMachine* machine),
  void (*setStateMachine)(StateMachine* machine, std::string newState),
  void (*startRecording)(objid id, std::string recordingPath),
  void (*playRecording)(objid id, std::string recordingPath),
  objid (*makeObjectAttr)(std::string name, std::map<std::string, std::string> stringAttributes, std::map<std::string, double> numAttributes, std::map<std::string, glm::vec3> vecAttributes),
  std::vector<objid> (*raycast)(glm::vec3 pos, glm::quat direction, float maxDistance)
);

void defineFunctions(objid id, bool isServer);

void onFrame();
void onCollisionEnter(short obj1, short obj2, glm::vec3 contactPos);
void onCollisionExit(short obj1, short obj2);
void onMouseCallback(int button, int action, int mods);
void onMouseMoveCallback(double xPos, double yPos);
void onObjectSelected(short index);
void onKeyCallback(int key, int scancode, int action, int mods);
void onKeyCharCallback(unsigned int codepoint);
void onCameraSystemChange(bool usingBuiltInCamera);
void onMessage(std::string message);
void onTcpMessage(std::string message);
void onUdpMessage(std::string message);
void onPlayerJoined(std::string connectionHash);
void onPlayerLeave(std::string connectionHash);

#endif
