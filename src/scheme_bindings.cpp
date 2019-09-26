#include "./scheme_bindings.h"

void* startGuile(void* data){
	return NULL;
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


void onFrame(){
  static SCM func_symbol = scm_variable_ref(scm_c_lookup("onFrame"));
  scm_call_0(func_symbol);
}
void onMouseCallback(int button, int action, int mods){
  static SCM func_symbol = scm_variable_ref(scm_c_lookup("onMouse"));
  scm_call_3(func_symbol, scm_from_int(button), scm_from_int(action), scm_from_int(mods));
}
void onKeyCallback(int key, int scancode, int action, int mods){
  static SCM func_symbol = scm_variable_ref(scm_c_lookup("onKey"));
  scm_call_4(func_symbol, scm_from_int(key), scm_from_int(scancode), scm_from_int(action), scm_from_int(mods));  
}

// Gameobject manipulation
struct gameObject {
  short id;
};
SCM gameObjectType;
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

void onObjectSelected(short index){
  auto obj = (gameObject *)scm_gc_malloc(sizeof(gameObject), "gameobj");
  obj->id = index;
  SCM gameobject = scm_make_foreign_object_1(gameObjectType, obj);

  static SCM func_symbol = scm_variable_ref(scm_c_lookup("onObjSelected"));
  scm_call_1(func_symbol, gameobject);
}

std::string (*getGameObjNameForId)(short id);
SCM getGameObjNameForIdFn(SCM value){
  gameObject *obj;
  scm_assert_foreign_object_type (gameObjectType, value);
  obj = (gameObject*)scm_foreign_object_ref (value, 0);
  return scm_from_locale_string(getGameObjNameForId(obj->id).c_str());
}

glm::vec3 (*getGameObjectPosn)(short index);
SCM getGameObjectPosition(SCM value){
  gameObject *obj;
  scm_assert_foreign_object_type (gameObjectType, value);
  obj = (gameObject*)scm_foreign_object_ref (value, 0);
  
  glm::vec3 pos = getGameObjectPosn(obj->id);
  SCM list = scm_make_list(scm_from_unsigned_integer(3), scm_from_unsigned_integer(0));
  scm_list_set_x (list, scm_from_unsigned_integer(0), scm_from_double(pos.x));
  scm_list_set_x (list, scm_from_unsigned_integer(1), scm_from_double(pos.y));
  scm_list_set_x (list, scm_from_unsigned_integer(2), scm_from_double(pos.z));
  return list;
}
void (*setGameObjectPosn)(short index, glm::vec3 pos);
SCM setGameObjectPosition(SCM value, SCM positon){
  gameObject *obj;
  scm_assert_foreign_object_type (gameObjectType, value);
  obj = (gameObject*)scm_foreign_object_ref (value, 0);
  
  auto x = scm_to_double(scm_list_ref(positon, scm_from_int64(0)));
  auto y = scm_to_double(scm_list_ref(positon, scm_from_int64(1)));
  auto z = scm_to_double(scm_list_ref(positon, scm_from_int64(2)));
  setGameObjectPosn(obj->id, glm::vec3(x, y, z));
  return SCM_UNSPECIFIED;
}
SCM getGameObjectId(SCM value){
  gameObject *obj;
  scm_assert_foreign_object_type (gameObjectType, value);
  obj = (gameObject*)scm_foreign_object_ref (value, 0);
  return scm_from_short(obj->id);
}
short (*getGameObjName)(std::string name);
SCM getGameObjByName(SCM value){
  short id = getGameObjName(scm_to_locale_string(value));
  if (id == -1){
    return SCM_UNSPECIFIED;
  }
  auto obj = (gameObject *)scm_gc_malloc(sizeof(gameObject), "gameobj");
  obj->id = id;
  return scm_make_foreign_object_1(gameObjectType, obj);
}
////////////

SchemeBindingCallbacks createStaticSchemeBindings(
  std::string scriptPath,
	void (*moveCamera)(glm::vec3),  
	void (*rotateCamera)(float xoffset, float yoffset),
	void (*removeObjectById)(short id),
	void (*makeObjectV)(std::string, std::string, float, float, float),
	std::vector<short> (*getObjectsByType)(std::string),
	void (*setActiveCamera)(short cameraId),
  void (*drawText)(std::string word, float left, float top, unsigned int fontSize),
  std::string (*getGameObjectNameForId)(short id),
  glm::vec3 (*getGameObjectPos)(short index),
  void (*setGameObjectPos)(short index, glm::vec3 pos),
  short (*getGameObjectByName)(std::string name),
  void (*setSelectionMode)(bool enabled)
){
  scm_init_guile();
  
  selectionMode = setSelectionMode;
	moveCam = moveCamera;
	rotateCam = rotateCamera;
	removeObjById = removeObjectById;
	makeObj = makeObjectV;
	getObjByType = getObjectsByType;
	setActCamera = setActiveCamera;
  drawTextV = drawText;
  getGameObjNameForId = getGameObjectNameForId;
  getGameObjectPosn = getGameObjectPos;
  setGameObjectPosn = setGameObjectPos;
  getGameObjName = getGameObjectByName;

  scm_c_define_gsubr("setSelectionMode", 1, 0, 0, (void *)setSelectionMod);
  scm_c_define_gsubr("setCamera", 1, 0, 0, (void *)setActiveCam);
	scm_c_define_gsubr("movCam", 3, 0, 0, (void *)scmMoveCamera);
	scm_c_define_gsubr("rotCam", 2, 0, 0, (void *)scmRotateCamera);
	scm_c_define_gsubr("mkObj", 2, 1, 1, (void *)makeObject);
	scm_c_define_gsubr("rmObj", 1, 0, 0, (void *)removeObject);
  scm_c_define_gsubr("lsObjByType", 1, 0, 0, (void *)lsObjectsByType);
  scm_c_define_gsubr("lsObjByName", 1, 0, 0, (void *)getGameObjByName);
  scm_c_define_gsubr("drawText", 4, 0, 0, (void *)drawTextWords);
 
  // Gameobject funcs
  SCM name = scm_from_utf8_symbol("gameobj");
  SCM slots = scm_list_1(scm_from_utf8_symbol("data"));
  scm_t_struct_finalize finalizer = NULL;
  gameObjectType = scm_make_foreign_object_type(name, slots, finalizer);
  scm_c_define_gsubr("gameobj-pos", 1, 0, 0, (void *)getGameObjectPosition);
  scm_c_define_gsubr("gameobj-setpos!", 2, 0, 0, (void *)setGameObjectPosition);
  scm_c_define_gsubr("gameobj-id", 1, 0, 0, (void *)getGameObjectId);
  scm_c_define_gsubr("gameobj-name", 1, 0, 0, (void *)getGameObjNameForIdFn);
  ////
  
  scm_c_primitive_load(scriptPath.c_str());

  SchemeBindingCallbacks callbackFuncs = {
    .onFrame = onFrame,
    .onMouseCallback = onMouseCallback,
    .onObjectSelected = onObjectSelected,
    .onKeyCallback = onKeyCallback,
  };

  return callbackFuncs;
}

void startShell(){
  scm_with_guile(&startGuile, NULL);
  int argc = 0;
  char* argv[] = { { } };
  scm_shell(argc, argv);
}

