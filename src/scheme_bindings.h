#ifndef MOD_SCHEMEBINDINGS
#define MOD_SCHEMEBINDINGS

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <libguile.h>
#include <limits>       

typedef void(*func)();
typedef void(*mousecallback)(int button, int action, int mods);
typedef void(*keycallback)(int key, int scancode, int action, int mods);
typedef void(*onobjectSelectedFunc)(short index);

struct SchemeBindingCallbacks {
    func onFrame;
    mousecallback onMouseCallback;
    onobjectSelectedFunc onObjectSelected;
    keycallback onKeyCallback;
};

SchemeBindingCallbacks createStaticSchemeBindings(
	std::string scriptPath,
	void (*moveCamera)(glm::vec3),  
	void (*rotateCamera)(float xoffset, float yoffset),
	void (*removeObjectById)(short id),
	void (*makeObjectV)(std::string, std::string, float, float, float),
	std::vector<short> (*getObjectsByType)(std::string),
	void (*setActiveCamera)(short cameraId),
	void (*drawText)(std::string word, float left, float top, unsigned int fontSize),
	glm::vec3 (*getGameObjectPos)(short index),
	void (*setGameObjectPos)(short index, glm::vec3 pos)

);

void startShell();		// this isn't thread safe and dont feel like writing thread sync code atm

#endif