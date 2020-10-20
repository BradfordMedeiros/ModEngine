#ifndef MOD_SCRIPTMANAGER
#define MOD_SCRIPTMANAGER

#include <map>
#include <string>
#include <vector>
#include <queue>          
#include "./scheme_bindings.h"

typedef void(*func)();
typedef void(*colposfun)(int32_t obj1, int32_t obj2, glm::vec3 pos);
typedef void(*colfun)(int32_t obj1, int32_t obj2);
typedef void(*mousecallback)(int button, int action, int mods);
typedef void(*mousemovecallback)(double xPos, double yPos);
typedef void(*keycallback)(int key, int scancode, int action, int mods);
typedef void(*keycharcallback)(unsigned int codepoint);
typedef void(*onobjectSelectedFunc)(int32_t index, glm::vec3 color);
typedef void(*onobjectHoverFunc)(int32_t index, bool hoverOn);
typedef void(*boolfunc)(bool value);
typedef void(*messagefunc)(std::queue<std::string>&);
typedef void(*messagefloatfunc)(std::queue<StringFloat>&);
typedef void(*stringfunc)(std::string);

struct SchemeBindingCallbacks {
  func onFrame;
  colposfun onCollisionEnter;
  colfun onCollisionExit;
  mousecallback onMouseCallback;
  mousemovecallback onMouseMoveCallback;
  onobjectSelectedFunc onObjectSelected;
  onobjectHoverFunc onObjectHover;
  keycallback onKeyCallback;
  keycharcallback onKeyCharCallback;
  boolfunc onCameraSystemChange;
  messagefunc onMessage;
  messagefloatfunc onFloatMessage;
  stringfunc onTcpMessage;
  stringfunc onUdpMessage;
  stringfunc onPlayerJoined;
  stringfunc onPlayerLeave;
};

SchemeBindingCallbacks getSchemeCallbacks();
void loadScript(std::string script, objid id, bool isServer);
void unloadScript(std::string script, objid id);

#endif