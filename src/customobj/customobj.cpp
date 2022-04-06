#include "./customobj.h"

/* This should be temporary to try out rendering in C++ 
   Should port the scheme scripting to C++, and then these just become ordinary api methods
*/

std::vector<CustomObjBinding> bindings = {};

void registerAllBindings(std::vector<CustomObjBinding> pluginBindings){
  std::set<std::string> names;
  for (auto &plugin : pluginBindings){
    bindings.push_back(plugin);
    if (names.count(plugin.name) > 0){
      std::cout << "plugin name duplicate: " << plugin.name << std::endl;
      assert(false);
    }
    names.insert(plugin.name);
  }
}

struct CustomObjInstance {
  std::string name;
  void* data;
};
std::map<int, CustomObjInstance> customObjInstances = {};


CustomObjBinding* getCustomObjBinding(const char* name){
  for (auto &customObj : bindings){
    if (customObj.name == name){
      return &customObj;
    }
  }
  assert(false);
  return NULL;
}

void createCustomObj(int id, const char* name, int sceneId, bool bootstrapperMode, bool isFreeScript){
  assert(customObjInstances.find(id) == customObjInstances.end());
  auto binding = getCustomObjBinding(name);
  auto data = binding -> create();
  customObjInstances[id] = CustomObjInstance {
    .name = name,
    .data = data,
  };
}
void removeCustomObj(int id){
  auto instanceExists = customObjInstances.find(id) != customObjInstances.end();
  if (instanceExists){
    auto objInstance = customObjInstances.at(id);
    auto binding = getCustomObjBinding(objInstance.name.c_str());
    binding -> remove(objInstance.data);   
  }
}
void renderCustomObj(int id){
  auto instanceExists = customObjInstances.find(id) != customObjInstances.end();
  if (instanceExists){
    auto objInstance = customObjInstances.at(id);
    auto binding = getCustomObjBinding(objInstance.name.c_str());
    binding -> render(objInstance.data);   
  }
}


//// Callbacks ////
void onCFrameAllScripts(){
  for (auto &[instanceId, objInstance] : customObjInstances){
    auto binding = getCustomObjBinding(objInstance.name.c_str());
    assert(binding != NULL);
    binding -> onFrame();
  }
}

void onCCollisionEnterAllScripts(int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal){
}
void onCCollisionExitAllScripts(int32_t obj1, int32_t obj2){
}
void onCMouseCallbackAllScripts(int button, int action, int mods){
}
void onCMouseMoveCallbackAllScripts(double xPos, double yPos, float xNdc, float yNdc){
}
void onCScrollCallbackAllScripts(double amount){
}
void onCObjectSelectedAllScripts(int32_t index, glm::vec3 color){
}
void onCObjectHoverAllScripts(int32_t index, bool isHover){
}
void onCKeyCallbackAllScripts(int key, int scancode, int action, int mods){
}
void onCKeyCharCallbackAllScripts(unsigned int codepoint){
}

void onCCameraSystemChangeAllScripts(std::string camera, bool usingBuiltInCamera){
}
void onCMessageAllScripts(std::queue<StringString>& messages){
}

void onCTcpMessageAllScripts(std::string& message){
}
void onCUdpMessageAllScripts(std::string& message){
}

void onCPlayerJoinedAllScripts(std::string& connectionHash){
}
void onCPlayerLeaveAllScripts(std::string& connectionHash){
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