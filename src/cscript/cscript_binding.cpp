#include "./cscript_binding.h"

CScriptBinding createCScriptBinding(const char* bindingMatcher, CustomApiBindings& api){
  CScriptBinding defaultBinding { 
    .bindingMatcher = bindingMatcher,
    .api = api,
    .create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* { 
        return NULL; 
    },
    .remove = [](std::string scriptname, objid id, void*) -> void { },
    .render = [](void*) -> void { },
    
    .onFrame = [](objid scriptId) -> void { },
    .onCollisionEnter = [](objid scriptId, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal) -> void {},
    .onCollisionExit = [](objid scriptId, int32_t obj1, int32_t obj2) -> void { },
    .onMouseCallback = [](objid scriptId, int button, int action, int mods) -> void { },
    .onMouseMoveCallback = [](objid scriptId, double xPos, double yPos, float xNdc, float yNdc) -> void { },
    
    .onScrollCallback = [](double amount) -> void{ },
    .onObjectSelected = [](int32_t index, glm::vec3 color) -> void {},
    .onObjectHover = [](int32_t index, bool hoverOn) -> void {},
    .onKeyCallback = [](int key, int scancode, int action, int mods) -> void {},
    .onKeyCharCallback = [](unsigned int codepoint) -> void {},
    .onCameraSystemChange = [](std::string, bool) -> void {},
    .onMessage = [](std::string& topic, AttributeValue& value) -> void {},
    .onTcpMessage = [](std::string&) -> void {},
    .onUdpMessage = [](std::string&) -> void {},
    .onPlayerJoined = [](std::string&) -> void {},
    .onPlayerLeave = [](std::string&) -> void {},
  };
  return defaultBinding;
}

