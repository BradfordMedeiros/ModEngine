#ifndef MOD_SCHEMEBINDINGS
#define MOD_SCHEMEBINDINGS

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <libguile.h>
#include <limits>       

typedef void(*func)();
typedef void(*colfun)(short obj1, short obj2);
typedef void(*mousecallback)(int button, int action, int mods);
typedef void(*keycallback)(int key, int scancode, int action, int mods);
typedef void(*keycharcallback)(unsigned int codepoint);
typedef void(*onobjectSelectedFunc)(short index);

struct SchemeBindingCallbacks {
    func onFrame;
    colfun onCollisionEnter;
    colfun onCollisionExit;
    mousecallback onMouseCallback;
    onobjectSelectedFunc onObjectSelected;
    keycallback onKeyCallback;
    keycharcallback onKeyCharCallback;
};

SchemeBindingCallbacks createStaticSchemeBindings(
	std::string scriptPath,
  short (*loadScene)(std::string),  
  void (*unloadScene)(short id),  
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
  glm::quat (*getGameObjectRot)(short index),
  void (*setGameObjectRot)(short index, glm::quat rotation),
  glm::quat (*setFrontDelta)(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta),
	short (*getGameObjectByName)(std::string name),
	void (*setSelectionMode)(bool enabled),
  void (*applyImpulse)(short index, glm::vec3 impulse),
  void (*clearImpulse)(short index)
);

#endif
