#ifndef MOD_CSCRIPT_BINDING
#define MOD_CSCRIPT_BINDING

#include <string>
#include <functional>
#include <glm/gtx/compatibility.hpp>
#include "../common/util.h"

struct CustomApiBindings {
  int32_t (*listSceneId)(int32_t objid);
  int32_t (*loadScene)(std::string, std::vector<std::vector<std::string>>);
  void (*unloadScene)(int32_t id);
  void (*unloadAllScenes)();
  std::vector<int32_t> (*listScenes)();
  std::vector<std::string> (*listSceneFiles)();
  void (*sendLoadScene)(int32_t id);
  void (*createScene)(std::string scenename);
  void (*moveCamera)(glm::vec3);
  void (*rotateCamera)(float xoffset, float yoffset);
  void (*removeObjectById)(int32_t id);
  std::vector<int32_t> (*getObjectsByType)(std::string);
  void (*setActiveCamera)(int32_t cameraId, float interpolationTime);
  void (*drawText)(std::string word, float left, float top, unsigned int fontSize);
  int32_t (*drawLine)(glm::vec3 posFrom, glm::vec3 posTo, bool permaline, objid owner);
  void (*freeLine)(int32_t lineid);
  std::string (*getGameObjectNameForId)(int32_t id);
  GameobjAttributes (*getGameObjectAttr)(int32_t id);
  void (*setGameObjectAttr)(int32_t id, GameobjAttributes& attr);
  glm::vec3 (*getGameObjectPos)(int32_t index, bool world);
  void (*setGameObjectPos)(int32_t index, glm::vec3 pos);
  void (*setGameObjectPosRelative)(int32_t index, glm::vec3 pos);
  glm::quat (*getGameObjectRotation)(int32_t index, bool world);
  void (*setGameObjectRot)(int32_t index, glm::quat rotation);
  glm::quat (*setFrontDelta)(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta);
  glm::vec3 (*moveRelative)(glm::vec3 pos, glm::quat orientation, float distance);
  glm::vec3 (*moveRelativeVec)(glm::vec3 posFrom, glm::quat orientation, glm::vec3 vec);
  glm::quat (*orientationFromPos)(glm::vec3 fromPos, glm::vec3 toPos);
  std::optional<objid> (*getGameObjectByName)(std::string name, objid sceneId);
  void (*applyImpulse)(int32_t index, glm::vec3 impulse);
  void (*applyImpulseRel)(int32_t index, glm::vec3 impulse);
  void (*clearImpulse)(int32_t index);
  std::vector<std::string> (*listAnimations)(int32_t id);
  void (*playAnimation)(int32_t id, std::string animationToPlay);
  std::vector<std::string>(*listClips)();
  void (*playClip)(std::string, objid sceneId);

  ///////////
  std::vector<std::string> (*listModels)();
  void (*sendNotifyMessage)(std::string message, std::string value);
  double (*timeSeconds)();
  double (*timeElapsed)();
  void (*saveScene)(bool includeIds, objid sceneId);
  std::map<std::string, std::string> (*listServers)();
  std::string (*connectServer)(std::string server);
  void (*disconnectServer)();
  void (*sendMessageTcp)(std::string data);
  void (*sendMessageUdp)(std::string data);
  void (*playRecording)(objid id, std::string recordingPath);
  void (*stopRecording)(objid id, std::string recordingPath);
  objid (*createRecording)(objid id);
  void (*saveRecording)(objid recordingId, std::string filepath);
  objid (*makeObjectAttr)(objid sceneId, std::string name, std::map<std::string, std::string> stringAttributes, std::map<std::string, double> numAttributes, std::map<std::string, glm::vec3> vecAttributes);
  void (*makeParent)(objid child, objid parent);
  std::vector<HitObject> (*raycast)(glm::vec3 pos, glm::quat direction, float maxDistance);
  void (*saveScreenshot)(std::string);
  void (*setState)(std::string);
  void (*setFloatState)(std::string stateName, float value);
  void (*setIntState)(std::string stateName, int value);
  glm::vec3 (*navPosition)(objid, glm::vec3 pos);
  void (*emit)(objid id, std::optional<glm::vec3> initPosition, std::optional<glm::quat> initOrientation, std::optional<glm::vec3> initVelocity, std::optional<glm::vec3> initAvelocity);
  objid (*loadAround)(objid);
  void (*rmLoadAround)(objid);
  void (*generateMesh)(std::vector<glm::vec3> face, std::vector<glm::vec3> points, std::string);
  std::map<std::string, std::string> (*getArgs)();
  bool (*lock)(std::string, objid);
  bool (*unlock)(std::string, objid);
  void (*debugInfo)(std::string infoType, std::string filepath);
  void (*setWorldState)(std::vector<ObjectValue> values);
  void (*enforceLayout)(objid layoutId);
  //std::vector<func_t> registerGuileFns
};


struct CScriptBinding {
  std::string bindingMatcher;
  CustomApiBindings& api;
  std::function<void*(std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript)> create;
  std::function<void(std::string scriptname, objid id, void*)> remove;
  std::function<void(void*)> render;

  // Other callbacks
  id_func onFrame;
  id_colposfun onCollisionEnter;
  id_colfun onCollisionExit;
  id_mousecallback onMouseCallback;
  id_mousemovecallback onMouseMoveCallback;
  id_scrollcallback onScrollCallback;
  id_onobjectSelectedFunc onObjectSelected;
  id_onobjectHoverFunc onObjectHover;
  
  keycallback onKeyCallback;
  keycharcallback onKeyCharCallback;
  stringboolFunc onCameraSystemChange;
  string2func onMessage;
  stringfunc onTcpMessage;
  stringfunc onUdpMessage;
  stringfunc onPlayerJoined;
  stringfunc onPlayerLeave;
};

CScriptBinding createCScriptBinding(const char* bindingMatcher, CustomApiBindings& api);

#endif