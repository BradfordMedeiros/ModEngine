#include "./scriptmanager.h"

static std::map<std::string, SCM> scriptnameToModule;

void loadScript(std::string script){  // scm_current_module()
  assert(scriptnameToModule.find(script) == scriptnameToModule.end());
  SCM module = scm_c_define_module("testmodulename", NULL, NULL);
  scriptnameToModule[script] = module;    // This probably will be per entity not 1:1 with script paths
  scm_set_current_module(module);
  defineFunctions();
  scm_c_primitive_load(script.c_str());
}
void unloadScript(std::string script){
  assert(scriptnameToModule.find(script) != scriptnameToModule.end());
  assert(false);    
}

void onFrameAllScripts(){
  for (auto &[_, module] : scriptnameToModule){
    scm_set_current_module(module);
    onFrame();
  }
}
void onCollisionEnterAllScripts(short obj1, short obj2){
  for (auto &[_, module] : scriptnameToModule){
    scm_set_current_module(module);
    onCollisionEnter(obj1, obj2);
  }
}
void onCollisionExitAllScripts(short obj1, short obj2){
  for (auto &[_, module] : scriptnameToModule){
    scm_set_current_module(module);
    onCollisionExit(obj1, obj2);
  }
}
void onMouseCallbackAllScripts(int button, int action, int mods){
  for (auto &[_, module] : scriptnameToModule){
    scm_set_current_module(module);
    onMouseCallback(button, action, mods);
  }
}
void onMouseMoveCallbackAllScripts(double xPos, double yPos){
  for (auto &[_, module] : scriptnameToModule){
    scm_set_current_module(module);
    onMouseMoveCallback(xPos, yPos);
  }
}
void onObjectSelectedAllScripts(short index){
  for (auto &[_, module] : scriptnameToModule){
    scm_set_current_module(module);
    onObjectSelected(index);
  }
}
void onKeyCallbackAllScripts(int key, int scancode, int action, int mods){
  for (auto &[_, module] : scriptnameToModule){
    scm_set_current_module(module);
    onKeyCallback(key, scancode, action, mods);
  }
}
void onKeyCharCallbackAllScripts(unsigned int codepoint){
  for (auto &[_, module] : scriptnameToModule){
    scm_set_current_module(module);
    onKeyCharCallback(codepoint);
  }
}
void onCameraSystemChangeAllScripts(bool usingBuiltInCamera){
  for (auto &[_, module] : scriptnameToModule){
    scm_set_current_module(module);
    onCameraSystemChange(usingBuiltInCamera);
  }
}

SchemeBindingCallbacks getSchemeCallbacks(){
  SchemeBindingCallbacks callbackFuncs = {
    .onFrame = onFrameAllScripts,
    .onCollisionEnter = onCollisionEnterAllScripts,
    .onCollisionExit = onCollisionExitAllScripts,
    .onMouseCallback = onMouseCallbackAllScripts,
    .onMouseMoveCallback = onMouseMoveCallbackAllScripts,
    .onObjectSelected = onObjectSelectedAllScripts,
    .onKeyCallback = onKeyCallbackAllScripts,
    .onKeyCharCallback = onKeyCharCallbackAllScripts,
    .onCameraSystemChange = onCameraSystemChangeAllScripts,
  };




 
  //auto topLevelHasLs = symbolDefinedInModule("onFrame", topLevelModule);
  //auto someLevelHasLs = symbolDefinedInModule("lsclips", somemoduleRef);

  //scm_set_current_module(somemoduleRef);

  return callbackFuncs;
}