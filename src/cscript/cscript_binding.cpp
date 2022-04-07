#include "./cscript_binding.h"

CScriptBinding createCScriptBinding(const char* bindingMatcher, CustomApiBindings& api){
  CScriptBinding defaultBinding { 
    .bindingMatcher = bindingMatcher,
    .api = api,
    .create = [](void) -> void* { return NULL; },
    .remove = [](void*) -> void { },
    .render = [](void*) -> void { },
    
    .onFrame = []() -> void { },

    .onMouseCallback = [](int button, int action, int mods) -> void { },
    .onMouseMoveCallback = [](double xPos, double yPos, float xNdc, float yNdc) -> void { },
    .onScrollCallback = [](double amount) -> void{ },
    
    .onObjectSelected = [](int32_t index, glm::vec3 color) -> void {},
    .onObjectHover = [](int32_t index, bool hoverOn) -> void {},
    .onKeyCallback = [](int key, int scancode, int action, int mods) -> void {},
    .onKeyCharCallback = [](unsigned int codepoint) -> void {},
    .onCameraSystemChange = [](std::string, bool) -> void {},
    //.onMessage = []() -> void {},
    .onTcpMessage = [](std::string&) -> void {},
    .onUdpMessage = [](std::string&) -> void {},
    .onPlayerJoined = [](std::string&) -> void {},
    .onPlayerLeave = [](std::string&) -> void {},
  };
  return defaultBinding;
}

