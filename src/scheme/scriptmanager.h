#ifndef MOD_SCRIPTMANAGER
#define MOD_SCRIPTMANAGER

#include <map>
#include <string>
#include <vector>
#include <queue>          
#include <functional>
#include "./scheme_bindings.h"

typedef void(*func)();
typedef void(*colposfun)(int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal);
typedef void(*colfun)(int32_t obj1, int32_t obj2);
typedef void(*mousecallback)(int button, int action, int mods);
typedef void(*mousemovecallback)(double xPos, double yPos);
typedef void(*scrollcallback)(double amount);
typedef void(*keycallback)(int key, int scancode, int action, int mods);
typedef void(*keycharcallback)(unsigned int codepoint);
typedef void(*onobjectSelectedFunc)(int32_t index, glm::vec3 color);
typedef void(*onobjectHoverFunc)(int32_t index, bool hoverOn);
typedef void(*boolfunc)(bool value);
typedef void(*messagefunc)(std::queue<StringString>&);
typedef void(*stringfunc)(std::string&);

struct SchemeBindingCallbacks {
  func onFrame;
  colposfun onCollisionEnter;
  colfun onCollisionExit;
  mousecallback onMouseCallback;
  mousemovecallback onMouseMoveCallback;
  scrollcallback onScrollCallback;
  onobjectSelectedFunc onObjectSelected;
  onobjectHoverFunc onObjectHover;
  keycallback onKeyCallback;
  keycharcallback onKeyCharCallback;
  boolfunc onCameraSystemChange;
  messagefunc onMessage;
  stringfunc onTcpMessage;
  stringfunc onUdpMessage;
  stringfunc onPlayerJoined;
  stringfunc onPlayerLeave;
};

objid currentModuleId();
objid currentSceneId();
SchemeBindingCallbacks getSchemeCallbacks();
void loadScript(std::string script, objid id, objid sceneId, bool isServer, bool isFreeScript);
void unloadScript(std::string script, objid id, std::function<void()> additionalUnload);

#endif