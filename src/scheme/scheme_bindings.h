#ifndef MOD_SCHEMEBINDINGS
#define MOD_SCHEMEBINDINGS

#include <iostream>
#include <map>
#include <glm/gtx/compatibility.hpp>
#include "../common/util.h"
#include "./scriptmanager.h"      // need to eliminate the circular relationship here
#include "./scheme_util.h"

void createStaticSchemeBindings(
  int32_t (*listSceneId)(int32_t objid),
  int32_t (*loadScene)(std::string),
  void (*unloadScene)(int32_t id),  
  void (*unloadAllScenes)(),
  std::vector<int32_t> (*listScenes)(),  
  std::vector<std::string> (*listSceneFiles)(),
  void (*sendLoadScene)(int32_t id),
  void (*createScene)(std::string scenename),
  void (*moveCamera)(glm::vec3),  
  void (*rotateCamera)(float xoffset, float yoffset),
  void (*removeObjectById)(int32_t id),
  std::vector<int32_t> (*getObjectsByType)(std::string),
  void (*setActiveCamera)(int32_t cameraId),
  void (*drawText)(std::string word, float left, float top, unsigned int fontSize),
  void (*drawLine)(glm::vec3 posFrom, glm::vec3 posTo),  
  std::string (*getGameObjectNameForId)(int32_t id),
  GameobjAttributes getGameObjectAttr(int32_t id),
  void (*setGameObjectAttr)(int32_t id, GameobjAttributes& attr),
  glm::vec3 (*getGameObjectPos)(int32_t index, bool world),
  void (*setGameObjectPos)(int32_t index, glm::vec3 pos),
  void (*setGameObjectPosRelative)(int32_t index, float x, float y, float z, bool xzPlaneOnly),
  glm::quat (*getGameObjectRotation)(int32_t index, bool world),
  void (*setGameObjectRot)(int32_t index, glm::quat rotation),
  glm::quat (*setFrontDelta)(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta),
  glm::vec3 (*moveRelative)(glm::vec3 pos, glm::quat orientation, float distance),
  glm::quat (*orientationFromPos)(glm::vec3 fromPos, glm::vec3 toPos),
  std::optional<objid> (*getGameObjectByName)(std::string name, objid sceneId),
  void (*setSelectionMode)(bool enabled),
  void (*applyImpulse)(int32_t index, glm::vec3 impulse),
  void (*applyImpulseRel)(int32_t index, glm::vec3 impulse),
  void (*clearImpulse)(int32_t index),
  std::vector<std::string> (*listAnimations)(int32_t id),
  void playAnimation(int32_t id, std::string animationToPlay),
  std::vector<std::string>(*listClips)(),
  void (*playClip)(std::string, objid sceneId),
  std::vector<std::string> (*listModels)(),
  void (*sendEventMessage)(std::string message),
  void (*sendNotifyMessage)(std::string message, std::string value),
  double (*timeSeconds)(),
  void (*saveScene)(bool includeIds, objid sceneId), 
  std::map<std::string, std::string> (*listServers)(),
  std::string (*connectServer)(std::string server),
  void (*disconnectServer)(),
  void (*sendMessageTcp)(std::string data),
  void (*sendMessageUdp)(std::string data),
  void (*playRecording)(objid id, std::string recordingPath),
  void (*stopRecording)(objid id, std::string recordingPath),
  objid (*createRecording)(objid id),
  void (*saveRecording)(objid recordingId, std::string filepath),
  objid (*makeObjectAttr)(objid sceneId, std::string name, std::map<std::string, std::string> stringAttributes, std::map<std::string, double> numAttributes, std::map<std::string, glm::vec3> vecAttributes),
  void (*makeParent)(objid child, objid parent),
  std::vector<HitObject> (*raycast)(glm::vec3 pos, glm::quat direction, float maxDistance),
  void (*saveScreenshot)(std::string),
  void (*setState)(std::string),
  void (*setFloatState)(std::string stateName, float value),
  void (*setIntState)(std::string stateName, int value),
  glm::vec3 (*navPosition)(objid, glm::vec3 pos),
  void (*scmEmit)(objid),
  objid (*loadAround)(objid),
  void (*rmLoadAround)(objid),
  void (*generateMesh)(std::vector<glm::vec3> face, std::vector<glm::vec3> points, std::string),
  void (*setSkybox)(std::string),
  std::map<std::string, std::string> (*getArgs)(),
  bool (*lock)(std::string, objid),
  bool (*unlock)(std::string, objid),
  void (*debugInfo)(std::string infoType, std::string filepath),
  std::vector<func_t> registerGuileFns
);

void defineFunctions(objid id, bool isServer);

void onFrame();
void onCollisionEnter(int32_t obj1, glm::vec3 contactPos, glm::vec3 normal);
void onCollisionExit(int32_t obj1);
void onMouseCallback(int button, int action, int mods);
void onMouseMoveCallback(double xPos, double yPos);
void onScrollCallback(double amount);
void onObjectSelected(int32_t index, glm::vec3 color);
void onObjectHover(int32_t index);
void onObjectUnhover(int32_t index);
void onKeyCallback(int key, int scancode, int action, int mods);
void onKeyCharCallback(unsigned int codepoint);
void onCameraSystemChange(bool usingBuiltInCamera);
void onAttrMessage(std::string message, AttributeValue value);
void onTcpMessage(std::string message);
void onUdpMessage(std::string message);
void onPlayerJoined(std::string connectionHash);
void onPlayerLeave(std::string connectionHash);
void onScriptUnload();

#endif
