#ifndef MOD_SCHEMEBINDINGS
#define MOD_SCHEMEBINDINGS

#include <iostream>
#include <map>
#include <glm/gtx/compatibility.hpp>
#include "./scriptmanager.h"      // need to eliminate the circular relationship here
#include "./scheme_util.h"
#include "../../../common/util.h"
#include "../../../sql/sql.h"  // ideally move this away from being a direct dependency

void createStaticSchemeBindings(
  int32_t (*listSceneId)(int32_t objid),
  int32_t (*loadScene)(std::string sceneFile, std::vector<std::vector<std::string>> additionalTokens, std::optional<std::string> name, std::optional<std::vector<std::string>> tags),
  void (*unloadScene)(int32_t id),  
  void (*unloadAllScenes)(),
  void (*resetScene)(std::optional<objid> sceneId),
  std::vector<int32_t> (*listScenes)(std::optional<std::vector<std::string>> tags),  
  std::vector<std::string> (*listSceneFiles)(std::optional<objid> sceneId),
  bool (*parentScene)(objid sceneId, objid* _parentSceneId),
  std::vector<objid> (*childScenes)(objid sceneId),
  std::optional<objid> (*sceneIdByName)(std::string name),
  objid (*rootIdForScene)(objid sceneId),
  std::vector<ScenegraphDebug> (*_scenegraph)(),
  void (*sendLoadScene)(int32_t id),
  void (*createScene)(std::string scenename),
  void (*deleteScene)(std::string scenename),
  void (*moveCamera)(glm::vec3, std::optional<bool> relative),  
  void (*rotateCamera)(float xoffset, float yoffset),
  void (*removeObjectById)(int32_t id),
  std::vector<int32_t> (*getObjectsByType)(std::string),
  std::vector<int32_t> (*getObjectsByAttr)(std::string, std::optional<AttributeValue>, std::optional<int32_t>),
  void (*setActiveCamera)(int32_t cameraId, float interpolationTime),
  void (*drawText)(std::string word, float left, float top, unsigned int fontSize, bool permatext, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<std::string> fontFamily, std::optional<objid> selectionId),
  void (*drawRect)(float centerX, float centerY, float width, float height, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId),
  int32_t (*drawLine)(glm::vec3 posFrom, glm::vec3 posTo, bool permaline, objid owner, std::optional<glm::vec4> color, std::optional<unsigned int> textureId, std::optional<unsigned int> linewidth),
  void (*freeLine)(int32_t lineid),
  std::optional<std::string> (*getGameObjNameForId)(int32_t id),
  GameobjAttributes getGameObjectAttr(int32_t id),
  void (*setGameObjectAttr)(int32_t id, GameobjAttributes& attr),
  glm::vec3 (*getGameObjectPos)(int32_t index, bool world),
  void (*setGameObjectPosition)(int32_t index, glm::vec3 pos, bool world),
  glm::quat (*getGameObjectRotation)(int32_t index, bool world),
  void (*setGameObjectRot)(int32_t index, glm::quat rotation, bool world),
  void (*setGameObjectScale)(int32_t index, glm::vec3 scale, bool world),
  glm::quat (*setFrontDelta)(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta),
  glm::vec3 (*moveRelative)(glm::vec3 pos, glm::quat orientation, float distance),
  glm::vec3 (*moveRelativeVec)(glm::vec3 posFrom, glm::quat orientation, glm::vec3 vec),
  glm::quat (*orientationFromPos)(glm::vec3 fromPos, glm::vec3 toPos),
  std::optional<objid> (*getGameObjectByName)(std::string name, objid sceneId, bool sceneIdExplicit),
  void (*applyImpulse)(int32_t index, glm::vec3 impulse),
  void (*applyImpulseRel)(int32_t index, glm::vec3 impulse),
  void (*clearImpulse)(int32_t index),
  std::vector<std::string> (*listAnimations)(int32_t id),
  void playAnimation(int32_t id, std::string animationToPlay, bool loop),
  std::vector<std::string>(*listClips)(),
  void (*playClip)(std::string, objid sceneId, std::optional<float> volume, std::optional<glm::vec3> position),
  std::vector<std::string> (*listResources)(std::string),
  void (*sendNotifyMessage)(std::string message, AttributeValue value),
  double (*timeSeconds)(bool realtime),
  double (*timeElapsed)(),
  bool (*saveScene)(bool includeIds, objid sceneId, std::optional<std::string> filename),
  void (*saveHeightmap)(objid id, std::optional<std::string> filename),
  std::map<std::string, std::string> (*listServers)(),
  std::string (*connectServer)(std::string server),
  void (*disconnectServer)(),
  void (*sendMessageTcp)(std::string data),
  void (*sendMessageUdp)(std::string data),
  void (*playRecording)(objid id, std::string recordingPath, std::optional<RecordingPlaybackType> type),
  void (*stopRecording)(objid id),
  objid (*createRecording)(objid id),
  void (*saveRecording)(objid recordingId, std::string filepath),
  std::optional<objid> (*makeObjectAttr)(objid sceneId, std::string name, GameobjAttributes& attr, std::map<std::string, GameobjAttributes>& submodelAttributes),
  void (*makeParent)(objid child, objid parent),
  std::vector<HitObject> (*raycast)(glm::vec3 pos, glm::quat direction, float maxDistance),
  std::vector<HitObject> (*contactTest)(objid id),
  void (*saveScreenshot)(std::string),
  void (*setState)(std::string),
  void (*setFloatState)(std::string stateName, float value),
  void (*setIntState)(std::string stateName, int value),
  glm::vec3 (*navPosition)(objid, glm::vec3 pos),
  void (*emit)(objid id, std::optional<glm::vec3> initPosition, std::optional<glm::quat> initOrientation, std::optional<glm::vec3> initVelocity, std::optional<glm::vec3> initAvelocity, std::optional<objid> parentId),
  objid (*loadAround)(objid),
  void (*rmLoadAround)(objid),
  void (*generateMesh)(std::vector<glm::vec3> face, std::vector<glm::vec3> points, std::string),
  std::map<std::string, std::string> (*getArgs)(),
  bool (*lock)(std::string, objid),
  bool (*unlock)(std::string, objid),
  void (*debugInfo)(std::string infoType, std::string filepath),
  void (*setWorldState)(std::vector<ObjectValue> values),
  std::vector<ObjectValue> (*getWorldState)(),
  void (*setLayerState)(std::vector<StrValues> values),
  void (*enforceLayout)(objid layoutId),
  unsigned int  (*createTexture)(std::string name, unsigned int width, unsigned int height, objid ownerId),
  void (*freeTexture)(std::string name, objid ownerId),
  void (*clearTexture)(unsigned int textureId, std::optional<bool> autoclear, std::optional<glm::vec4> color, std::optional<std::string> texture),
  AttributeValue (*runStats)(std::string& field),
  unsigned int (*scmStat)(std::string),
  void (*logStat)(unsigned int, AttributeValue amount),
  void (*installMod)(std::string layer),
  void (*uninstallMod)(std::string layer),
  std::vector<std::string> (*listMods)(),
  sql::SqlQuery (*compileSqlQuery)(std::string queryString, std::vector<std::string> bindValues),
  std::vector<std::vector<std::string>> (*executeSqlQuery)(sql::SqlQuery& query, bool* valid),
  std::vector<objid> (*selected)(),
  void (*click)(int button, int action),
  void (*moveMouse)(glm::vec2 ndi),
  std::vector<func_t> registerGuileFns
);

void defineFunctions(objid id, bool isServer, bool isFreeScript);

void onFrame();
void onCollisionEnter(int32_t obj1, glm::vec3 contactPos, glm::vec3 normal);
void onCollisionExit(int32_t obj1);
void onGlobalCollisionEnter(int32_t obj1, int32_t obj2, glm::vec3 contactPos, glm::vec3 normal, glm::vec3 oppositeNormal);
void onGlobalCollisionExit(int32_t obj1, int32_t obj2);
void onMouseCallback(int button, int action, int mods);
void onMouseMoveCallback(double xPos, double yPos, float xNdc, float yNdc);
void onScrollCallback(double amount);
void onObjectSelected(int32_t index, glm::vec3 color);
void onObjectUnselected();
void onObjectHover(int32_t index);
void onObjectUnhover(int32_t index);
void onMapping(int32_t index);
void onKeyCallback(int key, int scancode, int action, int mods);
void onKeyCharCallback(unsigned int codepoint);
void onCameraSystemChange(std::string camera, bool usingBuiltInCamera);
void onAttrMessage(std::string message, AttributeValue value);
void onTcpMessage(std::string message);
void onUdpMessage(std::string message);
void onPlayerJoined(std::string connectionHash);
void onPlayerLeave(std::string connectionHash);
void onObjectAdded(objid idAdded);
void onObjectRemoved(objid idRemoved);
void onScriptUnload();

#endif