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
void afterFrameForScripts();

struct CScriptBindingCallbacks {
  func onFrame;
  func onFrameAfterUpdate;
  colposfun onCollisionEnter;
  colfun onCollisionExit;
  mousecallback onMouseCallback;
  mousemovecallback onMouseMoveCallback;
  scrollcallback onScrollCallback;
  onobjectSelectedFunc onObjectSelected;
  func onObjectUnselected;
  onobjectHoverFunc onObjectHover;
  keycallback onKeyCallback;
  keycharcallback onKeyCharCallback;
  std::function<void(int, bool)> onController;
  std::function<void(int, BUTTON_TYPE, bool)> onControllerKey;

  stringboolFunc onCameraSystemChange;
  string2func onMessage;

  std::function<void(objid)> onObjectAdded;
  std::function<void(objid)> onObjectRemoved;
};

CScriptBindingCallbacks getCScriptBindingCallbacks();


#endif