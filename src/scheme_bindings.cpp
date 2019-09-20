#include "./scheme_bindings.h"

void* startGuile(void* data){
	return NULL;
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

std::vector<short> (*getObjByType)(std::string);
SCM lsObjectsByType(SCM value){
  std::string objectType = scm_to_locale_string(value);
  std::vector indexes = getObjByType(objectType);

  SCM list = scm_make_list(scm_from_unsigned_integer(indexes.size()), scm_from_unsigned_integer(0));
 
  for (unsigned int i = 0; i < indexes.size(); i++){
  	SCM num = scm_from_short(indexes[i]);
	  scm_list_set_x (list, scm_from_unsigned_integer(i), num);
  }

  return list;
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
  static SCM func_symbol = scm_variable_ref(scm_c_lookup("onKeyPressed"));
  scm_call_3(func_symbol, scm_from_int(button), scm_from_int(action), scm_from_int(mods));
}
void onObjectSelected(short index){
  static SCM func_symbol = scm_variable_ref(scm_c_lookup("onObjSelected"));
  scm_call_1(func_symbol, scm_from_short(index));
}

SchemeBindingCallbacks createStaticSchemeBindings(
  std::string scriptPath,
	void (*moveCamera)(glm::vec3),  
	void (*rotateCamera)(float xoffset, float yoffset),
	void (*removeObjectById)(short id),
	void (*makeObjectV)(std::string, std::string, float, float, float),
	std::vector<short> (*getObjectsByType)(std::string),
	void (*setActiveCamera)(short cameraId),
  void (*drawText)(std::string word, float left, float top, unsigned int fontSize)
){
  scm_with_guile(&startGuile, NULL);
  
	moveCam = moveCamera;
	rotateCam = rotateCamera;
	removeObjById = removeObjectById;
	makeObj = makeObjectV;
	getObjByType = getObjectsByType;
	setActCamera = setActiveCamera;
  drawTextV = drawText;

  scm_c_define_gsubr("setCamera", 1, 0, 0, (void *)setActiveCam);
	scm_c_define_gsubr("movCam", 3, 0, 0, (void *)scmMoveCamera);
	scm_c_define_gsubr("rotCam", 2, 0, 0, (void *)scmRotateCamera);
	scm_c_define_gsubr("mkObj", 2, 1, 1, (void *)makeObject);
	scm_c_define_gsubr("rmObj", 1, 0, 0, (void *)removeObject);
	scm_c_define_gsubr("lsObjByType", 1, 0, 0, (void *)lsObjectsByType);
  scm_c_define_gsubr("drawText", 4, 0, 0, (void *)drawTextWords);

  scm_c_primitive_load(scriptPath.c_str());

  SchemeBindingCallbacks callbackFuncs = {
    .onFrame = onFrame,
    .onMouseCallback = onMouseCallback,
    .onObjectSelected = onObjectSelected,
  };

  return callbackFuncs;
}

void startShell(){
  scm_with_guile(&startGuile, NULL);
  int argc = 0;
  char* argv[] = { { } };
  scm_shell(argc, argv);
}

/* Notes:
   scm_c_eval_string("(define evaleddata \"some evaled data\")");
*/

