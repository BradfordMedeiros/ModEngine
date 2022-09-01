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
  id_func onObjectUnselected;
  id_onobjectHoverFunc onObjectHover;
  id_funcMappingFunc onMapping;
  id_keycallback onKeyCallback;
  id_keycharcallback onKeyCharCallback;
  id_stringboolFunc onCameraSystemChange;
  id_string2func onMessage;
  id_stringfunc onTcpMessage;
  id_stringfunc onUdpMessage;
  id_stringfunc onPlayerJoined;
  id_stringfunc onPlayerLeave;
};

objid currentModuleId();
objid currentSceneId();
SchemeBindingCallbacks getSchemeCallbacks();
void loadScript(std::string script, objid id, objid sceneId, bool isServer, bool isFreeScript, std::function<std::string(std::string)> pathForModLayer);
void unloadScript(std::string script, objid id);
void unloadScriptsCleanup();

#endif
