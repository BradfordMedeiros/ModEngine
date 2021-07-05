#include "./scriptmanager.h"

struct ScriptModule {
  objid id;
  objid sceneId;
  SCM module;
};
static std::map<std::string, ScriptModule> scriptnameToModule;

// IMPORTANT BUG --> the create/destroy of modules probably a huge memory leak. 
// Need to figure out how to properly bring the up/down (global module tree?)

std::string getScriptName(std::string scriptpath, objid id){
  return scriptpath + ":" + std::to_string(id);
}

objid currentModuleId(){
  SCM module = scm_current_module();
  for (auto &[script, scriptModule] : scriptnameToModule){
    std::cout << "Checking module: " << scriptModule.id << std::endl;
    if (module == scriptModule.module){
      return scriptModule.id;
    }
  }
  assert(false);
}

objid currentSceneId(){
  SCM module = scm_current_module();
  for (auto &[script, scriptModule] : scriptnameToModule){
    std::cout << "Checking module: " << scriptModule.id << std::endl;
    if (module == scriptModule.module){
      return scriptModule.sceneId;
    }
  }
  assert(false); 
}

void loadScript(std::string scriptpath, objid id, objid sceneId, bool isServer){ 
  auto script = getScriptName(scriptpath, id);

  std::cout << "SYSTEM: LOADING SCRIPT: (" << script << ", " << id << ")" << std::endl;
  assert(scriptnameToModule.find(script) == scriptnameToModule.end());
  SCM module = scm_c_define_module(script.c_str(), NULL, NULL);         // should think about what we should name the module
  scriptnameToModule[script] = ScriptModule {
    .id = id,
    .sceneId = sceneId,
    .module = module,
  };                    
  scm_set_current_module(module);
  defineFunctions(id, isServer);
  scm_c_primitive_load(scriptpath.c_str());
  onFrame();
}

// @TODO -- need to figure out how to really unload a module.
// I don't think this actually causes this module to be garbage collected.
void unloadScript(std::string scriptpath, objid id){
  auto script = getScriptName(scriptpath, id);
  auto module = scriptnameToModule.at(script).module;
  scm_set_current_module(module);
  removeStateMachines(currentModuleId());
  onScriptUnload();

  std::cout << "SYSTEM: UNLOADING SCRIPT: (" << script << ", " << id << ")" << std::endl;
  assert(scriptnameToModule.find(script) != scriptnameToModule.end());
  scriptnameToModule.erase(script);
}

void onFrameAllScripts(){
  for (auto &[script, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
    onFrame();
  }
}

void onCollisionEnterAllScripts(int32_t obj1, int32_t obj2, glm::vec3 pos, glm::quat normal){
  for (auto &[_, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
    onCollisionEnter(obj1, obj2, pos, normal);
  }
}
void onCollisionExitAllScripts(int32_t obj1, int32_t obj2){
  for (auto &[_, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
    onCollisionExit(obj1, obj2);
  }
}
void onMouseCallbackAllScripts(int button, int action, int mods){
  for (auto &[_, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
    onMouseCallback(button, action, mods);
  }
}
void onMouseMoveCallbackAllScripts(double xPos, double yPos){
  for (auto &[_, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
    onMouseMoveCallback(xPos, yPos);
  }
}
void onScrollCallbackAllScripts(double amount){
  for (auto &[_, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
    onScrollCallback(amount);
  }
}

void onObjectSelectedAllScripts(int32_t index, glm::vec3 color){
  for (auto &[_, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
    onObjectSelected(index, color);
  }
}
void onObjectHoverAllScripts(int32_t index, bool isHover){
  for (auto &[_, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
    if (isHover){
      onObjectHover(index);
    }else{
      onObjectUnhover(index);
    }
  }  
}
void onKeyCallbackAllScripts(int key, int scancode, int action, int mods){
  for (auto &[_, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
    onKeyCallback(key, scancode, action, mods);
  }
}
void onKeyCharCallbackAllScripts(unsigned int codepoint){
  for (auto &[_, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
    onKeyCharCallback(codepoint);
  }
}
void onCameraSystemChangeAllScripts(bool usingBuiltInCamera){
  for (auto &[_, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
    onCameraSystemChange(usingBuiltInCamera);
  }
}
void onMessageAllScripts(std::queue<StringString>& messages){
  while (!messages.empty()){
    auto message = messages.front();
    messages.pop();

    for (auto &[name, scriptModule] : scriptnameToModule){
      scm_set_current_module(scriptModule.module);
      onAttrMessage(message.strTopic, message.strValue);
    }
  }
}

void onTcpMessageAllScripts(std::string& message){
  for (auto &[_, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
    onTcpMessage(message);
  } 
}
void onUdpMessageAllScripts(std::string& message){
  for (auto &[_, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
    onUdpMessage(message);
  } 
}

void onPlayerJoinedAllScripts(std::string& connectionHash){
  for (auto &[_, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
    onPlayerJoined(connectionHash);
  } 
}
void onPlayerLeaveAllScripts(std::string& connectionHash){
  for (auto &[_, scriptModule] : scriptnameToModule){
    scm_set_current_module(scriptModule.module);
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
    .onScrollCallback = onScrollCallbackAllScripts,
    .onObjectSelected = onObjectSelectedAllScripts,
    .onObjectHover = onObjectHoverAllScripts,
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