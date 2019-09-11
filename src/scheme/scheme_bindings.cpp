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
SCM makeObject(SCM value){
  std::cout << "placeholder make object" << std::endl;
  return SCM_UNSPECIFIED;
}

void (*removeObjById)(short id);
SCM removeObject(SCM value){
  removeObjById(scm_to_short(value));
  return SCM_UNSPECIFIED;
}

void (*getObjByType)();
SCM getObjectsByType(SCM value){
  std::cout << "placeholder get obj by type" << std::endl;
  return SCM_UNSPECIFIED;
}

void createStaticSchemeBindings(
	void(*moveCamera)(glm::vec3), 
	void(*rotateCamera)(float xoffset, float yoffset),
	void (*removeObjectById)(short id),
	void (*getObjectsByType)()
){
	std::cout << "scheme bindings placeholder here" << std::endl;
	initGuile();

	moveCam = moveCamera;
	rotateCam = rotateCamera;
	removeObjById = removeObjectById;
	getObjByType = getObjectsByType;

	scm_c_define_gsubr("movCam", 3, 0, 0, (void *)scmMoveCamera);
	scm_c_define_gsubr("rotCam", 2, 0, 0, (void *)scmRotateCamera);
	scm_c_define_gsubr("mkObj", 1, 0, 0, (void *)makeObject);
	scm_c_define_gsubr("rmObj", 1, 0, 0, (void *)removeObject);
	scm_c_define_gsubr("lsObjByType", 1, 0, 0, (void *)getObjectsByType);
}

void startShell(){
	startShellForNewThread();
}



