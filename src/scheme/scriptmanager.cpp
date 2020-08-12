#include "./scriptmanager.h"

static std::map<std::string, SCM> scriptnameToModule;

// IMPORTANT BUG --> the create/destroy of modules probably a huge memory leak. 
// Need to figure out how to properly bring the up/down (global module tree?)

void loadScript(std::string script, objid id){ 
  std::cout << "SYSTEM: LOADING SCRIPT: (" << script << ", " << id << ")" << std::endl;
  assert(scriptnameToModule.find(script) == scriptnameToModule.end());
  SCM module = scm_c_define_module(script.c_str(), NULL, NULL);         // should think about what we should name the module
  scriptnameToModule[script] = module;                                  // This probably will be per entity not 1:1 with script paths
  scm_set_current_module(module);
  defineFunctions(id);
  scm_c_primitive_load(script.c_str());
  onFrame();
}

// @TODO -- need to figure out how to really unload a module.
// I don't think this actually causes this module to be garbage collected.
void unloadScript(std::string script, objid id){
  std::cout << "SYSTEM: UNLOADING SCRIPT: (" << script << ", " << id << ")" << std::endl;
  assert(scriptnameToModule.find(script) != scriptnameToModule.end());
  scriptnameToModule.erase(script);
}

void onFrameAllScripts(){
  for (auto &[script, module] : scriptnameToModule){
    scm_set_current_module(module);
    onFrame();
  }
}

void onCollisionEnterAllScripts(short obj1, short obj2, glm::vec3 pos){
  for (auto &[_, module] : scriptnameToModule){
    scm_set_current_module(module);
    onCollisionEnter(obj1, obj2, pos);
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
void onMessageAllScripts(std::queue<std::string>& messages){
  while (!messages.empty()){
    auto message = messages.front();
    messages.pop();
    for (auto &[name, module] : scriptnameToModule){
      scm_set_current_module(module);
      onMessage(message);
    }
  }
}

void onTcpMessageAllScripts(std::string message){
  for (auto &[_, module] : scriptnameToModule){
    scm_set_current_module(module);
    onTcpMessage(message);
  } 
}
void onUdpMessageAllScripts(std::string message){
  for (auto &[_, module] : scriptnameToModule){
    scm_set_current_module(module);
    onUdpMessage(message);
  } 
}

void onPlayerJoinedAllScripts(std::string connectionHash){
  for (auto &[_, module] : scriptnameToModule){
    scm_set_current_module(module);
    onPlayerJoined(connectionHash);
  } 
}
void onPlayerLeaveAllScripts(std::string connectionHash){
  for (auto &[_, module] : scriptnameToModule){
    scm_set_current_module(module);
    onPlayerLeave(connectionHash);
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
    .onMessage = onMessageAllScripts,
    .onTcpMessage = onTcpMessageAllScripts,
    .onUdpMessage = onUdpMessageAllScripts,
    .onPlayerJoined = onPlayerJoinedAllScripts,
    .onPlayerLeave = onPlayerLeaveAllScripts,
  };

  return callbackFuncs;
}