#include "./scheme_bindings.h"


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

void (*makeObj)(float, float, float);
SCM makeObject(SCM value, SCM position, SCM rest){
  //int isKeyword = scm_is_keyword(keyword1);

  //std::string type = scm_to_locale_string(scm_symbol_to_string (scm_keyword_to_symbol (keyword1)));
  //std::cout << "is keyword: " << isKeyword << std::endl;

  //std::string objValue = scm_to_locale_string(value);
  //std::cout << "objValue is: " << objValue << std::endl;
  //SCM positionList = scm_list_ref (rest, scm_from_int64 (0));
  //std::string elString =  scm_to_locale_string (firstElement);
  //std::cout << "el string is: " << elString << std::endl;

  auto xPos = scm_to_double(scm_list_ref(position, scm_from_int64(0)));
  auto yPos = scm_to_double(scm_list_ref(position, scm_from_int64(1)));
  auto zPos = scm_to_double(scm_list_ref(position, scm_from_int64(2)));

  std::cout << "(" << xPos << ", " << yPos << ", " << zPos << ")" << std::endl;

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

void (*setActCamera)(short id);
SCM setActiveCam(SCM value){
  short id = scm_to_short(value);
  setActCamera(id);
  return SCM_UNSPECIFIED;
}

void createStaticSchemeBindings(
	void (*moveCamera)(glm::vec3),  
	void (*rotateCamera)(float xoffset, float yoffset),
	void (*removeObjectById)(short id),
	void (*makeObjectV)(float, float, float),
	std::vector<short> (*getObjectsByType)(std::string),
	void (*setActiveCamera)(short cameraId)
){
	std::cout << "scheme bindings placeholder here" << std::endl;
	initGuile();

	moveCam = moveCamera;
	rotateCam = rotateCamera;
	removeObjById = removeObjectById;
	makeObj = makeObjectV;
	getObjByType = getObjectsByType;
	setActCamera = setActiveCamera;

	scm_c_define_gsubr("movCam", 3, 0, 0, (void *)scmMoveCamera);
	scm_c_define_gsubr("rotCam", 2, 0, 0, (void *)scmRotateCamera);
	scm_c_define_gsubr("mkObj", 1, 1, 1, (void *)makeObject);
	scm_c_define_gsubr("rmObj", 1, 0, 0, (void *)removeObject);
	scm_c_define_gsubr("lsObjByType", 1, 0, 0, (void *)lsObjectsByType);
	scm_c_define_gsubr("setCamera", 1, 0, 0, (void *)setActiveCam);
}

void startShell(){
	startShellForNewThread();
}



