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
};
std::map<int, CustomObjInstance> customObjInstances = {};

bool patternMatchesScriptName(std::string pattern, std::string scriptname){
  std::basic_regex reg(pattern);
  auto matches = std::regex_match(scriptname, reg);
  return matches;
}

/* matching strategy: matches based upon registration order, * is a wildcard which matches anything*/
CScriptBinding* getCScriptBinding(const char* name){
  // thingiscool  =|= this*cool
  // thingcoolcool

  // thing1 index   thing2 index
  // not wildcard
  //  if letter in thing 2
    //  if match advance both
    //  if no match FAIL
  // if wildcard in thing2
    // two scenarios:
    // scenario1: if only wildcard match advance left 
    // scenario2: if match letter and wildcard, then 
        // create two matchers, and advance both

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
  };
}
void unloadCScript(int id){
  auto instanceExists = customObjInstances.find(id) != customObjInstances.end();
  if (instanceExists){
    auto objInstance = customObjInstances.at(id);
    auto binding = getCScriptBinding(objInstance.name.c_str());
    binding -> remove(objInstance.data);   
  }
}
void renderCustomObj(int id){
  auto instanceExists = customObjInstances.find(id) != customObjInstances.end();
  if (instanceExists){
    auto objInstance = customObjInstances.at(id);
    auto binding = getCScriptBinding(objInstance.name.c_str());
    binding -> render(objInstance.data);   
  }
}


//// Callbacks ////
void onCFrameAllScripts(){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = getCScriptBinding(objInstance.name.c_str());
    assert(binding != NULL);
    binding -> onFrame();
  }
}

void onCCollisionEnterAllScripts(int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal){
  std::cout << "not yet implemented" << std::endl;
  assert(false);
}
void onCCollisionExitAllScripts(int32_t obj1, int32_t obj2){
  std::cout << "not yet implemented" << std::endl;
  assert(false);
}
void onCMouseCallbackAllScripts(int button, int action, int mods){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = getCScriptBinding(objInstance.name.c_str());
    assert(binding != NULL);
    binding -> onMouseCallback(button, action, mods);
  }
}
void onCMouseMoveCallbackAllScripts(double xPos, double yPos, float xNdc, float yNdc){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = getCScriptBinding(objInstance.name.c_str());
    assert(binding != NULL);
    binding -> onMouseMoveCallback(xPos, yPos, xNdc, yNdc);
  }
}
void onCScrollCallbackAllScripts(double amount){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = getCScriptBinding(objInstance.name.c_str());
    assert(binding != NULL);
    binding -> onScrollCallback(amount);
  }
}
void onCObjectSelectedAllScripts(int32_t index, glm::vec3 color){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = getCScriptBinding(objInstance.name.c_str());
    assert(binding != NULL);
    binding -> onObjectSelected(index, color);
  }
}
void onCObjectHoverAllScripts(int32_t index, bool isHover){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = getCScriptBinding(objInstance.name.c_str());
    assert(binding != NULL);
    binding -> onObjectHover(index, isHover);
  }
}
void onCKeyCallbackAllScripts(int key, int scancode, int action, int mods){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = getCScriptBinding(objInstance.name.c_str());
    assert(binding != NULL);
    binding -> onKeyCallback(key, scancode, action, mods);
  }
}
void onCKeyCharCallbackAllScripts(unsigned int codepoint){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = getCScriptBinding(objInstance.name.c_str());
    assert(binding != NULL);
    binding -> onKeyCharCallback(codepoint);
  }
}

void onCCameraSystemChangeAllScripts(std::string camera, bool usingBuiltInCamera){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = getCScriptBinding(objInstance.name.c_str());
    assert(binding != NULL);
    binding -> onCameraSystemChange(camera, usingBuiltInCamera);
  }
}
void onCMessageAllScripts(std::queue<StringString>& messages){
  //std::cout << "on c message not yet implemented" << std::endl; // this one is weird since the scheme fn would dequeue a queue
  //assert(false);
}

void onCTcpMessageAllScripts(std::string& message){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = getCScriptBinding(objInstance.name.c_str());
    assert(binding != NULL);
    binding -> onTcpMessage(message);
  }
}
void onCUdpMessageAllScripts(std::string& message){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = getCScriptBinding(objInstance.name.c_str());
    assert(binding != NULL);
    binding -> onUdpMessage(message);
  }
}

void onCPlayerJoinedAllScripts(std::string& connectionHash){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = getCScriptBinding(objInstance.name.c_str());
    assert(binding != NULL);
    binding -> onPlayerJoined(connectionHash);
  }
}
void onCPlayerLeaveAllScripts(std::string& connectionHash){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = getCScriptBinding(objInstance.name.c_str());
    assert(binding != NULL);
    binding -> onPlayerLeave(connectionHash);
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
    .onObjectHover = onCObjectHoverAllScripts,
    .onKeyCallback = onCKeyCallbackAllScripts,
    .onKeyCharCallback = onCKeyCharCallbackAllScripts,
    .onCameraSystemChange = onCCameraSystemChangeAllScripts,
    .onMessage = onCMessageAllScripts,
    .onTcpMessage = onCTcpMessageAllScripts,
    .onUdpMessage = onCUdpMessageAllScripts,
    .onPlayerJoined = onCPlayerJoinedAllScripts,
    .onPlayerLeave = onCPlayerLeaveAllScripts,
  };
}