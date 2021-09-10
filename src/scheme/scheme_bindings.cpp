#include "./scheme_bindings.h"

int32_t (*_listSceneId)(int32_t objid);

// Main Api
int32_t (*_loadScene)(std::string);
SCM scm_loadScene(SCM value){
  auto sceneId = _loadScene(scm_to_locale_string(value));
  return scm_from_int32(sceneId);
}

void (*_unloadScene)(int32_t id);
SCM scm_unloadScene(SCM value){
  _unloadScene(scm_to_int32(value));
  return SCM_UNSPECIFIED;
}
void (*_unloadAllScenes)();
SCM scm_unloadAllScenes(){
  _unloadAllScenes();
  return SCM_UNSPECIFIED;
}

std::vector<int32_t> (*_listScenes)();
SCM scm_listScenes(){
  auto scenes = _listScenes();
  SCM list = scm_make_list(scm_from_unsigned_integer(scenes.size()), scm_from_unsigned_integer(0));
  for (int i = 0; i < scenes.size(); i++){
    scm_list_set_x (list, scm_from_unsigned_integer(i), scm_from_int32(scenes.at(i))); 
  }
  return list;
}

void (*_createScene)(std::string scenename);
SCM scm_createScene(SCM filepath){
  _createScene(scm_to_locale_string(filepath));
  return SCM_UNSPECIFIED;
}

std::vector<std::string> (*_listSceneFiles)();
SCM scm_listSceneFiles(){
  auto scenes = _listSceneFiles();
  SCM list = scm_make_list(scm_from_unsigned_integer(scenes.size()), scm_from_unsigned_integer(0));
  for (int i = 0; i < scenes.size(); i++){
    scm_list_set_x (list, scm_from_unsigned_integer(i), scm_from_locale_string(scenes.at(i).c_str())); 
  }
  return list;
}

void (*_sendLoadScene)(int32_t id);
SCM scm_sendLoadScene(SCM sceneId){
  _sendLoadScene(scm_to_int32(sceneId));
  return SCM_UNSPECIFIED;
}

void (*selectionMode)(bool enabled);
SCM setSelectionMod(SCM value){
  selectionMode(scm_to_bool(value));
  return SCM_UNSPECIFIED;
}

void (*setActCamera)(int32_t id);
SCM setActiveCam(SCM value){
  setActCamera(scm_to_int32(value));
  return SCM_UNSPECIFIED;
}

void (*moveCam)(glm::vec3);
SCM scmMoveCamera(SCM arg1, SCM arg2, SCM arg3){
  moveCam(glm::vec3(scm_to_double(arg1), scm_to_double(arg2), scm_to_double(arg3)));
  return SCM_UNSPECIFIED;
}

void (*rotateCam)(float, float);
SCM scmRotateCamera(SCM xoffset, SCM yoffset){
  rotateCam(scm_to_double(xoffset), scm_to_double(yoffset));
  return SCM_UNSPECIFIED;
}

objid (*_makeObjectAttr)(
  objid sceneId,
  std::string name, 
  std::map<std::string, std::string> stringAttributes,
  std::map<std::string, double> numAttributes,
  std::map<std::string, glm::vec3> vecAttributes
);
SCM scmMakeObjectAttr(SCM scmName, SCM scmAttributes){
  auto sceneId = _listSceneId(currentModuleId());
  auto attrs = scmToAttributes(scmAttributes);
  objid id = _makeObjectAttr(sceneId, scm_to_locale_string(scmName), attrs.stringAttributes, attrs.numAttributes, attrs.vecAttributes);
  return scm_from_int32(id);
}

void (*_makeParent)(objid child, objid parent);
SCM scmMakeParent(SCM childId, SCM parentId){
  _makeParent(scm_to_int32(childId), scm_to_int32(parentId));
  return SCM_UNSPECIFIED;
}

void (*removeObjById)(int32_t id);
SCM removeObject(SCM value){
  removeObjById(scm_to_int32(value));
  return SCM_UNSPECIFIED;
}


void (*_drawText)(std::string word, float left, float top, unsigned int fontSize);
SCM scmDrawText(SCM word, SCM left, SCM top, SCM fontSize){
  _drawText(
    scm_to_locale_string(word), 
    scm_to_double(left), 
    scm_to_double(top), 
    toUnsignedInt(fontSize)
  );
  return SCM_UNSPECIFIED;
}

void (*_drawLine)(glm::vec3 posFrom, glm::vec3 posTo);
SCM scmDrawLine(SCM posFrom, SCM posTo){
  _drawLine(listToVec3(posFrom), listToVec3(posTo));
  return SCM_UNSPECIFIED;
}

struct gameObject {
  int32_t id;
};
SCM gameObjectType;   // this is modified during init
SCM createGameObject(int32_t id){
  auto obj = (gameObject *)scm_gc_malloc(sizeof(gameObject), "gameobj");
  obj->id = id;
  return scm_make_foreign_object_1(gameObjectType, obj); 
}

std::vector<int32_t> (*getObjByType)(std::string);
SCM lsObjectsByType(SCM value){
  std::string objectType = scm_to_locale_string(value);
  std::vector indexes = getObjByType(objectType);
  SCM list = scm_make_list(scm_from_unsigned_integer(indexes.size()), scm_from_unsigned_integer(0));
  
  for (unsigned int i = 0; i < indexes.size(); i++){
    auto obj = (gameObject *)scm_gc_malloc(sizeof(gameObject), "gameobj");
    obj->id = indexes[i];
    scm_list_set_x (list, scm_from_unsigned_integer(i),  scm_make_foreign_object_1(gameObjectType, obj));
  }
  return list;
}

int32_t getGameobjId(SCM value){
  gameObject *obj;
  scm_assert_foreign_object_type (gameObjectType, value);
  obj = (gameObject*)scm_foreign_object_ref (value, 0);
  return obj->id;  
}

std::string (*getGameObjNameForId)(int32_t id);
SCM getGameObjNameForIdFn(SCM value){
  return scm_from_locale_string(getGameObjNameForId(getGameobjId(value)).c_str());
}

SCM scmGetGameObjectById(SCM scmId){
  auto id = scm_to_int32(scmId);
  getGameObjNameForId(id);          // this assert the object exists
  return createGameObject(id);
}

GameobjAttributes (*_getGameObjectAttr)(int32_t id);
SCM scmGetGameObjectAttr(SCM gameobj){
  auto attr = _getGameObjectAttr(getGameobjId(gameobj));
  int totalSize = attr.stringAttributes.size() + attr.numAttributes.size() + attr.vecAttributes.size();
  SCM list = scm_make_list(scm_from_unsigned_integer(totalSize), scm_from_unsigned_integer(0));

  int index = 0;
  for (auto &[key, value] : attr.stringAttributes){
    SCM attributePair = scm_make_list(scm_from_unsigned_integer(2), scm_from_unsigned_integer(0));
    scm_list_set_x(attributePair, scm_from_unsigned_integer(0), scm_from_locale_string(key.c_str()));
    scm_list_set_x(attributePair, scm_from_unsigned_integer(1), scm_from_locale_string(value.c_str()));
    scm_list_set_x(list, scm_from_unsigned_integer(index), attributePair);
    index++;   
  }
  for (auto &[key, value] : attr.numAttributes){
    SCM attributePair = scm_make_list(scm_from_unsigned_integer(2), scm_from_unsigned_integer(0));
    scm_list_set_x(attributePair, scm_from_unsigned_integer(0), scm_from_locale_string(key.c_str()));
    scm_list_set_x(attributePair, scm_from_unsigned_integer(1), scm_from_double(value));
    scm_list_set_x(list, scm_from_unsigned_integer(index), attributePair);
    index++;   
  }
  for (auto &[key, value] : attr.vecAttributes){
    SCM attributePair = scm_make_list(scm_from_unsigned_integer(2), scm_from_unsigned_integer(0));
    scm_list_set_x(attributePair, scm_from_unsigned_integer(0), scm_from_locale_string(key.c_str()));
    scm_list_set_x(attributePair, scm_from_unsigned_integer(1), vec3ToScmList(value));
    scm_list_set_x(list, scm_from_unsigned_integer(index), attributePair);
    index++;   
  }
  return list;
}

// @TODO -> add types around this
void (*_setGameObjectAttr)(int32_t id, GameobjAttributes& attr);
SCM scmSetGameObjectAttr(SCM gameobj, SCM attr){
  auto id = getGameobjId(gameobj);
  auto attrs = scmToAttributes(attr);
  _setGameObjectAttr(id, attrs);
  return SCM_UNSPECIFIED;
}

glm::vec3 (*_getGameObjectPos)(int32_t index, bool isWorld);
SCM scmGetGameObjectPos(SCM value){
  return vec3ToScmList(_getGameObjectPos(getGameobjId(value), false));
}
SCM scmGetGameObjectPosWorld(SCM value){
  return vec3ToScmList(_getGameObjectPos(getGameobjId(value), true));
}

void (*setGameObjectPosn)(int32_t index, glm::vec3 pos);
SCM setGameObjectPosition(SCM value, SCM positon){
  auto x = scm_to_double(scm_list_ref(positon, scm_from_int64(0)));   
  auto y = scm_to_double(scm_list_ref(positon, scm_from_int64(1)));
  auto z = scm_to_double(scm_list_ref(positon, scm_from_int64(2)));
  setGameObjectPosn(getGameobjId(value), glm::vec3(x, y, z));
  return SCM_UNSPECIFIED;
}

void (*setGameObjectPosnRel)(int32_t index, float x, float y, float z, bool xzPlaneOnly);
void setPosition(SCM value, SCM position, bool xzOnly){
  auto x = scm_to_double(scm_list_ref(position, scm_from_int64(0)));   
  auto y = scm_to_double(scm_list_ref(position, scm_from_int64(1)));
  auto z = scm_to_double(scm_list_ref(position, scm_from_int64(2)));
  setGameObjectPosnRel(getGameobjId(value), x, y, z, xzOnly);
}
SCM setGameObjectPositionRel(SCM value, SCM position){
  setPosition(value, position, false);
  return SCM_UNSPECIFIED;
}
SCM setGameObjectPositionRelXZ(SCM value, SCM position){
  setPosition(value, position, true);
  return SCM_UNSPECIFIED;
}

glm::quat (*_getGameObjectRotation)(int32_t index, bool world);
SCM scmGetGameObjectRotation(SCM value){
  return scmQuatToSCM(_getGameObjectRotation(getGameobjId(value), false));
}
SCM getGameObjectRotationWorld(SCM value){
  return scmQuatToSCM(_getGameObjectRotation(getGameobjId(value), true));
}
void (*setGameObjectRotn)(int32_t index, glm::quat rotation);
SCM setGameObjectRotation(SCM value, SCM rotation){
  setGameObjectRotn(getGameobjId(value), scmListToQuat(rotation));
  return SCM_UNSPECIFIED;
}

glm::quat (*_setFrontDelta)(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta);
SCM setGameObjectRotationDelta(SCM value, SCM deltaYaw, SCM deltaPitch, SCM deltaRoll){
  int32_t id = getGameobjId(value);
  glm::quat rot = _getGameObjectRotation(id, false);

  auto deltaY = scm_to_double(deltaYaw);
  auto deltaP = scm_to_double(deltaPitch);
  auto deltaR = scm_to_double(deltaRoll); 

  glm::quat newOrientation = _setFrontDelta(rot, deltaY, deltaP, deltaR, 0.1);
  setGameObjectRotn(id, newOrientation);

  return SCM_UNSPECIFIED;
}
SCM scmSetFrontDelta(SCM orientation, SCM deltaYaw, SCM deltaPitch, SCM deltaRoll){
  glm::quat intialOrientation = scmListToQuat(orientation);
  auto deltaY = scm_to_double(deltaYaw);
  auto deltaP = scm_to_double(deltaPitch);
  auto deltaR = scm_to_double(deltaRoll); 
  return scmQuatToSCM(_setFrontDelta(intialOrientation, deltaY, deltaP, deltaR, 1));
}

glm::vec3 (*_moveRelative)(glm::vec3 pos, glm::quat orientation, float distance);
SCM scmMoveRelative(SCM pos, SCM orientation, SCM distance){
  return vec3ToScmList(_moveRelative(listToVec3(pos), scmListToQuat(orientation), scm_to_double(distance)));
}

glm::quat (*_orientationFromPos)(glm::vec3 fromPos, glm::vec3 toPos);
SCM scmOrientationFromPos(SCM fromPos, SCM toPos){
  return scmQuatToSCM(_orientationFromPos(listToVec3(fromPos), listToVec3(toPos)));
}

std::vector<std::string>(*_listAnimations)(int32_t id);
SCM scmListAnimations(SCM value){
  auto animations = _listAnimations(getGameobjId(value));
  int numAnimations = animations.size();

  SCM list = scm_make_list(scm_from_unsigned_integer(numAnimations), scm_from_unsigned_integer(0));
  for (int i = 0; i < numAnimations; i++){
    scm_list_set_x (list, scm_from_unsigned_integer(i), scm_from_locale_string(animations.at(i).c_str()));
  }

  return list;
}

void (*_playAnimation)(int32_t id, std::string animationName);
SCM scmPlayAnimation(SCM value, SCM animationName){
  _playAnimation(getGameobjId(value), scm_to_locale_string(animationName));
  return SCM_UNSPECIFIED;
}

void (*_applyImpulse)(int32_t index, glm::vec3 impulse);
SCM scm_applyImpulse(SCM value, SCM impulse){
  _applyImpulse(getGameobjId(value), listToVec3(impulse));
  return SCM_UNSPECIFIED;
}
void (*_applyImpulseRel)(int32_t index, glm::vec3 impulse);
SCM scm_applyImpulseRel(SCM value, SCM impulse){
  _applyImpulseRel(getGameobjId(value), listToVec3(impulse));
  return SCM_UNSPECIFIED;
}

void (*_clearImpulse)(int32_t index);
SCM scm_clearImpulse(SCM value){
  _clearImpulse(getGameobjId(value));
  return SCM_UNSPECIFIED;
}

SCM getGameObjectId(SCM value){
  return scm_from_int32(getGameobjId(value));
}

std::optional<objid> (*_getGameObjectByName)(std::string name, objid sceneId);
SCM getGameObjByName(SCM value, SCM scmSceneId){
  auto sceneIdDefined = !scm_is_eq(scmSceneId, SCM_UNDEFINED);
  auto sceneId = sceneIdDefined ? scm_to_int32(scmSceneId) : _listSceneId(currentModuleId());
  auto id = _getGameObjectByName(scm_to_locale_string(value), sceneId);
  if (!id.has_value()){
    return scm_from_bool(false);
  }
  return createGameObject(id.value());
}

std::vector<std::string> (*_listClips)();
SCM scmListClips(){
  auto clips = _listClips();
  SCM list = scm_make_list(scm_from_unsigned_integer(clips.size()), scm_from_unsigned_integer(0));
  for (int i = 0; i < clips.size(); i++){
    scm_list_set_x (list, scm_from_unsigned_integer(i), scm_from_locale_string(clips.at(i).c_str()));
  }
  return list;
}
void (*_playClip)(std::string, objid);
SCM scmPlayClip(SCM soundname){
  auto sceneId = _listSceneId(currentModuleId());
  _playClip(scm_to_locale_string(soundname), sceneId);
  return SCM_UNSPECIFIED;
}

std::vector<std::string> (*_listModels)();
SCM scmListModels(){
  std::vector<std::string> models = _listModels();
  SCM list = scm_make_list(scm_from_unsigned_integer(models.size()), scm_from_unsigned_integer(0));
  for (int i = 0; i < models.size(); i++){
    scm_list_set_x(list, scm_from_unsigned_integer(i), scm_from_locale_string(models.at(i).c_str()));
  }
  return list;
}

void (*_sendEventMessage)(std::string message);
SCM scmSendEventMessage(SCM channelFrom){
  _sendEventMessage(scm_to_locale_string(channelFrom));
  return SCM_UNSPECIFIED;
}

void (*_sendNotifyMessage)(std::string message, std::string value);
SCM scmSendNotify(SCM topic, SCM value){
  _sendNotifyMessage(scm_to_locale_string(topic), scm_to_locale_string(value));
  return SCM_UNSPECIFIED;
}

double (*_timeSeconds)();
SCM scmTimeSeconds(){
  return scm_from_double(_timeSeconds());
}
void (*_saveScene)(bool includeIds, objid sceneId); 
SCM scmSaveScene(SCM scmIncludeIds, SCM scmSceneId){
  auto includeIds = scmIncludeIds == SCM_UNDEFINED ? false : scm_to_bool(scmIncludeIds);
  auto sceneId = scmSceneId == SCM_UNDEFINED ? currentSceneId() : scm_to_int32(scmSceneId);
  _saveScene(includeIds, sceneId);
  return SCM_UNSPECIFIED;
}

void (*_sendMessageTcp)(std::string data);
SCM scmSendMessageTcp(SCM topic){
  _sendMessageTcp(scm_to_locale_string(topic));
  return SCM_UNSPECIFIED;
}
void (*_sendMessageUdp)(std::string data);
SCM scmSendMessageUdp(SCM topic){
  _sendMessageUdp(scm_to_locale_string(topic));
  return SCM_UNSPECIFIED;
}

std::map<std::string, std::string> (*_listServers)();
SCM scmListServers(){
  auto servers = _listServers();
  SCM list = scm_make_list(scm_from_unsigned_integer(servers.size()), scm_from_unsigned_integer(0));
  int index = 0;
  for (auto [serverName, _] : servers){
    scm_list_set_x (list, scm_from_unsigned_integer(index), scm_from_locale_string(serverName.c_str()));
    index = index + 1;
  }
  return list;
}
std::string (*_connectServer)(std::string server);
SCM scmConnectServer(SCM server){
  return scm_from_locale_string(_connectServer(scm_to_locale_string(server)).c_str());
}
void (*_disconnectServer)();
SCM scmDisconnectServer(){
  _disconnectServer();
  return SCM_UNSPECIFIED;
}

void (*_playRecording)(objid id, std::string recordingPath);
SCM scmPlayRecording(SCM id, SCM recordingPath){
  _playRecording(scm_to_int32(id), scm_to_locale_string(recordingPath));
  return SCM_UNSPECIFIED;
}
void (*_stopRecording)(objid id, std::string recordingPath);
SCM scmStopRecording(SCM id, SCM recordingPath){
  _stopRecording(scm_to_int32(id), scm_to_locale_string(recordingPath));
  return SCM_UNSPECIFIED;
}

objid (*_createRecording)(objid id);
SCM scmCreateRecording(SCM objid){
  return scm_from_int32(_createRecording(scm_to_int32(objid)));
}
void (*_saveRecording)(objid recordingId, std::string filepath);
SCM scmSaveRecording(SCM recordingId, SCM filepath){
  _saveRecording(scm_to_int32(recordingId), scm_to_locale_string(filepath));
  return SCM_UNSPECIFIED;
}

std::vector<HitObject> (*_raycast)(glm::vec3 pos, glm::quat direction, float maxDistance);
SCM scmRaycast(SCM pos, SCM direction, SCM distance){
  auto hitObjects = _raycast(listToVec3(pos), scmListToQuat(direction), scm_to_double(distance));
  SCM list = scm_make_list(scm_from_unsigned_integer(hitObjects.size()), scm_from_unsigned_integer(0));
  for (int i = 0; i < hitObjects.size(); i++){
    auto hitObject = hitObjects.at(i);
    auto idAndHitPoint = scm_make_list(scm_from_unsigned_integer(3), scm_from_unsigned_integer(0));
    scm_list_set_x(idAndHitPoint, scm_from_unsigned_integer(0), scm_from_int32(hitObject.id));
    scm_list_set_x(idAndHitPoint, scm_from_unsigned_integer(1), vec3ToScmList(hitObject.point));
    scm_list_set_x(idAndHitPoint, scm_from_unsigned_integer(2), scmQuatToSCM(hitObject.normal));
    scm_list_set_x(list, scm_from_unsigned_integer(i), idAndHitPoint);
  }
  return list;
}

void (*_saveScreenshot)(std::string filepath);
SCM scmSaveScreenshot(SCM filepath){
  _saveScreenshot(scm_to_locale_string(filepath));
  return SCM_UNSPECIFIED;
}

void (*_setState)(std::string);
void (*_setIntState)(std::string, int);
void (*_setFloatState)(std::string, float);
SCM scmSetState(SCM value, SCM value2){
  if (!scm_is_eq (value2, SCM_UNDEFINED)){
    bool isInt = scm_is_exact_integer(value2);
    bool isNumber = scm_is_number(value2);
    assert(isInt || isNumber);
    if (isInt){
      _setIntState(scm_to_locale_string(value), scm_to_int32(value2));
    }else if (isNumber){
      _setFloatState(scm_to_locale_string(value), scm_to_double(value2));
    }
  }else{
    _setState(scm_to_locale_string(value));
  }
  return SCM_UNSPECIFIED;
}

glm::vec3 (*_navPosition)(objid, glm::vec3 pos);
SCM scmNavPosition(SCM obj, SCM pos){
  return vec3ToScmList(_navPosition(scm_to_int32(obj), listToVec3(pos)));
}

void (*_scmEmit)(objid id);
SCM scmEmit(SCM gameobjId){
  _scmEmit(scm_to_int32(gameobjId));
  return SCM_UNSPECIFIED;
}

objid (*_loadAround)(objid);
SCM scmLoadAround(SCM gameobjId){
  return scm_from_short(_loadAround(scm_to_int32(gameobjId)));
}
void (*_rmLoadAround)(objid);
SCM scmRmLoadAround(SCM loadingHandle){
  _rmLoadAround(scm_to_int32(loadingHandle));
  return SCM_UNSPECIFIED;
}

void (*_generateMesh)(std::vector<glm::vec3>, std::vector<glm::vec3>, std::string);
SCM scmGenerateMesh(SCM face, SCM points, SCM meshname){
  _generateMesh(listToVecVec3(face), listToVecVec3(points), scm_to_locale_string(meshname));
  return SCM_UNSPECIFIED;
}

void (*_setSkybox)(std::string);
SCM scmSetSkybox(SCM skybox){
  _setSkybox(scm_to_locale_string(skybox));
  return SCM_UNSPECIFIED;
}

std::map<std::string, std::string> (*_getArgs)();
SCM scmArgs(SCM argKey){
  auto args = _getArgs();
  auto key = scm_to_locale_string(argKey);
  if (args.find(key) == args.end()){
    return SCM_BOOL_F;
  }
  auto value = args.at(key);
  if (value == ""){
    return SCM_BOOL_T;
  }
  return scm_from_locale_string(value.c_str());
}

bool (*_lock)(std::string, objid);
SCM scmLock(SCM key){
  auto lockSuccess = _lock(scm_to_locale_string(key), currentModuleId());
  return scm_from_bool(lockSuccess);
}
bool (*_unlock)(std::string, objid);
SCM scmUnlock(SCM key){
  auto unlockSuccess = _unlock(scm_to_locale_string(key), currentModuleId());
  return scm_from_bool(unlockSuccess);
}

// Callbacks
void onFrame(){
  maybeCallFunc("onFrame");
}
void onCollisionEnter(int32_t obj1, glm::vec3 contactPos, glm::vec3 normal){
  const char* function = "onCollideEnter";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_3(func_symbol, createGameObject(obj1), vec3ToScmList(contactPos), vec3ToScmList(normal));
  }
}
void onCollisionExit(int32_t obj1){
  const char* function = "onCollideExit";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_1(func_symbol, createGameObject(obj1));
  }
}
void onMouseCallback(int button, int action, int mods){
  const char* function = "onMouse";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_3(func_symbol, scm_from_int(button), scm_from_int(action), scm_from_int(mods));
  }
}
void onMouseMoveCallback(double xPos, double yPos){
  const char* function = "onMouseMove";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_2(func_symbol, scm_from_double(xPos), scm_from_double(yPos));
  }
}
void onScrollCallback(double amount){
  const char* function = "onScroll";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_1(func_symbol, scm_from_double(amount));
  }  
}
void onObjectSelected(int32_t index, glm::vec3 color){
  const char* function = "onObjSelected";
  if (symbolDefined(function)){
    auto obj = (gameObject *)scm_gc_malloc(sizeof(gameObject), "gameobj");
    obj->id = index;
    SCM gameobject = scm_make_foreign_object_1(gameObjectType, obj);

    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_2(func_symbol, gameobject, vec3ToScmList(color));
  }
}

void callObjIdFunc(const char* function, objid index){
  if (symbolDefined(function)){
    auto obj = (gameObject *)scm_gc_malloc(sizeof(gameObject), "gameobj");
    obj->id = index;
    SCM gameobject = scm_make_foreign_object_1(gameObjectType, obj);

    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_1(func_symbol, gameobject);
  }
}
void onObjectHover(int32_t index){
  const char* function = "onObjHover";
  callObjIdFunc(function, index);
}
void onObjectUnhover(int32_t index){
  const char* function = "onObjUnhover";
  callObjIdFunc(function, index);
}
void onKeyCallback(int key, int scancode, int action, int mods){
  const char* function = "onKey";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_4(func_symbol, scm_from_int(key), scm_from_int(scancode), scm_from_int(action), scm_from_int(mods));  
  }
}
void onKeyCharCallback(unsigned int codepoint){
  const char* function = "onKeyChar";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_1(func_symbol, scm_from_unsigned_integer(codepoint));
  }
}
void onCameraSystemChange(bool usingBuiltInCamera){
  const char* function = "onCameraSystemChange";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_1(func_symbol, scm_from_bool(usingBuiltInCamera));
  }
}

void onAttrMessage(std::string message, AttributeValue value){
  const char* function = "onMessage";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));

    auto strValue = std::get_if<std::string>(&value);
    if (strValue != NULL){
      scm_call_2(func_symbol, scm_from_locale_string(message.c_str()), scm_from_locale_string(strValue -> c_str())); 
      return;
    }
    auto floatValue = std::get_if<float>(&value);
    if (floatValue != NULL){
      scm_call_2(func_symbol, scm_from_locale_string(message.c_str()), scm_from_double(*floatValue)); 
      return;   
    }
    auto vecValue = std::get_if<glm::vec3>(&value);
    if (vecValue != NULL){
      scm_call_2(func_symbol, scm_from_locale_string(message.c_str()), vec3ToScmList(*vecValue)); 
      return;
    }
    std::cout << "invalid type" << std::endl;
    assert(false);
  }  
}

void onTcpMessage(std::string message){
  const char* function = "onTcpMessage";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_1(func_symbol, scm_from_locale_string(message.c_str()));
  }
}
void onUdpMessage(std::string message){
  const char* function = "onUdpMessage";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_1(func_symbol, scm_from_locale_string(message.c_str()));
  }
}

void onPlayerJoined(std::string connectionHash){
  maybeCallFuncString("on-player-join", connectionHash.c_str());
}
void onPlayerLeave(std::string connectionHash){
  maybeCallFuncString("on-player-leave", connectionHash.c_str());
}

void onScriptUnload(){
  maybeCallFunc("beforeUnload");
}

std::vector<func_t> _registerGuileFns;
////////////
void defineFunctions(objid id, bool isServer){
  scm_c_define_gsubr("load-scene", 1, 0, 0, (void *)scm_loadScene);
  scm_c_define_gsubr("unload-scene", 1, 0, 0, (void *)scm_unloadScene);
  scm_c_define_gsubr("unload-all-scenes", 0, 0, 0, (void *)scm_unloadAllScenes);
  scm_c_define_gsubr("list-scenes", 0, 0, 0, (void *)scm_listScenes);
  scm_c_define_gsubr("list-scenefiles", 0, 0, 0, (void *)scm_listSceneFiles);
  scm_c_define_gsubr("create-scene", 1, 0, 0, (void *)scm_createScene);

  scm_c_define_gsubr("send-load-scene", 1, 0, 0, (void *)scm_sendLoadScene);

  scm_c_define_gsubr("ls-models", 0, 0, 0, (void *)scmListModels);

  scm_c_define_gsubr("set-selection-mode", 1, 0, 0, (void *)setSelectionMod);
  scm_c_define_gsubr("set-camera", 1, 0, 0, (void *)setActiveCam);    
  scm_c_define_gsubr("mov-cam", 3, 0, 0, (void *)scmMoveCamera);   // @TODO move + rotate camera can be removed since had the gameobj manipulation functions
  scm_c_define_gsubr("rot-cam", 2, 0, 0, (void *)scmRotateCamera);
  scm_c_define_gsubr("rm-obj", 1, 0, 0, (void *)removeObject);
  scm_c_define_gsubr("lsobj-type", 1, 0, 0, (void *)lsObjectsByType);
  scm_c_define_gsubr("lsobj-name", 1, 1, 0, (void *)getGameObjByName);
 
  scm_c_define_gsubr("draw-text", 4, 0, 0, (void*)scmDrawText);
  scm_c_define_gsubr("draw-line", 2, 0, 0, (void*)scmDrawLine);

  scm_c_define_gsubr("gameobj-pos", 1, 0, 0, (void *)scmGetGameObjectPos);
  scm_c_define_gsubr("gameobj-pos-world", 1, 0, 0, (void*)scmGetGameObjectPosWorld);
  scm_c_define_gsubr("gameobj-setpos!", 2, 0, 0, (void *)setGameObjectPosition);
  scm_c_define_gsubr("gameobj-setpos-rel!", 2, 0, 0, (void *)setGameObjectPositionRel);
  scm_c_define_gsubr("gameobj-setpos-relxz!", 2, 0, 0, (void *)setGameObjectPositionRelXZ);
  
  scm_c_define_gsubr("gameobj-rot", 1, 0, 0, (void *)scmGetGameObjectRotation);
  scm_c_define_gsubr("gameobj-rot-world", 1, 0, 0, (void *)getGameObjectRotationWorld);
  scm_c_define_gsubr("gameobj-setrot!", 2, 0, 0, (void *)setGameObjectRotation);
  scm_c_define_gsubr("gameobj-setrotd!", 4, 0, 0, (void *)setGameObjectRotationDelta);

  scm_c_define_gsubr("gameobj-id", 1, 0, 0, (void *)getGameObjectId);
  scm_c_define_gsubr("gameobj-name", 1, 0, 0, (void *)getGameObjNameForIdFn);
  scm_c_define_gsubr("gameobj-by-id", 1, 0, 0, (void *)scmGetGameObjectById);

  scm_c_define_gsubr("gameobj-animations", 1, 0, 0, (void *)scmListAnimations);
  scm_c_define_gsubr("gameobj-playanimation", 2, 0, 0, (void *)scmPlayAnimation);

  scm_c_define_gsubr("gameobj-attr", 1, 0, 0, (void *)scmGetGameObjectAttr);
  scm_c_define_gsubr("gameobj-setattr!", 2, 0, 0, (void *)scmSetGameObjectAttr);


  // UTILITY FUNCTIONS
  scm_c_define_gsubr("setfrontdelta", 4, 0, 0, (void*)scmSetFrontDelta);
  scm_c_define_gsubr("move-relative", 3, 0, 0, (void*)scmMoveRelative);
  scm_c_define_gsubr("orientation-from-pos", 2, 0, 0, (void*)scmOrientationFromPos);
  scm_c_define_gsubr("orientation-from-pos", 2, 0, 0, (void*)scmOrientationFromPos);
  //scm_c_define_gsubr("quat-to-euler", 1, 0, 0, (void*)scmOrientationFromPos);
  //scm_c_define_gsubr("euler-to-quat", 1, 0, 0, (void*)scmOrientationFromPos);

  // physics functions
  scm_c_define_gsubr("applyimpulse", 2, 0, 0, (void *)scm_applyImpulse);
  scm_c_define_gsubr("applyimpulse-rel", 2, 0, 0, (void *)scm_applyImpulseRel);
  scm_c_define_gsubr("clearimpulse", 1, 0, 0, (void *)scm_clearImpulse);

  // audio stuff
  scm_c_define_gsubr("lsclips", 0, 0, 0, (void*)scmListClips);
  scm_c_define_gsubr("playclip", 1, 0, 0, (void*)scmPlayClip);

  // event system
  scm_c_define_gsubr("sendmessage", 1, 0, 0, (void*)scmSendEventMessage);
  scm_c_define_gsubr("sendnotify", 2, 0, 0, (void*)scmSendNotify);

  scm_c_define_gsubr("time-seconds", 0, 0, 0, (void*)scmTimeSeconds);
  scm_c_define_gsubr("save-scene", 0, 2, 0, (void*)scmSaveScene);

  scm_c_define_gsubr("list-servers", 0, 0, 0, (void*)scmListServers);
  scm_c_define_gsubr("connect-server", 1, 0, 0, (void*)scmConnectServer);
  scm_c_define_gsubr("disconnect-server", 0, 0, 0, (void*)scmDisconnectServer);
  scm_c_define_gsubr("send-tcp", 1, 0, 0, (void*)scmSendMessageTcp);
  scm_c_define_gsubr("send-udp", 1, 0, 0, (void*)scmSendMessageUdp);

  scm_c_define_gsubr("play-recording", 2, 0, 0, (void*)scmPlayRecording);
  scm_c_define_gsubr("stop-recording", 2, 0, 0, (void*)scmStopRecording);
  scm_c_define_gsubr("create-recording", 1, 0, 0, (void*)scmCreateRecording);
  scm_c_define_gsubr("save-recording", 2, 0, 0, (void*) scmSaveRecording);

  scm_c_define_gsubr("mk-obj-attr", 2, 0, 0, (void*)scmMakeObjectAttr);
  scm_c_define_gsubr("make-parent", 2, 0, 0, (void*)scmMakeParent);

  scm_c_define_gsubr("raycast", 3, 0, 0, (void*)scmRaycast);

  scm_c_define("mainobj", createGameObject(id));
  scm_c_define("is-server", scm_from_bool(isServer));

  scm_c_define_gsubr("screenshot", 1, 0, 0, (void*)scmSaveScreenshot);
  scm_c_define_gsubr("set-state", 1, 1, 0, (void*)scmSetState);

  scm_c_define_gsubr("navpos", 2, 0, 0, (void*)scmNavPosition);

  scm_c_define_gsubr("emit", 1, 0, 0, (void*)scmEmit);

  scm_c_define_gsubr("load-around", 1, 0, 0, (void*)scmLoadAround);
  scm_c_define_gsubr("rm-load-around", 1, 0, 0, (void*)scmRmLoadAround);

  scm_c_define_gsubr("genmesh", 3, 0, 0, (void*)scmGenerateMesh);
  scm_c_define_gsubr("set-skybox", 1, 0, 0, (void*)scmSetSkybox);

  scm_c_define_gsubr("args", 1, 0, 0, (void*)scmArgs);
  scm_c_define_gsubr("lock", 1, 0, 0, (void*)scmLock);
  scm_c_define_gsubr("unlock", 1, 0, 0, (void*)scmUnlock);

  for (auto registerFn : _registerGuileFns){
    registerFn();
  }
}


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
  void (*playClip)(std::string, objid),
  std::vector<std::string> (*listModels)(),
  void (*sendEventMessage)(std::string message),
  void (*sendNotifyMessage)(std::string topic, std::string value),
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
  std::vector<func_t> registerGuileFns
){
  scm_init_guile();
  gameObjectType = scm_make_foreign_object_type(scm_from_utf8_symbol("gameobj"), scm_list_1(scm_from_utf8_symbol("data")), NULL);
  _listSceneId = listSceneId;

  _loadScene = loadScene;

  _unloadScene = unloadScene;
  _unloadAllScenes = unloadAllScenes;
  _listScenes = listScenes;
  _listSceneFiles = listSceneFiles;
  _sendLoadScene = sendLoadScene;
  _createScene = createScene;
  
  selectionMode = setSelectionMode;
	moveCam = moveCamera;
	rotateCam = rotateCamera;
	removeObjById = removeObjectById;
	getObjByType = getObjectsByType;
	setActCamera = setActiveCamera;
  
  _drawText = drawText;
  _drawLine = drawLine;

  getGameObjNameForId = getGameObjectNameForId;
  _getGameObjectAttr = getGameObjectAttr;
  _setGameObjectAttr = setGameObjectAttr;

  _getGameObjectPos = getGameObjectPos;
  setGameObjectPosn = setGameObjectPos;
  setGameObjectPosnRel = setGameObjectPosRelative;
  _getGameObjectRotation = getGameObjectRotation;
  setGameObjectRotn = setGameObjectRot;
  
  _setFrontDelta = setFrontDelta;
  _moveRelative = moveRelative;
  _orientationFromPos = orientationFromPos;  

  _applyImpulse = applyImpulse;
  _applyImpulseRel = applyImpulseRel;
  _clearImpulse = clearImpulse;


  _getGameObjectByName = getGameObjectByName;
  _listAnimations = listAnimations;
  _playAnimation = playAnimation;
  _listClips = listClips;
  _playClip = playClip;
  _listModels = listModels;

  _sendEventMessage = sendEventMessage;
  _sendNotifyMessage = sendNotifyMessage;

  _timeSeconds = timeSeconds;
  _saveScene = saveScene;

  _listServers = listServers;
  _connectServer = connectServer;
  _disconnectServer = disconnectServer;
  _sendMessageTcp = sendMessageTcp;
  _sendMessageUdp = sendMessageUdp;

  _playRecording = playRecording;
  _stopRecording = stopRecording;
  _createRecording = createRecording;
  _saveRecording = saveRecording;

  _makeObjectAttr = makeObjectAttr;
  _makeParent = makeParent;

  _raycast = raycast;

  _saveScreenshot = saveScreenshot;
  _setState = setState;
  _setFloatState = setFloatState;
  _setIntState = setIntState;

  _navPosition = navPosition;
  _scmEmit = scmEmit;
  _loadAround = loadAround;
  _rmLoadAround = rmLoadAround;
  _generateMesh = generateMesh;

  _setSkybox = setSkybox;
  _getArgs = getArgs;
  _lock = lock;
  _unlock = unlock;

  _registerGuileFns = registerGuileFns;
}
