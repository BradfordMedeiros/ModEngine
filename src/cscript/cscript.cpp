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
  int instanceId;
  std::string name;
  int runlevel;
  void* data;
  CScriptBinding* cScriptBinding;
  bool shouldRemove;
};

std::vector<CustomObjInstance> customObjInstances = {};
std::vector<CustomObjInstance> stagedObjInstances = {};  // this is needed so we don't invalidate iterators while loading, same with should remove

bool patternMatchesScriptName(std::string pattern, std::string scriptname){
  std::basic_regex reg(pattern);
  auto matches = std::regex_match(scriptname, reg);
  return matches;
}

struct ParsedCscriptName {
  std::string scriptname;
  int runlevel;
};

ParsedCscriptName parseCScriptName(std::string name){
  auto values = carAndRest(name, ':');
  int runlevel = 0;
  if (values.second != ""){
    runlevel = std::atoi(values.second.c_str());
  }
  return ParsedCscriptName {
    .scriptname = values.first,
    .runlevel = runlevel,
  };
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

CustomObjInstance* getCustomObjInstance(int id){
  for (int i = 0; i < customObjInstances.size(); i++){
    CustomObjInstance& instance = customObjInstances.at(i);
    if (instance.instanceId == id){
      return &instance;
    }
  }
  return NULL;
}

void loadCScript(int id, const char* name, int sceneId, bool bootstrapperMode, bool isFreeScript){
  assert(getCustomObjInstance(id) == NULL);
  auto parsedScript = parseCScriptName(name);
  auto binding = getCScriptBinding(parsedScript.scriptname.c_str());
  auto data = binding -> create(parsedScript.scriptname.c_str(), id, sceneId, bootstrapperMode, isFreeScript);
  stagedObjInstances.push_back(
    CustomObjInstance {
      .instanceId = id,
      .name = parsedScript.scriptname,
      .runlevel = parsedScript.runlevel,
      .data = data,
      .cScriptBinding = binding,
      .shouldRemove = false,
    }
  );
}
void unloadCScript(int id){
  auto objInstance = getCustomObjInstance(id);
  if (objInstance){
    auto binding = objInstance -> cScriptBinding;
    binding -> remove(objInstance -> name, id, objInstance -> data); 
    objInstance -> shouldRemove = true;
  }
}

std::string print(std::vector<CustomObjInstance>& instances){
  std::string value = "[";
  for (auto &instance : instances){
    value += " " + instance.name;
  }
  value += " ]";
  return value;
}

int indexForRunLevel(std::vector<CustomObjInstance>& instances, int runlevel){
  if (instances.size() == 0){
    return 0;
  }
  int runLevel = instances.at(0).runlevel;
  int index = 0;
  for (index = 0; index < instances.size(); index++){
    if (runlevel < instances.at(index).runlevel){
      break;
    }
  }
  return index;
}
std::vector<CustomObjInstance> insertIntoCustomObj(std::vector<CustomObjInstance>& instances, CustomObjInstance& instance){
  std::vector<CustomObjInstance> newInstances = {};
  auto indexToInsert = indexForRunLevel(instances, instance.runlevel);
  for (int i = 0; i < indexToInsert; i++){
    newInstances.push_back(instances.at(i));
  }
  newInstances.push_back(instance);
  for (int i = indexToInsert; i < instances.size(); i++){
    newInstances.push_back(instances.at(i));
  }
  return newInstances;
}
void afterFrameForScripts(){
  for (auto &stagedObjInstance : stagedObjInstances){
    customObjInstances = insertIntoCustomObj(customObjInstances, stagedObjInstance);
  }
  stagedObjInstances = {};


  std::vector<CustomObjInstance> newInstances = {};
  for (int i = 0; i < customObjInstances.size(); i++){
    if (!customObjInstances.at(i).shouldRemove){
      newInstances.push_back(customObjInstances.at(i));
    }
  }
  customObjInstances = newInstances;
  //std::cout << "scripts: " << print(customObjInstances) << std::endl;
}

void renderCustomObj(int id){
  //auto instanceExists = customObjInstances.find(id) != customObjInstances.end();
  //if (instanceExists){
  //  auto objInstance = customObjInstances.at(id);
  //  auto binding = objInstance.cScriptBinding;
  //  binding -> render(objInstance.data);   
  //}
}


//// Callbacks ////

void onCFrameAllScripts(){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onFrame(objInstance.instanceId, objInstance.data);
  }
}

void onCCollisionEnterAllScripts(int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal, float force){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onCollisionEnter(objInstance.instanceId, objInstance.data, obj1, obj2, pos, normal, oppositeNormal, force);
  }
}
void onCCollisionExitAllScripts(int32_t obj1, int32_t obj2){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onCollisionExit(objInstance.instanceId, objInstance.data, obj1, obj2);
  }
}
void onCMouseCallbackAllScripts(int button, int action, int mods){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onMouseCallback(objInstance.instanceId, objInstance.data, button, action, mods);
  }
}
void onCMouseMoveCallbackAllScripts(double xPos, double yPos, float xNdc, float yNdc){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onMouseMoveCallback(objInstance.instanceId, objInstance.data, xPos, yPos, xNdc, yNdc);
  }
}
void onCScrollCallbackAllScripts(double amount){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onScrollCallback(objInstance.instanceId, objInstance.data, amount);
  }
}
void onCObjectSelectedAllScripts(int32_t index, glm::vec3 color){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onObjectSelected(objInstance.instanceId, objInstance.data, index, color);
  }
}
void onCObjectUnselectedAllScripts(){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onObjectUnselected(objInstance.instanceId, objInstance.data);
  }
}
void onCObjectHoverAllScripts(int32_t index, bool isHover){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onObjectHover(objInstance.instanceId, objInstance.data, index, isHover);
  }
}

void onCKeyCallbackAllScripts(int key, int scancode, int action, int mods){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onKeyCallback(objInstance.instanceId, objInstance.data, key, scancode, action, mods);
  }
}
void onCKeyCharCallbackAllScripts(unsigned int codepoint){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onKeyCharCallback(objInstance.instanceId, objInstance.data, codepoint);
  }
}

void onCCameraSystemChangeAllScripts(std::string camera, bool usingBuiltInCamera){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onCameraSystemChange(objInstance.instanceId, camera, usingBuiltInCamera);
  }
}
void onCMessageAllScripts(std::string& topic, std::any& value){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onMessage(objInstance.instanceId, objInstance.data, topic, value);
  }
}

void onCTcpMessageAllScripts(std::string& message){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onTcpMessage(objInstance.instanceId, message);
  }
}
void onCUdpMessageAllScripts(std::string& message){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onUdpMessage(objInstance.instanceId, message);
  }
}

void onCPlayerJoinedAllScripts(std::string& connectionHash){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onPlayerJoined(objInstance.instanceId, connectionHash);
  }
}
void onCPlayerLeaveAllScripts(std::string& connectionHash){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onPlayerLeave(objInstance.instanceId, connectionHash);
  }
}
void onCObjectAddedAllScripts(objid idAdded){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onObjectAdded(objInstance.instanceId, objInstance.data, idAdded);
  }
}
void onCObjectRemovedAllScripts(objid idRemoved){
  for (auto &objInstance : customObjInstances){
    if (objInstance.shouldRemove){
      continue;
    }
    auto binding = objInstance.cScriptBinding;
    assert(binding != NULL);
    binding -> onObjectRemoved(objInstance.instanceId, objInstance.data, idRemoved);
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