#ifndef MOD_SCRIPTMANAGER
#define MOD_SCRIPTMANAGER

#include <map>
#include <string>
#include <vector>
#include <queue>          
#include <functional>
#include "./scheme_bindings.h"
#include "../../../common/util.h"

struct SchemeBindingCallbacks {
  id_func onFrame;
  id_colposfun onCollisionEnter;
  id_colfun onCollisionExit;
  id_mousecallback onMouseCallback;
  id_mousemovecallback onMouseMoveCallback;
  id_scrollcallback onScrollCallback;
  id_onobjectSelectedFunc onObjectSelected;
  id_onobjectHoverFunc onObjectHover;
  id_keycallback onKeyCallback;
  
  keycharcallback onKeyCharCallback;
  stringboolFunc onCameraSystemChange;
  string2func onMessage;
  stringfunc onTcpMessage;
  stringfunc onUdpMessage;
  stringfunc onPlayerJoined;
  stringfunc onPlayerLeave;
};

objid currentModuleId();
objid currentSceneId();
SchemeBindingCallbacks getSchemeCallbacks();
void loadScript(std::string script, objid id, objid sceneId, bool isServer, bool isFreeScript);
void unloadScript(std::string script, objid id);
void unloadScriptsCleanup();

#endif
