#include "./scriptmanager.h"

struct ScriptModule {
  objid id;
  objid sceneId;
  SCM module;
  bool isvalid;
};
static std::map<std::string, ScriptModule> scriptnameToModule;

std::string scriptnameForId(objid id){
  for (auto &[name, module] : scriptnameToModule){
    if (module.id == id){
      return name;
    }
  }
  assert(false);
  return "";
}
ScriptModule& moduleForId(objid id){
  return scriptnameToModule.at(scriptnameForId(id));
}

// IMPORTANT BUG --> the create/destroy of modules probably a huge memory leak. 
// Need to figure out how to properly bring the up/down (global module tree?)

std::string getScriptName(std::string scriptpath, objid id){
  return scriptpath + ":" + std::to_string(id);
}

objid currentModuleId(){
  SCM module = scm_current_module();
  for (auto &[script, scriptModule] : scriptnameToModule){
    //std::cout << "Current module id: Checking module: " << scriptModule.id << std::endl;
    if (module == scriptModule.module){
      return scriptModule.id;
    }
  }
  assert(false);
}

objid currentSceneId(){
  SCM module = scm_current_module();
  for (auto &[script, scriptModule] : scriptnameToModule){
    std::cout << "Current scene id: Checking module: " << scriptModule.id << std::endl;
    if (module == scriptModule.module){
      return scriptModule.sceneId;
    }
  }
  assert(false); 
}

void loadScript(std::string scriptpath, objid id, objid sceneId, bool isServer, bool isFreeScript, std::function<std::string(std::string)> pathForModLayer){ 
  auto script = getScriptName(scriptpath, id);
  SCM oldModule = scm_current_module();

  std::cout << "SYSTEM: LOADING SCRIPT: (" << script << ", " << id << ")" << std::endl;
  assert(scriptnameToModule.find(script) == scriptnameToModule.end());
  SCM module = scm_c_define_module(script.c_str(), NULL, NULL);         // should think about what we should name the module
  scriptnameToModule[script] = ScriptModule {
    .id = id,
    .sceneId = sceneId,
    .module = module,
    .isvalid = true,
  };                    

  scm_set_current_module(module);
  defineFunctions(id, isServer, isFreeScript);
  scm_c_primitive_load(pathForModLayer(scriptpath).c_str());
  onFrame();
  scm_set_current_module(oldModule); // because this might be inside of another script load function,
}

// @TODO -- need to figure out how to really unload a module.
// I don't know this actually causes this module to be garbage collected.
void unloadScript(std::string scriptpath, objid id){
  auto script = getScriptName(scriptpath, id);
  auto module = scriptnameToModule.at(script).module;
  scm_set_current_module(module);

  onScriptUnload();

  std::cout << "SYSTEM: UNLOADING SCRIPT: (" << script << ", " << id << ")" << std::endl;
  assert(scriptnameToModule.find(script) != scriptnameToModule.end());
  scriptnameToModule.at(script).isvalid = false;
}
void unloadScriptsCleanup(){
  std::vector<std::string> scriptsToRemove;
  for (auto &[script, scriptModule] : scriptnameToModule){
    if (!scriptModule.isvalid){
      scriptsToRemove.push_back(script);
    }
  } 
  for (auto script : scriptsToRemove){
    scriptnameToModule.erase(script);
  }
}

void onFrameAllScripts(objid scriptId, void* data){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onFrame();
}

void onCollisionEnterAllScripts(objid scriptId, void* data, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  if (scriptId == obj1){
    onCollisionEnter(obj2, pos, oppositeNormal);
  }else if (scriptId == obj2){
    onCollisionEnter(obj1, pos, normal);
  }
  onGlobalCollisionEnter(obj1, obj2, pos, normal, oppositeNormal);
}
void onCollisionExitAllScripts(objid scriptId, void* data, int32_t obj1, int32_t obj2){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  if (scriptId == obj1){
    onCollisionExit(obj2);
  }else if (scriptId == obj2){
    onCollisionExit(obj1);
  } 
  onGlobalCollisionExit(obj1, obj2);
}

void onMouseCallbackAllScripts(objid scriptId, int button, int action, int mods){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onMouseCallback(button, action, mods);
}

void onMouseMoveCallbackAllScripts(objid scriptId, void* data, double xPos, double yPos, float xNdc, float yNdc){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onMouseMoveCallback(xPos, yPos, xNdc, yNdc);
}
void onScrollCallbackAllScripts(objid scriptId, double amount){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onScrollCallback(amount);  
}

void onObjectSelectedAllScripts(objid scriptId, int32_t index, glm::vec3 color){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onObjectSelected(index, color);
}
void onObjectUnselectedAllScripts(objid scriptId){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onObjectUnselected();
}
void onObjectHoverAllScripts(int32_t scriptId, int32_t index, bool isHover){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  if (isHover){
    onObjectHover(index);
  }else{
    onObjectUnhover(index);
  }
}
void onMappingAllScripts(int32_t scriptId, int32_t index){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onMapping(index);
}

void onKeyCallbackAllScripts(int32_t scriptId, void* data, int key, int scancode, int action, int mods){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onKeyCallback(key, scancode, action, mods);  
}
void onKeyCharCallbackAllScripts(int32_t scriptId, unsigned int codepoint){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onKeyCharCallback(codepoint);
}

void onCameraSystemChangeAllScripts(int32_t scriptId, std::string camera, bool usingBuiltInCamera){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onCameraSystemChange(camera, usingBuiltInCamera);  
}

void onMessageAllScripts(objid scriptId, void* data, std::string& topic, AttributeValue& value){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onAttrMessage(topic, value);
}

void onTcpMessageAllScripts(objid scriptId, std::string& message){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onTcpMessage(message);  
}
void onUdpMessageAllScripts(objid scriptId, std::string& message){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onUdpMessage(message);
}

void onPlayerJoinedAllScripts(objid scriptId, std::string& connectionHash){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onPlayerJoined(connectionHash);
}
void onPlayerLeaveAllScripts(objid scriptId, std::string& connectionHash){
  auto scriptModule = moduleForId(scriptId);
  if (!scriptModule.isvalid){
    return;
  }
  scm_set_current_module(scriptModule.module);
  onPlayerLeave(connectionHash);
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
    .onObjectUnselected = onObjectUnselectedAllScripts,
    .onObjectHover = onObjectHoverAllScripts,
    .onMapping = onMappingAllScripts,
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