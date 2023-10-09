#ifndef MOD_CSCRIPT_BINDING
#define MOD_CSCRIPT_BINDING

#include <string>
#include <functional>
#include <any>
#include <glm/gtx/compatibility.hpp>
#include "../common/util.h"
#include "../sql/sql.h"  // ideally move this away from being a direct dependency

struct CustomApiBindings {
  int32_t (*listSceneId)(int32_t objid);
  int32_t (*loadScene)(std::string sceneFile, std::vector<std::vector<std::string>> additionalTokens, std::optional<std::string> name, std::optional<std::vector<std::string>> tags);
  void (*unloadScene)(int32_t id);
  void (*unloadAllScenes)();
  void (*resetScene)(std::optional<objid> sceneId);
  std::vector<int32_t> (*listScenes)(std::optional<std::vector<std::string>> tags);
  std::vector<std::string> (*listSceneFiles)(std::optional<objid> sceneId);
  bool (*parentScene)(objid sceneId, objid* _parentSceneId);
  std::vector<objid> (*childScenes)(objid sceneId);
  std::optional<objid> (*sceneIdByName)(std::string name);
  objid (*rootIdForScene)(objid sceneId);
  std::vector<ScenegraphDebug> (*scenegraph)();
  void (*sendLoadScene)(int32_t id);
  void (*createScene)(std::string scenename);
  void (*deleteScene)(std::string scenename);
  void (*moveCamera)(glm::vec3, std::optional<bool> relative);  // this should be deleted - move through normal methods instead
  void (*rotateCamera)(float xoffset, float yoffset);           // should be deleted - move a camera instead
  void (*removeObjectById)(int32_t id);
  std::vector<int32_t> (*getObjectsByType)(std::string);
  std::vector<int32_t> (*getObjectsByAttr)(std::string, std::optional<AttributeValue>, std::optional<int32_t>);
  void (*setActiveCamera)(int32_t cameraId, float interpolationTime);
  Transformation (*getView)();
  void (*drawText)(std::string word, float left, float top, unsigned int fontSize, bool permatext, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<std::string> fontFamily, std::optional<objid> selectionId);
  void (*drawRect)(float centerX, float centerY, float width, float height, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture);
  void (*drawLine2D)(glm::vec3 fromPos, glm::vec3 toPos, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture);
  int32_t (*drawLine)(glm::vec3 posFrom, glm::vec3 posTo, bool permaline, objid owner, std::optional<glm::vec4> color, std::optional<unsigned int> textureId, std::optional<unsigned int> linewidth);
  void (*freeLine)(int32_t lineid);
  std::optional<std::string> (*getGameObjNameForId)(int32_t id);
  GameobjAttributes (*getGameObjectAttr)(int32_t id);
  void (*setGameObjectAttr)(int32_t id, GameobjAttributes& attr);
  glm::vec3 (*getGameObjectPos)(int32_t index, bool world);
  void (*setGameObjectPosition)(int32_t index, glm::vec3 pos, bool world);
  glm::quat (*getGameObjectRotation)(int32_t index, bool world);
  void (*setGameObjectRot)(int32_t index, glm::quat rotation, bool world);
  void (*setGameObjectScale)(int32_t index, glm::vec3 scale, bool world);
  glm::quat (*setFrontDelta)(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta);
  glm::vec3 (*moveRelative)(glm::vec3 pos, glm::quat orientation, float distance);
  glm::vec3 (*moveRelativeVec)(glm::vec3 posFrom, glm::quat orientation, glm::vec3 vec);
  glm::quat (*orientationFromPos)(glm::vec3 fromPos, glm::vec3 toPos);
  std::optional<objid> (*getGameObjectByName)(std::string name, objid sceneId, bool sceneIdExplicit);
  void (*applyImpulse)(int32_t index, glm::vec3 impulse);
  void (*applyImpulseRel)(int32_t index, glm::vec3 impulse);
  void (*clearImpulse)(int32_t index);
  void (*applyForce)(int32_t index, glm::vec3 force);
  void (*applyTorque)(int32_t index, glm::vec3 torque);
  std::optional<ModAABB> (*getModAABB)(int32_t index); 
  std::vector<std::string> (*listAnimations)(int32_t id);
  void (*playAnimation)(int32_t id, std::string animationToPlay, AnimationType animationType);
  void (*stopAnimation)(int32_t id);
  std::vector<std::string>(*listClips)();
  void (*playClip)(std::string, objid sceneId, std::optional<float> volume, std::optional<glm::vec3> position);
  void (*playClipById)(objid id, std::optional<float> volume, std::optional<glm::vec3> position);
  void (*stopClip)(std::string source, objid sceneId);

  ///////////
  std::vector<std::string> (*listResources)(std::string);
  void (*sendNotifyMessage)(std::string message, std::any value);
  double (*timeSeconds)(bool realtime);
  double (*timeElapsed)();
  bool (*saveScene)(bool includeIds, objid sceneId, std::optional<std::string> filename);
  void (*saveHeightmap)(objid id, std::optional<std::string> filename);
  std::map<std::string, std::string> (*listServers)();
  std::string (*connectServer)(std::string server);
  void (*disconnectServer)();
  void (*sendMessageTcp)(std::string data);
  void (*sendMessageUdp)(std::string data);
  void (*playRecording)(objid id, std::string recordingPath, std::optional<RecordingPlaybackType> type);
  void (*stopRecording)(objid id);
  objid (*createRecording)(objid id);
  void (*saveRecording)(objid recordingId, std::string filepath);
  std::optional<objid> (*makeObjectAttr)(objid sceneId, std::string name, GameobjAttributes& attr, std::map<std::string, GameobjAttributes>& submodelAttributes);
  void (*makeParent)(objid child, objid parent);
  std::vector<HitObject> (*raycast)(glm::vec3 pos, glm::quat direction, float maxDistance);
  std::vector<HitObject> (*contactTest)(objid id);
  std::vector<HitObject> (*contactTestShape)(glm::vec3 pos, glm::quat orientation, glm::vec3 scale);
  void (*saveScreenshot)(std::string);
  void (*setState)(std::string);
  void (*setFloatState)(std::string stateName, float value);
  void (*setIntState)(std::string stateName, int value);
  glm::vec3 (*navPosition)(objid, glm::vec3 pos);
  void (*emit)(objid id, std::optional<glm::vec3> initPosition, std::optional<glm::quat> initOrientation, std::optional<glm::vec3> initVelocity, std::optional<glm::vec3> initAvelocity, std::optional<objid> parentId);
  objid (*loadAround)(objid);
  void (*rmLoadAround)(objid);
  void (*generateMesh)(std::vector<glm::vec3> face, std::vector<glm::vec3> points, std::string);
  std::map<std::string, std::string> (*getArgs)();
  bool (*lock)(std::string, objid);
  bool (*unlock)(std::string, objid);
  void (*debugInfo)(std::string infoType, std::string filepath);
  void (*setWorldState)(std::vector<ObjectValue> values);
  std::vector<ObjectValue> (*getWorldState)();
  void (*setLayerState)(std::vector<StrValues> values);

  unsigned int  (*createTexture)(std::string name, unsigned int width, unsigned int height, objid ownerId);
  void (*freeTexture)(std::string name, objid ownerId);
  void (*clearTexture)(unsigned int textureId, std::optional<bool> autoclear, std::optional<glm::vec4> color, std::optional<std::string> texture);
  AttributeValue (*runStats)(std::string& field);
  unsigned int (*stat)(std::string);
  void (*logStat)(unsigned int, AttributeValue amount);
  void (*installMod)(std::string layer);
  void (*uninstallMod)(std::string layer);
  std::vector<std::string> (*listMods)();

  sql::SqlQuery (*compileSqlQuery)(std::string queryString, std::vector<std::string> bindValues);
  std::vector<std::vector<std::string>> (*executeSqlQuery)(sql::SqlQuery& query, bool* valid);
  std::vector<objid> (*selected)();
  void (*setSelected)(std::optional<std::set<objid>> id);

  void (*click)(int button, int action);
  void (*moveMouse)(glm::vec2 ndi);
  void (*schedule)(objid id, float delayTimeMs, void* data, std::function<void(void*)> fn);
  FrameInfo (*getFrameInfo)();
  RotationDirection (*getCursorInfoWorld)(float ndix, float ndiy);
  std::optional<objid> (*idAtCoord)(float ndix, float ndiy, bool onlyGameObjId);
  bool (*gameobjExists)(objid id);
  std::optional<objid> (*prefabId)(objid id);
  //std::vector<func_t> registerGuileFns
};


struct CScriptBinding {
  std::string bindingMatcher;
  CustomApiBindings& api;
  std::function<void*(std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript)> create;
  std::function<void(std::string scriptname, objid id, void*)> remove;

  // Other callbacks
  id_func_data onFrame;
  id_colposfun onCollisionEnter;
  id_colfun onCollisionExit;
  id_mousecallback onMouseCallback;
  id_mousemovecallback onMouseMoveCallback;
  id_scrollcallback onScrollCallback;
  id_onobjectSelectedFunc onObjectSelected;
  id_func_data onObjectUnselected;
  id_onobjectHoverFunc onObjectHover;
  id_funcMappingFunc onMapping;
  id_keycallback onKeyCallback;
  id_keycharcallback onKeyCharCallback;
  id_stringboolFunc onCameraSystemChange;
  id_string2func onMessage;
  id_stringfunc onTcpMessage;
  id_stringfunc onUdpMessage;
  id_stringfunc onPlayerJoined;
  id_stringfunc onPlayerLeave;
  std::function<void(objid, void*, objid)> onObjectAdded;
  std::function<void(objid, void*, objid)> onObjectRemoved;
  std::function<void(void*)> render;
};

CScriptBinding createCScriptBinding(const char* bindingMatcher, CustomApiBindings& api);

#endif