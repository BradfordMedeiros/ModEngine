#ifndef MOD_SCRIPTMANAGER
#define MOD_SCRIPTMANAGER

#include <map>
#include <string>
#include <vector>
#include "./scheme_bindings.h"

typedef void(*func)();
typedef void(*colposfun)(short obj1, short obj2, glm::vec3 pos);
typedef void(*colfun)(short obj1, short obj2);
typedef void(*mousecallback)(int button, int action, int mods);
typedef void(*mousemovecallback)(double xPos, double yPos);
typedef void(*keycallback)(int key, int scancode, int action, int mods);
typedef void(*keycharcallback)(unsigned int codepoint);
typedef void(*onobjectSelectedFunc)(short index);
typedef void(*boolfunc)(bool value);
typedef void(*messagefunc)(std::vector<std::string>&);
typedef void(*stringfunc)(std::string);

struct SchemeBindingCallbacks {
  func onFrame;
  colposfun onCollisionEnter;
  colfun onCollisionExit;
  mousecallback onMouseCallback;
  mousemovecallback onMouseMoveCallback;
  onobjectSelectedFunc onObjectSelected;
  keycallback onKeyCallback;
  keycharcallback onKeyCharCallback;
  boolfunc onCameraSystemChange;
  messagefunc onMessage;
  stringfunc onTcpMessage;
  stringfunc onUdpMessage;
  stringfunc onPlayerJoined;
  stringfunc onPlayerLeave;
};

SchemeBindingCallbacks getSchemeCallbacks();
void loadScript(std::string script, objid id);
void unloadScript(std::string script);

#endif