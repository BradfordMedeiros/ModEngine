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
#include "../gizmo/sequencer.h"   // TODO -  don't depend on this directly 
#include "../sql.h" // TODO don't depend on this directly

void createStaticSchemeBindings(
  int32_t (*loadScene)(std::string),
  void (*unloadScene)(int32_t id),  
  void (*unloadAllScenes)(),
  std::vector<int32_t> (*listScenes)(),  
  void (*sendLoadScene)(int32_t id),
  void (*moveCamera)(glm::vec3),  
  void (*rotateCamera)(float xoffset, float yoffset),
  void (*removeObjectById)(int32_t id),
  std::vector<int32_t> (*getObjectsByType)(std::string),
  void (*setActiveCamera)(int32_t cameraId),
  void (*drawText)(std::string word, float left, float top, unsigned int fontSize),
  void (*drawLine)(glm::vec3 posFrom, glm::vec3 posTo),  
  std::string (*getGameObjectNameForId)(int32_t id),
  std::map<std::string, std::string> getGameObjectAttr(int32_t id),
  void (*setGameObjectAttr)(int32_t id, std::map<std::string, std::string> attr),
  glm::vec3 (*getGameObjectPos)(int32_t index, bool world),
  void (*setGameObjectPos)(int32_t index, glm::vec3 pos),
  void (*setGameObjectPosRelative)(int32_t index, float x, float y, float z, bool xzPlaneOnly),
  glm::quat (*getGameObjectRotation)(int32_t index, bool world),
  void (*setGameObjectRot)(int32_t index, glm::quat rotation),
  glm::quat (*setFrontDelta)(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta),
  glm::vec3 (*moveRelative)(glm::vec3 pos, glm::quat orientation, float distance),
  glm::quat (*orientationFromPos)(glm::vec3 fromPos, glm::vec3 toPos),
  std::optional<objid> (*getGameObjectByName)(std::string name),
  void (*setSelectionMode)(bool enabled),
  void (*applyImpulse)(int32_t index, glm::vec3 impulse),
  void (*applyImpulseRel)(int32_t index, glm::vec3 impulse),
  void (*clearImpulse)(int32_t index),
  std::vector<std::string> (*listAnimations)(int32_t id),
  void playAnimation(int32_t id, std::string animationToPlay),
  std::vector<std::string>(*listClips)(),
  void (*playClip)(std::string),
  std::vector<std::string> (*listModels)(),
  void (*sendEventMessage)(std::string message),
  void (*sendNotifyMessage)(std::string message),
  double (*timeSeconds)(),
  void (*saveScene)(bool includeIds),
  std::map<std::string, std::string> (*listServers)(),
  std::string (*connectServer)(std::string server),
  void (*disconnectServer)(),
  void (*sendMessageTcp)(std::string data),
  void (*sendMessageUdp)(std::string data),
  Track (*createTrack)(std::string, std::vector<std::function<void()>> fns),
  void (*playbackTrack)(Track& track),
  StateMachine (*createStateMachine)(std::vector<State> states),
  void (*playStateMachine)(StateMachine* machine),
  void (*setStateMachine)(StateMachine* machine, std::string newState),
  void (*playRecording)(objid id, std::string recordingPath),
  void (*stopRecording)(objid id, std::string recordingPath),
  objid (*createRecording)(objid id),
  void (*saveRecording)(objid recordingId, std::string filepath),
  objid (*makeObjectAttr)(std::string name, std::map<std::string, std::string> stringAttributes, std::map<std::string, double> numAttributes, std::map<std::string, glm::vec3> vecAttributes),
  std::vector<HitObject> (*raycast)(glm::vec3 pos, glm::quat direction, float maxDistance),
  void (*saveScreenshot)(std::string),
  void (*setState)(std::string),
  void (*setFloatState)(std::string stateName, float value),
  void (*setIntState)(std::string stateName, int value),
  void (*setTexture)(objid id, std::string texture),
  glm::vec3 (*navPosition)(objid, glm::vec3 pos),
  void (*scmEmit)(objid)
);

void defineFunctions(objid id, bool isServer);

void onFrame();
void onCollisionEnter(int32_t obj1, int32_t obj2, glm::vec3 contactPos);
void onCollisionExit(int32_t obj1, int32_t obj2);
void onMouseCallback(int button, int action, int mods);
void onMouseMoveCallback(double xPos, double yPos);
void onScrollCallback(double amount);
void onObjectSelected(int32_t index, glm::vec3 color);
void onObjectHover(int32_t index);
void onObjectUnhover(int32_t index);
void onKeyCallback(int key, int scancode, int action, int mods);
void onKeyCharCallback(unsigned int codepoint);
void onCameraSystemChange(bool usingBuiltInCamera);
void onMessage(std::string message);
void onFloatMessage(StringFloat message);
void onTcpMessage(std::string message);
void onUdpMessage(std::string message);
void onPlayerJoined(std::string connectionHash);
void onPlayerLeave(std::string connectionHash);

#endif
