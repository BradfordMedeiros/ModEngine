#ifndef MOD_SCHEMEBINDINGS
#define MOD_SCHEMEBINDINGS

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <libguile.h>
#include <limits>       
#include <map>

void createStaticSchemeBindings(
  short (*loadScene)(std::string),  
  void (*unloadScene)(short id),  
  std::vector<short> (*listScenes)(),  
  void (*moveCamera)(glm::vec3),  
  void (*rotateCamera)(float xoffset, float yoffset),
  void (*removeObjectById)(short id),
  void (*makeObjectV)(std::string, std::string, float, float, float),
  std::vector<short> (*getObjectsByType)(std::string),
  void (*setActiveCamera)(short cameraId),
  void (*drawText)(std::string word, float left, float top, unsigned int fontSize),
  std::string (*getGameObjectNameForId)(short id),
  std::map<std::string, std::string> getGameObjectAttr(short id),
  void (*setGameObjectAttr)(short id, std::map<std::string, std::string> attr),
  glm::vec3 (*getGameObjectPos)(short index),
  void (*setGameObjectPos)(short index, glm::vec3 pos),
  void (*setGameObjectPosRelative)(short index, float x, float y, float z, bool xzPlaneOnly),
  glm::quat (*getGameObjectRot)(short index),
  void (*setGameObjectRot)(short index, glm::quat rotation),
  glm::quat (*setFrontDelta)(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta),
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
  void (*saveScene)(),
  std::map<std::string, std::string> (*listServers)(),
  void (*connectServer)(std::string server),
  void (*sendMessageTcp)(std::string data),
  void (*sendMessageUdp)(std::string data)
);

void defineFunctions();

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
