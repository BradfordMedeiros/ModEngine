#include "./cscript.h"

/* This should be temporary to try out rendering in C++ 
   Should port the scheme scripting to C++, and then these just become ordinary api methods
*/

std::vector<CScriptBinding> bindings = {};

void registerAllBindings(std::vector<CScriptBinding> pluginBindings){
  std::set<std::string> names;
  for (auto &plugin : pluginBindings){
    bindings.push_back(plugin);
    if (names.count(plugin.bindingMatcher) > 0){
      std::cout << "plugin matcher duplicate: " << plugin.bindingMatcher << std::endl;
      assert(false);
    }
    names.insert(plugin.bindingMatcher);
  }
}

struct CustomObjInstance {
  std::string name;
  void* data;
  CScriptBinding* cScriptBinding;
};
std::map<int, CustomObjInstance> customObjInstances = {};

bool patternMatchesScriptName(std::string pattern, std::string scriptname){
  std::basic_regex reg(pattern);
  auto matches = std::regex_match(scriptname, reg);
  return matches;
}

/* matching strategy: matches based upon registration order, * is a wildcard which matches anything*/
CScriptBinding* getCScriptBinding(const char* name){
  for (auto &customObj : bindings){
    if (patternMatchesScriptName(customObj.bindingMatcher, name)){
      return &customObj;
    }
  }
  assert(false);
  return NULL;
}

void loadCScript(int id, const char* name, int sceneId, bool bootstrapperMode, bool isFreeScript){
  assert(customObjInstances.find(id) == customObjInstances.end());
  auto binding = getCScriptBinding(name);
  auto data = binding -> create(name, id, sceneId, bootstrapperMode, isFreeScript);
  customObjInstances[id] = CustomObjInstance {
    .name = name,
    .data = data,
    .cScriptBinding = binding,
  };
}
void unloadCScript(int id){
  auto instanceExists = customObjInstances.find(id) != customObjInstances.end();
  if (instanceExists){
    auto objInstance = customObjInstances.at(id);
    auto binding = objInstance.cScriptBinding;
    binding -> remove(objInstance.name, id, objInstance.data); 
    customObjInstances.erase(id);
  }
}
void renderCustomObj(int id){
  auto instanceExists = customObjInstances.find(id) != customObjInstances.end();
  if (instanceExists){
    auto objInstance = customObjInstances.at(id);
    auto binding = objInstance.cScriptBinding;
    binding -> render(objInstance.data);   
  }
}


//// Callbacks ////

void onCFrameAllScripts(){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onFrame(instanceId, objInstance.data);
  }
}

void onCCollisionEnterAllScripts(int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onCollisionEnter(instanceId, objInstance.data, obj1, obj2, pos, normal, oppositeNormal);
  }
}
void onCCollisionExitAllScripts(int32_t obj1, int32_t obj2){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onCollisionExit(instanceId, objInstance.data, obj1, obj2);
  }
}
void onCMouseCallbackAllScripts(int button, int action, int mods){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onMouseCallback(instanceId, objInstance.data, button, action, mods);
  }
}
void onCMouseMoveCallbackAllScripts(double xPos, double yPos, float xNdc, float yNdc){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onMouseMoveCallback(instanceId, objInstance.data, xPos, yPos, xNdc, yNdc);
  }
}
void onCScrollCallbackAllScripts(double amount){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onScrollCallback(instanceId, objInstance.data, amount);
  }
}
void onCObjectSelectedAllScripts(int32_t index, glm::vec3 color){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onObjectSelected(instanceId, objInstance.data, index, color);
  }
}
void onCObjectUnselectedAllScripts(){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onObjectUnselected(instanceId, objInstance.data);
  }
}
void onCObjectHoverAllScripts(int32_t index, bool isHover){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onObjectHover(instanceId, objInstance.data, index, isHover);
  }
}
void onCMappingAllScripts(int32_t index){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onMapping(instanceId, objInstance.data, index);
  }
}

void onCKeyCallbackAllScripts(int key, int scancode, int action, int mods){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onKeyCallback(instanceId, objInstance.data, key, scancode, action, mods);
  }
}
void onCKeyCharCallbackAllScripts(unsigned int codepoint){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onKeyCharCallback(instanceId, objInstance.data, codepoint);
  }
}

void onCCameraSystemChangeAllScripts(std::string camera, bool usingBuiltInCamera){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onCameraSystemChange(instanceId, camera, usingBuiltInCamera);
  }
}
void onCMessageAllScripts(std::string& topic, AttributeValue& value){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onMessage(instanceId, objInstance.data, topic, value);
  }
}

void onCTcpMessageAllScripts(std::string& message){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onTcpMessage(instanceId, message);
  }
}
void onCUdpMessageAllScripts(std::string& message){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onUdpMessage(instanceId, message);
  }
}

void onCPlayerJoinedAllScripts(std::string& connectionHash){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onPlayerJoined(instanceId, connectionHash);
  }
}
void onCPlayerLeaveAllScripts(std::string& connectionHash){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onPlayerLeave(instanceId, connectionHash);
  }
}
void onCObjectAddedAllScripts(objid idAdded){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onObjectAdded(instanceId, objInstance.data, idAdded);
  }
}
void onCObjectRemovedAllScripts(objid idRemoved){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onObjectRemoved(instanceId, objInstance.data, idRemoved);
  }
}

CScriptBindingCallbacks getCScriptBindingCallbacks(){
  return CScriptBindingCallbacks {
    .onFrame = onCFrameAllScripts,
    .onCollisionEnter = onCCollisionEnterAllScripts,
    .onCollisionExit = onCCollisionExitAllScripts,
    .onMouseCallback = onCMouseCallbackAllScripts,
    .onMouseMoveCallback = onCMouseMoveCallbackAllScripts,
    .onScrollCallback = onCScrollCallbackAllScripts,
    .onObjectSelected = onCObjectSelectedAllScripts,
    .onObjectUnselected = onCObjectUnselectedAllScripts,
    .onObjectHover = onCObjectHoverAllScripts,
    .onMapping = onCMappingAllScripts,
    .onKeyCallback = onCKeyCallbackAllScripts,
    .onKeyCharCallback = onCKeyCharCallbackAllScripts,
    .onCameraSystemChange = onCCameraSystemChangeAllScripts,
    .onMessage = onCMessageAllScripts,
    .onTcpMessage = onCTcpMessageAllScripts,
    .onUdpMessage = onCUdpMessageAllScripts,
    .onPlayerJoined = onCPlayerJoinedAllScripts,
    .onPlayerLeave = onCPlayerLeaveAllScripts,
    .onObjectAdded = onCObjectAddedAllScripts,
    .onObjectRemoved = onCObjectRemovedAllScripts,
  };
}