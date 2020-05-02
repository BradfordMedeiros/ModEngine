#ifndef MOD_SCRIPTMANAGER
#define MOD_SCRIPTMANAGER

#include <map>
#include <string>
#include <vector>
#include "./scheme_bindings.h"

typedef void(*func)();
typedef void(*colfun)(short obj1, short obj2);
typedef void(*mousecallback)(int button, int action, int mods);
typedef void(*mousemovecallback)(double xPos, double yPos);
typedef void(*keycallback)(int key, int scancode, int action, int mods);
typedef void(*keycharcallback)(unsigned int codepoint);
typedef void(*onobjectSelectedFunc)(short index);
typedef void(*boolfunc)(bool value);
typedef void(*messagefunc)(std::vector<std::string>&);

struct SchemeBindingCallbacks {
  func onFrame;
  colfun onCollisionEnter;
  colfun onCollisionExit;
  mousecallback onMouseCallback;
  mousemovecallback onMouseMoveCallback;
  onobjectSelectedFunc onObjectSelected;
  keycallback onKeyCallback;
  keycharcallback onKeyCharCallback;
  boolfunc onCameraSystemChange;
  messagefunc onMessage;
};

SchemeBindingCallbacks getSchemeCallbacks();
void loadScript(std::string script);

#endif