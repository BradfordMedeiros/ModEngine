#include "./scheme_bindings.h"

bool symbolDefinedInModule(const char* symbol, SCM module){
  return scm_to_bool(scm_defined_p(scm_string_to_symbol(scm_from_locale_string(symbol)), module));
}
bool symbolDefined(const char* symbol){
  return symbolDefinedInModule(symbol, scm_current_module());
}

// Main Api
short (*_loadScene)(std::string);
SCM scm_loadScene(SCM value){
  auto sceneId = _loadScene(scm_to_locale_string(value));
  return scm_from_short(sceneId);
}
void (*_unloadScene)(short id);
SCM scm_unloadScene(SCM value){
  _unloadScene(scm_to_short(value));
  return SCM_UNSPECIFIED;
}
std::vector<short> (*_listScenes)();
SCM scm_listScenes(){
  auto scenes = _listScenes();
  SCM list = scm_make_list(scm_from_unsigned_integer(scenes.size()), scm_from_unsigned_integer(0));
  for (int i = 0; i < scenes.size(); i++){
    scm_list_set_x (list, scm_from_unsigned_integer(i), scm_from_short(scenes[i])); 
  }
  return list;
}

void (*selectionMode)(bool enabled);
SCM setSelectionMod(SCM value){
  selectionMode(scm_to_bool(value));
  return SCM_UNSPECIFIED;
}

void (*setActCamera)(short id);
SCM setActiveCam(SCM value){
  setActCamera(scm_to_short(value));
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

void (*makeObj)(std::string, std::string, float, float, float);
SCM makeObject(SCM name, SCM mesh, SCM position, SCM rest){
  auto xPos = scm_to_double(scm_list_ref(position, scm_from_int64(0)));
  auto yPos = scm_to_double(scm_list_ref(position, scm_from_int64(1)));
  auto zPos = scm_to_double(scm_list_ref(position, scm_from_int64(2)));
  makeObj(scm_to_locale_string(name), scm_to_locale_string(mesh), xPos, yPos, zPos);
  return SCM_UNSPECIFIED;
}

void (*removeObjById)(short id);
SCM removeObject(SCM value){
  removeObjById(scm_to_short(value));
  return SCM_UNSPECIFIED;
}

void (*drawTextV)(std::string word, float left, float top, unsigned int fontSize);
SCM drawTextWords(SCM word, SCM left, SCM top, SCM fontSize){
  drawTextV(
    scm_to_locale_string(word), 
    scm_to_double(left), 
    scm_to_double(top), 
    scm_to_unsigned_integer(fontSize, std::numeric_limits<unsigned int>::min(), std::numeric_limits<unsigned int>::max())
  );
  return SCM_UNSPECIFIED;
}

struct gameObject {
  short id;
};
SCM gameObjectType;   // this is modified during init
std::vector<short> (*getObjByType)(std::string);
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

short getGameobjId(SCM value){
  gameObject *obj;
  scm_assert_foreign_object_type (gameObjectType, value);
  obj = (gameObject*)scm_foreign_object_ref (value, 0);
  return obj->id;  
}

std::string (*getGameObjNameForId)(short id);
SCM getGameObjNameForIdFn(SCM value){
  return scm_from_locale_string(getGameObjNameForId(getGameobjId(value)).c_str());
}

std::map<std::string, std::string> (*_getGameObjectAttr)(short id);
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
void (*_setGameObjectAttr)(short id, std::map<std::string, std::string> attr);
SCM scmSetGameObjectAttr(SCM gameobj, SCM attr){
  auto id = getGameobjId(gameobj);
  std::map<std::string, std::string> newAttributes;
  auto numElements = scm_to_unsigned_integer(scm_length(attr), std::numeric_limits<unsigned int>::min(), std::numeric_limits<unsigned int>::max());
  for (int i = 0; i < numElements; i++){
    auto nameValuePair = scm_list_ref(attr, scm_from_unsigned_integer(i));
    auto attrName = scm_to_locale_string(scm_list_ref(nameValuePair, scm_from_unsigned_integer(0)));
    auto attrValue = scm_to_locale_string(scm_list_ref(nameValuePair, scm_from_unsigned_integer(1)));
    newAttributes[attrName] = attrValue;
  }
  _setGameObjectAttr(id, newAttributes);
  return SCM_UNSPECIFIED;
}

glm::vec3 (*getGameObjectPosn)(short index);
SCM getGameObjectPosition(SCM value){
  glm::vec3 pos = getGameObjectPosn(getGameobjId(value));
  SCM list = scm_make_list(scm_from_unsigned_integer(3), scm_from_unsigned_integer(0));
  scm_list_set_x (list, scm_from_unsigned_integer(0), scm_from_double(pos.x));
  scm_list_set_x (list, scm_from_unsigned_integer(1), scm_from_double(pos.y));
  scm_list_set_x (list, scm_from_unsigned_integer(2), scm_from_double(pos.z));
  return list;
}
void (*setGameObjectPosn)(short index, glm::vec3 pos);
SCM setGameObjectPosition(SCM value, SCM positon){
  auto x = scm_to_double(scm_list_ref(positon, scm_from_int64(0)));   
  auto y = scm_to_double(scm_list_ref(positon, scm_from_int64(1)));
  auto z = scm_to_double(scm_list_ref(positon, scm_from_int64(2)));
  setGameObjectPosn(getGameobjId(value), glm::vec3(x, y, z));
  return SCM_UNSPECIFIED;
}

void (*setGameObjectPosnRel)(short index, float x, float y, float z, bool xzPlaneOnly);
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

glm::quat (*getGameObjectRotn)(short index);
SCM getGameObjectRotation(SCM value){
  return scmQuatToSCM(getGameObjectRotn(getGameobjId(value)));
}
void (*setGameObjectRotn)(short index, glm::quat rotation);
SCM setGameObjectRotation(SCM value, SCM rotation){
  setGameObjectRotn(getGameobjId(value), scmListToQuat(rotation));
  return SCM_UNSPECIFIED;
}

glm::quat (*_setFrontDelta)(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta);
SCM setGameObjectRotationDelta(SCM value, SCM deltaYaw, SCM deltaPitch, SCM deltaRoll){
  short id = getGameobjId(value);
  glm::quat rot = getGameObjectRotn(id);

  auto deltaY = scm_to_double(deltaYaw);
  auto deltaP = scm_to_double(deltaPitch);
  auto deltaR = scm_to_double(deltaRoll); 

  glm::quat newOrientation = _setFrontDelta(rot, deltaY, deltaP, deltaR, 0.1);
  setGameObjectRotn(id, newOrientation);

  return SCM_UNSPECIFIED;
}
SCM scm_setFrontDelta(SCM orientation, SCM deltaYaw, SCM deltaPitch, SCM deltaRoll){
  glm::quat intialOrientation = scmListToQuat(orientation);
  auto deltaY = scm_to_double(deltaYaw);
  auto deltaP = scm_to_double(deltaPitch);
  auto deltaR = scm_to_double(deltaRoll); 
  return scmQuatToSCM(_setFrontDelta(intialOrientation, deltaY, deltaP, deltaR, 1));
}

std::vector<std::string>(*_listAnimations)(short id);
SCM scmListAnimations(SCM value){
  auto animations = _listAnimations(getGameobjId(value));
  int numAnimations = animations.size();

  SCM list = scm_make_list(scm_from_unsigned_integer(numAnimations), scm_from_unsigned_integer(0));
  for (int i = 0; i < numAnimations; i++){
    scm_list_set_x (list, scm_from_unsigned_integer(i), scm_from_locale_string(animations.at(i).c_str()));
  }

  return list;
}

void (*_playAnimation)(short id, std::string animationName);
SCM scmPlayAnimation(SCM value, SCM animationName){
  _playAnimation(getGameobjId(value), scm_to_locale_string(animationName));
  return SCM_UNSPECIFIED;
}

glm::vec3 listToVec3(SCM vecList){
  auto x = scm_to_double(scm_list_ref(vecList, scm_from_int64(0)));   
  auto y = scm_to_double(scm_list_ref(vecList, scm_from_int64(1)));
  auto z = scm_to_double(scm_list_ref(vecList, scm_from_int64(2))); 
  return glm::vec3(x, y, z);
}

void (*_applyImpulse)(short index, glm::vec3 impulse);
SCM scm_applyImpulse(SCM value, SCM impulse){
  _applyImpulse(getGameobjId(value), listToVec3(impulse));
  return SCM_UNSPECIFIED;
}
void (*_applyImpulseRel)(short index, glm::vec3 impulse);
SCM scm_applyImpulseRel(SCM value, SCM impulse){
  _applyImpulseRel(getGameobjId(value), listToVec3(impulse));
  return SCM_UNSPECIFIED;
}

void (*_clearImpulse)(short index);
SCM scm_clearImpulse(SCM value){
  _clearImpulse(getGameobjId(value));
  return SCM_UNSPECIFIED;
}

SCM getGameObjectId(SCM value){
  return scm_from_short(getGameobjId(value));
}
SCM createGameObject(short id){
  auto obj = (gameObject *)scm_gc_malloc(sizeof(gameObject), "gameobj");
  obj->id = id;
  return scm_make_foreign_object_1(gameObjectType, obj); 
}

short (*getGameObjName)(std::string name);
SCM getGameObjByName(SCM value){
  short id = getGameObjName(scm_to_locale_string(value));
  if (id == -1){
    return scm_from_bool(false);
  }
  return createGameObject(id);
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

void (*_attachToRail)(short id, std::string rail);
SCM scmAttachToRail(SCM obj, SCM rail){
  _attachToRail(getGameobjId(obj), scm_to_locale_string(rail));
  return SCM_UNSPECIFIED;
}
void (*_unattachFromRail)(short id);
SCM scmUnattachFromRail(SCM obj){
  _unattachFromRail(getGameobjId(obj));
  return SCM_UNSPECIFIED;
}

double (*_timeSeconds)();
SCM scmTimeSeconds(){
  return scm_from_double(_timeSeconds());
}

// Callbacks
void onFrame(){
  const char* function = "onFrame";

  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_0(func_symbol);
  }
}
void onCollisionEnter(short obj1, short obj2){
  const char* function = "onCollideEnter";
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_2(func_symbol, createGameObject(obj1), createGameObject(obj2));
  }
}
void onCollisionExit(short obj1, short obj2){
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
void onObjectSelected(short index){
  const char* function = "onObjSelected";
  if (symbolDefined(function)){
    auto obj = (gameObject *)scm_gc_malloc(sizeof(gameObject), "gameobj");
    obj->id = index;
    SCM gameobject = scm_make_foreign_object_1(gameObjectType, obj);

    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_1(func_symbol, gameobject);
  }
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

////////////
void defineFunctions(){
  scm_c_define_gsubr("load-scene", 1, 0, 0, (void *)scm_loadScene);
  scm_c_define_gsubr("unload-scene", 1, 0, 0, (void *)scm_unloadScene);
  scm_c_define_gsubr("list-scenes", 0, 0, 0, (void *)scm_listScenes);
  scm_c_define_gsubr("ls-models", 0, 0, 0, (void *)scmListModels);

  scm_c_define_gsubr("set-selection-mode", 1, 0, 0, (void *)setSelectionMod);
  scm_c_define_gsubr("set-camera", 1, 0, 0, (void *)setActiveCam);    
  scm_c_define_gsubr("mov-cam", 3, 0, 0, (void *)scmMoveCamera);   // @TODO move + rotate camera can be removed since had the gameobj manipulation functions
  scm_c_define_gsubr("rot-cam", 2, 0, 0, (void *)scmRotateCamera);
  scm_c_define_gsubr("mk-obj", 2, 1, 1, (void *)makeObject);
  scm_c_define_gsubr("rm-obj", 1, 0, 0, (void *)removeObject);
  scm_c_define_gsubr("lsobj-type", 1, 0, 0, (void *)lsObjectsByType);
  scm_c_define_gsubr("lsobj-name", 1, 0, 0, (void *)getGameObjByName);
  scm_c_define_gsubr("draw-text", 4, 0, 0, (void *)drawTextWords);
 
  scm_c_define_gsubr("gameobj-pos", 1, 0, 0, (void *)getGameObjectPosition);
  scm_c_define_gsubr("gameobj-setpos!", 2, 0, 0, (void *)setGameObjectPosition);
  scm_c_define_gsubr("gameobj-setpos-rel!", 2, 0, 0, (void *)setGameObjectPositionRel);
  scm_c_define_gsubr("gameobj-setpos-relxz!", 2, 0, 0, (void *)setGameObjectPositionRelXZ);
  
  scm_c_define_gsubr("gameobj-rot", 1, 0, 0, (void *)getGameObjectRotation);
  scm_c_define_gsubr("gameobj-setrot!", 2, 0, 0, (void *)setGameObjectRotation);
  scm_c_define_gsubr("gameobj-setrotd!", 4, 0, 0, (void *)setGameObjectRotationDelta);

  scm_c_define_gsubr("gameobj-id", 1, 0, 0, (void *)getGameObjectId);
  scm_c_define_gsubr("gameobj-name", 1, 0, 0, (void *)getGameObjNameForIdFn);

  scm_c_define_gsubr("gameobj-animations", 1, 0, 0, (void *)scmListAnimations);
  scm_c_define_gsubr("gameobj-playanimation", 2, 0, 0, (void *)scmPlayAnimation);

  scm_c_define_gsubr("gameobj-attr", 1, 0, 0, (void *)scmGetGameObjectAttr);
  scm_c_define_gsubr("gameobj-setattr!", 2, 0, 0, (void *)scmSetGameObjectAttr);


  // UTILITY FUNCTIONS
  scm_c_define_gsubr("setfrontdelta", 4, 0, 0, (void *)scm_setFrontDelta);

  // physics functions
  scm_c_define_gsubr("applyimpulse", 2, 0, 0, (void *)scm_applyImpulse);
  scm_c_define_gsubr("applyimpulse-rel", 2, 0, 0, (void *)scm_applyImpulseRel);
  scm_c_define_gsubr("clearimpulse", 1, 0, 0, (void *)scm_clearImpulse);

  // audio stuff
  scm_c_define_gsubr("lsclips", 0, 0, 0, (void*)scmListClips);
  scm_c_define_gsubr("playclip", 1, 0, 0, (void*)scmPlayClip);

  // event system
  scm_c_define_gsubr("sendmessage", 1, 0, 0, (void*)scmSendEventMessage);

  // rails
  scm_c_define_gsubr("attach-rail", 2, 0, 0, (void*)scmAttachToRail);
  scm_c_define_gsubr("unattach-rail", 1, 0, 0, (void*)scmUnattachFromRail);

  // time
  scm_c_define_gsubr("time-seconds", 0, 0, 0, (void*)scmTimeSeconds);
}


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
  double (*timeSeconds)()
){
  scm_init_guile();
  gameObjectType = scm_make_foreign_object_type(scm_from_utf8_symbol("gameobj"), scm_list_1(scm_from_utf8_symbol("data")), NULL);

  _loadScene = loadScene;
  _unloadScene = unloadScene;
  _listScenes = listScenes;
  selectionMode = setSelectionMode;
	moveCam = moveCamera;
	rotateCam = rotateCamera;
	removeObjById = removeObjectById;
	makeObj = makeObjectV;
	getObjByType = getObjectsByType;
	setActCamera = setActiveCamera;
  drawTextV = drawText;
  getGameObjNameForId = getGameObjectNameForId;
  _getGameObjectAttr = getGameObjectAttr;
  _setGameObjectAttr = setGameObjectAttr;

  getGameObjectPosn = getGameObjectPos;
  setGameObjectPosn = setGameObjectPos;
  setGameObjectPosnRel = setGameObjectPosRelative;
  getGameObjectRotn = getGameObjectRot;
  setGameObjectRotn = setGameObjectRot;
  _setFrontDelta = setFrontDelta;

  _applyImpulse = applyImpulse;
  _applyImpulseRel = applyImpulseRel;
  _clearImpulse = clearImpulse;


  getGameObjName = getGameObjectByName;
  _listAnimations = listAnimations;
  _playAnimation = playAnimation;
  _listClips = listClips;
  _playClip = playClip;
  _listModels = listModels;
  _sendEventMessage = sendEventMessage;
  _attachToRail = attachToRail;
  _unattachFromRail = unattachFromRail;
  _timeSeconds = timeSeconds;
}
