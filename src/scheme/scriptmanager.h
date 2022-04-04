#ifndef MOD_SCRIPTMANAGER
#define MOD_SCRIPTMANAGER

#include <map>
#include <string>
#include <vector>
#include <queue>          
#include <functional>
#include "./scheme_bindings.h"

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
  stringboolFunc onCameraSystemChange;
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
void unloadScriptsCleanup();

#endif