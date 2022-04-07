#ifndef MOD_CSCRIPT
#define MOD_CSCRIPT

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <assert.h>
#include <regex>
#include "./cscript_binding.h"

void registerAllBindings(std::vector<CScriptBinding> pluginBindings);
void loadCScript(int id, const char* name, int sceneId, bool bootstrapperMode, bool isFreeScript);
void unloadCScript(int id);
void renderCustomObj(int id);

struct CScriptBindingCallbacks {
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

CScriptBindingCallbacks getCScriptBindingCallbacks();


#endif