#include "./scheme_bindings.h"

unsigned int toUnsignedInt(SCM value){
  return scm_to_unsigned_integer(value, std::numeric_limits<unsigned int>::min(), std::numeric_limits<unsigned int>::max());
}
glm::quat scmListToQuat(SCM rotation){
  auto w = scm_to_double(scm_list_ref(rotation, scm_from_int64(0)));  
  auto x = scm_to_double(scm_list_ref(rotation, scm_from_int64(1)));
  auto y = scm_to_double(scm_list_ref(rotation, scm_from_int64(2)));
  auto z = scm_to_double(scm_list_ref(rotation, scm_from_int64(3)));  
  return  glm::quat(w, x, y, z);
}
SCM scmQuatToSCM(glm::quat rotation){
  SCM list = scm_make_list(scm_from_unsigned_integer(4), scm_from_unsigned_integer(0));
  scm_list_set_x (list, scm_from_unsigned_integer(0), scm_from_double(rotation.w));
  scm_list_set_x (list, scm_from_unsigned_integer(1), scm_from_double(rotation.x));
  scm_list_set_x (list, scm_from_unsigned_integer(2), scm_from_double(rotation.y));
  scm_list_set_x (list, scm_from_unsigned_integer(3), scm_from_double(rotation.z));
  return list;
}

glm::vec3 listToVec3(SCM vecList){
  auto x = scm_to_double(scm_list_ref(vecList, scm_from_int64(0)));   
  auto y = scm_to_double(scm_list_ref(vecList, scm_from_int64(1)));
  auto z = scm_to_double(scm_list_ref(vecList, scm_from_int64(2))); 
  return glm::vec3(x, y, z);
}

bool symbolDefinedInModule(const char* symbol, SCM module){
  return scm_to_bool(scm_defined_p(scm_string_to_symbol(scm_from_locale_string(symbol)), module));
}
bool symbolDefined(const char* symbol){
  return symbolDefinedInModule(symbol, scm_current_module());
}
void maybeCallFunc(const char* function){
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_0(func_symbol);
  }   
}
void maybeCallFuncString(const char* function, const char* payload){
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_1(func_symbol, scm_from_locale_string(payload));
  }   
}

SCM vec3ToScmList(glm::vec3 vec){
  SCM list = scm_make_list(scm_from_unsigned_integer(3), scm_from_unsigned_integer(0));
  scm_list_set_x (list, scm_from_unsigned_integer(0), scm_from_double(vec.x));
  scm_list_set_x (list, scm_from_unsigned_integer(1), scm_from_double(vec.y));
  scm_list_set_x (list, scm_from_unsigned_integer(2), scm_from_double(vec.z));
  return list;
}

// Main Api
int32_t (*_loadScene)(std::string);
SCM scm_loadScene(SCM value){
  auto sceneId = _loadScene(scm_to_locale_string(value));
  return scm_from_int32(sceneId);
}

objid (*_loadSceneObj)(std::string, objid);
SCM scm_loadSceneObj(SCM sceneFile, SCM sceneId){
  _loadSceneObj(scm_to_locale_string(sceneFile), scm_to_int32(sceneId));
  return SCM_UNSPECIFIED;
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
    scm_list_set_x (list, scm_from_unsigned_integer(i), scm_from_int32(scenes[i])); 
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
  std::string name, 
  std::map<std::string, std::string> stringAttributes,
  std::map<std::string, double> numAttributes,
  std::map<std::string, glm::vec3> vecAttributes
);
SCM scmMakeObjectAttr(SCM scmName, SCM scmAttributes){
  std::map<std::string, double> numAttributes;
  std::map<std::string, std::string> stringAttributes;
  std::map<std::string, glm::vec3> vecAttributes;

  auto numElements = toUnsignedInt(scm_length(scmAttributes));
  for (int i = 0; i < numElements; i++){
    auto propertyPair = scm_list_ref(scmAttributes, scm_from_unsigned_integer(i));
    auto pairLength = toUnsignedInt(scm_length(propertyPair));
    assert(pairLength == 2);
    auto attrName = scm_to_locale_string(scm_list_ref(propertyPair, scm_from_unsigned_integer(0)));
    assert(
      stringAttributes.find(attrName) == stringAttributes.end() &&
      vecAttributes.find(attrName) == vecAttributes.end()
    );

    auto attrValue = scm_list_ref(propertyPair, scm_from_unsigned_integer(1));

    bool isNumber = scm_is_number(attrValue);
    bool isString = scm_is_string(attrValue);
    bool isList = scm_to_bool(scm_list_p(attrValue));
    assert(isNumber || isString || isList);

    if (isNumber){
      numAttributes[attrName] = scm_to_double(attrValue);
    }else if (isString){
      stringAttributes[attrName] = scm_to_locale_string(attrValue);
    }else{
      auto vec3ListLength = toUnsignedInt(scm_length(attrValue));
      assert(vec3ListLength == 3);

      double values[] = {0, 0, 0};
      for (int j = 0; j < vec3ListLength; j++){
        auto vecValue = scm_list_ref(attrValue, scm_from_unsigned_integer(j));
        values[j] = scm_to_double(vecValue);
      }
      vecAttributes[attrName] = glm::vec3(values[0], values[1], values[2]);
    }
  }

  objid id = _makeObjectAttr(scm_to_locale_string(scmName), stringAttributes, numAttributes, vecAttributes);
  return scm_from_int32(id);
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

std::map<std::string, std::string> (*_getGameObjectAttr)(int32_t id);
SCM scmGetGameObjectAttr(SCM gameobj){
  std::map<std::string, std::string> attributes = _getGameObjectAttr(getGameobjId(gameobj));
  SCM list = scm_make_list(scm_from_unsigned_integer(attributes.size()), scm_from_unsigned_integer(0));
  int index = 0;
  for (auto [name, value] : attributes){
    SCM attributePair = scm_make_list(scm_from_unsigned_integer(2), scm_from_unsigned_integer(0));
    scm_list_set_x(attributePair, scm_from_unsigned_integer(0), scm_from_locale_string(name.c_str()));
    scm_list_set_x(attributePair, scm_from_unsigned_integer(1), scm_from_locale_string(value.c_str()));
    scm_list_set_x(list, scm_from_unsigned_integer(index), attributePair);
    index = index + 1;
  }
  return list;
}

// @TODO -> add types around this
void (*_setGameObjectAttr)(int32_t id, std::map<std::string, std::string> attr);
SCM scmSetGameObjectAttr(SCM gameobj, SCM attr){
  auto id = getGameobjId(gameobj);
  std::map<std::string, std::string> newAttributes;
  auto numElements = toUnsignedInt(scm_length(attr));
  for (int i = 0; i < numElements; i++){
    auto nameValuePair = scm_list_ref(attr, scm_from_unsigned_integer(i));
    auto attrName = scm_to_locale_string(scm_list_ref(nameValuePair, scm_from_unsigned_integer(0)));
    auto attrValue = scm_to_locale_string(scm_list_ref(nameValuePair, scm_from_unsigned_integer(1)));
    newAttributes[attrName] = attrValue;
  }
  _setGameObjectAttr(id, newAttributes);
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

std::optional<objid> (*_getGameObjectByName)(std::string name);
SCM getGameObjByName(SCM value){
  auto id = _getGameObjectByName(scm_to_locale_string(value));
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
void (*_playClip)(std::string);
SCM scmPlayClip(SCM soundname){
  _playClip(scm_to_locale_string(soundname));
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

void (*_sendNotifyMessage)(std::string message);
SCM scmSendNotify(SCM message){
  _sendNotifyMessage(scm_to_locale_string(message));
  return SCM_UNSPECIFIED;
}

void (*_attachToRail)(int32_t id, std::string rail);
SCM scmAttachToRail(SCM obj, SCM rail){
  _attachToRail(getGameobjId(obj), scm_to_locale_string(rail));
  return SCM_UNSPECIFIED;
}
void (*_unattachFromRail)(int32_t id);
SCM scmUnattachFromRail(SCM obj){
  _unattachFromRail(getGameobjId(obj));
  return SCM_UNSPECIFIED;
}

double (*_timeSeconds)();
SCM scmTimeSeconds(){
  return scm_from_double(_timeSeconds());
}
void (*_saveScene)(bool includeIds);
SCM scmSaveScene(SCM includeIds){
  _saveScene(includeIds == SCM_UNDEFINED ? false : scm_to_bool(includeIds));
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

// State machine functions
SCM scmAttributes(){
  return SCM_UNSPECIFIED;
}

SCM trackType; // this is modified during init
struct scmTrack {
  Track track;
  std::vector<SCM> funcRefs;
};
scmTrack* getTrackFromScmType(SCM value){
  scmTrack* obj;
  scm_assert_foreign_object_type (trackType, value);
  obj = (scmTrack*)scm_foreign_object_ref(value, 0);
  return obj; 
}
void finalizeTrack (SCM trackobj){  // test bvy invoking [gc]
  auto track = getTrackFromScmType(trackobj);
  for (auto scmFn : track -> funcRefs){
    scm_gc_unprotect_object(scmFn);
  }
}
Track (*_createTrack)(std::string name, std::vector<std::function<void()>> fns);
SCM scmCreateTrack(SCM name, SCM funcs){
  auto trackobj = (scmTrack*)scm_gc_malloc(sizeof(scmTrack), "track");

  std::vector<std::function<void()>> tracks;
  std::vector<SCM> funcRefs;

  auto numTrackFns = toUnsignedInt(scm_length(funcs));
  for (int i = 0; i < numTrackFns; i++){
    SCM func = scm_list_ref(funcs, scm_from_unsigned_integer(i));  
    bool isThunk = scm_to_bool(scm_procedure_p(func));
    assert(isThunk);
    tracks.push_back([func]() -> void{
      scm_call_0(func);  
    });

    scm_gc_protect_object(func); // ref counting to prevent garbage collection, decrement happens in finalizer 
    funcRefs.push_back(func);
  }

  auto track = createTrack(scm_to_locale_string(name), tracks);
  trackobj -> track = track;
  trackobj -> funcRefs = funcRefs;

  scm_t_struct_finalize finalizer = finalizeTrack;
  return scm_make_foreign_object_1(trackType, trackobj);
}
void (*_playbackTrack)(Track& track);
SCM scmPlayTrack(SCM track){
  _playbackTrack(getTrackFromScmType(track) -> track);
  return SCM_UNSPECIFIED;
}

SCM stateType; // this is modified during init
SCM scmState(SCM name, SCM scmTracks){                  
  auto stateobj = (State*)scm_gc_malloc(sizeof(State), "state");
  std::map<std::string, std::string> attributes;
  std::map<std::string, Track> tracks;

  auto listLength = toUnsignedInt(scm_length(scmTracks));

  std::string defaultTrack;
  for (unsigned int i = 0; i < listLength; i++){
    SCM trackValue = scm_list_ref(scmTracks, scm_from_int64(i));
    scmTrack* track = getTrackFromScmType(trackValue);
    tracks[track -> track.name] = track -> track;  // @TODO - probably need to add extra track scm gc lock here + finalize, since making copy of the track
    if (i == 0){
      defaultTrack = track -> track.name;
    }
  }
  assert(listLength > 0);

  State state {
    .name = scm_to_locale_string(name),
    .defaultTrack = defaultTrack,
    .attributes = attributes,
    .tracks = tracks,
  };
  *stateobj = state;
  return scm_make_foreign_object_1(stateType, stateobj);
}

std::vector<State> fromScmStateList(SCM statesList){
  std::vector<State> states;
  auto listLength = toUnsignedInt(scm_length(statesList));
  for (unsigned int i = 0; i < listLength; i++){
    SCM scmValue = scm_list_ref(statesList, scm_from_int64(i));
    scm_assert_foreign_object_type(stateType, scmValue);
    State* obj = (State*)scm_foreign_object_ref(scmValue, 0);
    states.push_back(*obj);
  }
  return states;
}

SCM stateMachineType; // this is modified during init
StateMachine* getMachineFromScmType(SCM value){
  StateMachine* obj;
  scm_assert_foreign_object_type (stateMachineType, value);
  obj = (StateMachine*)scm_foreign_object_ref(value, 0);
  return obj; 
}
StateMachine (*_createStateMachine)(std::vector<State> states);
SCM scmStateMachine(SCM states){
  auto statemachineobj = (StateMachine*)scm_gc_malloc(sizeof(StateMachine), "statemachine");
  auto machine = _createStateMachine(fromScmStateList(states));
  *statemachineobj = machine;
  return scm_make_foreign_object_1(stateMachineType, statemachineobj);
}

void (*_playStateMachine)(StateMachine* machine);
SCM scmPlayStateMachine(SCM scmMachine){
  _playStateMachine(getMachineFromScmType(scmMachine));
  return SCM_UNSPECIFIED;
}
void (*_setStateMachine)(StateMachine* machine, std::string newState);
SCM scmSetStateMachine(SCM scmMachine, SCM state){
  _setStateMachine(getMachineFromScmType(scmMachine), scm_to_locale_string(state));
  return SCM_UNSPECIFIED;
}

void (*_startRecording)(objid id, std::string recordingPath);
SCM scmStartRecording(SCM id, SCM recordingPath){
  _startRecording(scm_to_int32(id), scm_to_locale_string(recordingPath));
  return SCM_UNSPECIFIED;
}
void (*_playRecording)(objid id, std::string recordingPath);
SCM scmPlayRecording(SCM id, SCM recordingPath){
  _playRecording(scm_to_int32(id), scm_to_locale_string(recordingPath));
  return SCM_UNSPECIFIED;
}
std::vector<objid> (*_raycast)(glm::vec3 pos, glm::quat direction, float maxDistance);
SCM scmRaycast(SCM pos, SCM direction, SCM distance){
  std::vector<objid> ids = _raycast(listToVec3(pos), scmListToQuat(direction), scm_to_double(distance));
  SCM list = scm_make_list(scm_from_unsigned_integer(ids.size()), scm_from_unsigned_integer(0));
  for (int i = 0; i < ids.size(); i++){
    scm_list_set_x(list, scm_from_unsigned_integer(i), scm_from_int32(ids.at(i)));
  }
  return list;
}

void (*_saveScreenshot)(std::string filepath);
SCM scmSaveScreenshot(SCM filepath){
  _saveScreenshot(scm_to_locale_string(filepath));
  return SCM_UNSPECIFIED;
}

void (*_setState)(std::string);
SCM scmSetState(SCM value){
  _setState(scm_to_locale_string(value));
  return SCM_UNSPECIFIED;
}

void (*_setFloatState)(std::string, float);
SCM scmSetFloatState(SCM value, SCM amount){
  _setFloatState(scm_to_locale_string(value), scm_to_double(amount));
  return SCM_UNSPECIFIED;
}

// Callbacks
void onFrame(){
  maybeCallFunc("onFrame");
}
void onCollisionEnter(int32_t obj1, int32_t obj2, glm::vec3 contactPos){
  const char* function = "onCollideEnter";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_3(func_symbol, createGameObject(obj1), createGameObject(obj2), vec3ToScmList(contactPos));
  }
}
void onCollisionExit(int32_t obj1, int32_t obj2){
  const char* function = "onCollideExit";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_2(func_symbol, createGameObject(obj1), createGameObject(obj2));
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
void onMessage(std::string message){
  const char* function = "onMessage";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_1(func_symbol, scm_from_locale_string(message.c_str()));
  }
}
void onFloatMessage(StringFloat message){
  const char* function = "onFloatMessage";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_2(func_symbol, scm_from_locale_string(message.strValue.c_str()), scm_from_double(message.floatValue));
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

////////////
void defineFunctions(objid id, bool isServer){
  scm_c_define_gsubr("load-scene", 1, 0, 0, (void *)scm_loadScene);
  scm_c_define_gsubr("load-subscene", 2, 0, 0, (void *)scm_loadSceneObj);

  scm_c_define_gsubr("unload-scene", 1, 0, 0, (void *)scm_unloadScene);
  scm_c_define_gsubr("unload-all-scenes", 0, 0, 0, (void *)scm_unloadAllScenes);
  scm_c_define_gsubr("list-scenes", 0, 0, 0, (void *)scm_listScenes);
  scm_c_define_gsubr("send-load-scene", 1, 0, 0, (void *)scm_sendLoadScene);

  scm_c_define_gsubr("ls-models", 0, 0, 0, (void *)scmListModels);

  scm_c_define_gsubr("set-selection-mode", 1, 0, 0, (void *)setSelectionMod);
  scm_c_define_gsubr("set-camera", 1, 0, 0, (void *)setActiveCam);    
  scm_c_define_gsubr("mov-cam", 3, 0, 0, (void *)scmMoveCamera);   // @TODO move + rotate camera can be removed since had the gameobj manipulation functions
  scm_c_define_gsubr("rot-cam", 2, 0, 0, (void *)scmRotateCamera);
  scm_c_define_gsubr("rm-obj", 1, 0, 0, (void *)removeObject);
  scm_c_define_gsubr("lsobj-type", 1, 0, 0, (void *)lsObjectsByType);
  scm_c_define_gsubr("lsobj-name", 1, 0, 0, (void *)getGameObjByName);
 
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

  // physics functions
  scm_c_define_gsubr("applyimpulse", 2, 0, 0, (void *)scm_applyImpulse);
  scm_c_define_gsubr("applyimpulse-rel", 2, 0, 0, (void *)scm_applyImpulseRel);
  scm_c_define_gsubr("clearimpulse", 1, 0, 0, (void *)scm_clearImpulse);

  // audio stuff
  scm_c_define_gsubr("lsclips", 0, 0, 0, (void*)scmListClips);
  scm_c_define_gsubr("playclip", 1, 0, 0, (void*)scmPlayClip);

  // event system
  scm_c_define_gsubr("sendmessage", 1, 0, 0, (void*)scmSendEventMessage);
  scm_c_define_gsubr("sendnotify", 1, 0, 0, (void*)scmSendNotify);

  // rails
  scm_c_define_gsubr("attach-rail", 2, 0, 0, (void*)scmAttachToRail);
  scm_c_define_gsubr("unattach-rail", 1, 0, 0, (void*)scmUnattachFromRail);

  scm_c_define_gsubr("time-seconds", 0, 0, 0, (void*)scmTimeSeconds);
  scm_c_define_gsubr("save-scene", 0, 1, 0, (void*)scmSaveScene);

  scm_c_define_gsubr("list-servers", 0, 0, 0, (void*)scmListServers);
  scm_c_define_gsubr("connect-server", 1, 0, 0, (void*)scmConnectServer);
  scm_c_define_gsubr("disconnect-server", 0, 0, 0, (void*)scmDisconnectServer);
  scm_c_define_gsubr("send-tcp", 1, 0, 0, (void*)scmSendMessageTcp);
  scm_c_define_gsubr("send-udp", 1, 0, 0, (void*)scmSendMessageUdp);

  scm_c_define_gsubr("attributes", 0, 0, 0, (void*)scmAttributes);
  scm_c_define_gsubr("create-track", 2, 0, 0, (void*)scmCreateTrack);
  scm_c_define_gsubr("play-track", 1, 0, 0, (void*)scmPlayTrack);
  scm_c_define_gsubr("state", 2, 0, 0, (void*)scmState);
  scm_c_define_gsubr("machine", 1, 0, 0, (void*)scmStateMachine);
  scm_c_define_gsubr("play-machine", 1, 0, 0, (void*)scmPlayStateMachine);
  scm_c_define_gsubr("set-machine", 2, 0, 0, (void*)scmSetStateMachine);

  // state-on-event
  // state-on-exit
  scm_c_define_gsubr("start-recording", 2, 0, 0, (void*)scmStartRecording);
  scm_c_define_gsubr("play-recording", 2, 0, 0, (void*)scmPlayRecording);

  scm_c_define_gsubr("mk-obj-attr", 2, 0, 0, (void*)scmMakeObjectAttr);
  scm_c_define_gsubr("raycast", 3, 0, 0, (void*)scmRaycast);

  scm_c_define("mainobj", createGameObject(id));
  scm_c_define("is-server", scm_from_bool(isServer));

  scm_c_define_gsubr("ss", 1, 0, 0, (void*)scmSaveScreenshot);
  scm_c_define_gsubr("set-state", 1, 0, 0, (void*)scmSetState);
  scm_c_define_gsubr("set-fstate", 2, 0, 0, (void*)scmSetFloatState);
}


void createStaticSchemeBindings(
  int32_t (*loadScene)(std::string),  
  objid(*loadSceneObj)(std::string, objid),
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
  void (*attachToRail)(int32_t id, std::string rail),
  void (*unattachFromRail)(int32_t id),
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
  void (*startRecording)(objid id, std::string recordingPath),
  void (*playRecording)(objid id, std::string recordingPath),
  objid (*makeObjectAttr)(std::string name, std::map<std::string, std::string> stringAttributes, std::map<std::string, double> numAttributes, std::map<std::string, glm::vec3> vecAttributes),
  std::vector<objid> (*raycast)(glm::vec3 pos, glm::quat direction, float maxDistance),
  void (*saveScreenshot)(std::string),
  void (*setState)(std::string),
  void (*setFloatState)(std::string stateName, float value)
){
  scm_init_guile();
  gameObjectType = scm_make_foreign_object_type(scm_from_utf8_symbol("gameobj"), scm_list_1(scm_from_utf8_symbol("data")), NULL);
  trackType = scm_make_foreign_object_type(scm_from_utf8_symbol("track"), scm_list_1(scm_from_utf8_symbol("data")), finalizeTrack);
  stateType = scm_make_foreign_object_type(scm_from_utf8_symbol("state"),  scm_list_1(scm_from_utf8_symbol("data")), NULL);
  stateMachineType = scm_make_foreign_object_type(scm_from_utf8_symbol("statemachine"),  scm_list_1(scm_from_utf8_symbol("data")), NULL);

  _loadScene = loadScene;
  _loadSceneObj = loadSceneObj;

  _unloadScene = unloadScene;
  _unloadAllScenes = unloadAllScenes;
  _listScenes = listScenes;
  _sendLoadScene = sendLoadScene;
  
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

  _attachToRail = attachToRail;
  _unattachFromRail = unattachFromRail;
  _timeSeconds = timeSeconds;
  _saveScene = saveScene;

  _listServers = listServers;
  _connectServer = connectServer;
  _disconnectServer = disconnectServer;
  _sendMessageTcp = sendMessageTcp;
  _sendMessageUdp = sendMessageUdp;

  // state machine
  _createTrack = createTrack;
  _playbackTrack = playbackTrack;
  _createStateMachine = createStateMachine;
  _playStateMachine = playStateMachine;
  _setStateMachine = setStateMachine;

  _startRecording = startRecording;
  _playRecording = playRecording;

  _makeObjectAttr = makeObjectAttr;
  _raycast = raycast;

  _saveScreenshot = saveScreenshot;
  _setState = setState;
  _setFloatState = setFloatState;
}
