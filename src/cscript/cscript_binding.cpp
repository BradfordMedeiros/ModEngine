#include "./cscript_binding.h"

CScriptBinding createCScriptBinding(const char* bindingMatcher, CustomApiBindings& api){
  CScriptBinding defaultBinding { 
    .bindingMatcher = bindingMatcher,
    .api = api,
    .create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* { 
        return NULL; 
    },
    .remove = [](std::string scriptname, objid id, void*) -> void { },

    .onFrame = [](objid scriptId, void* data) -> void { },
    .onCollisionEnter = [](objid scriptId, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal) -> void {},
    .onCollisionExit = [](objid scriptId, int32_t obj1, int32_t obj2) -> void { },
    .onMouseCallback = [](objid scriptId, int button, int action, int mods) -> void { },
    .onMouseMoveCallback = [](objid scriptId, double xPos, double yPos, float xNdc, float yNdc) -> void { },
    .onScrollCallback = [](objid scriptId, double amount) -> void{ },
    .onObjectSelected = [](objid scriptId, int32_t index, glm::vec3 color) -> void {},
    .onObjectUnselected = [](objid scriptId) -> void {},
    .onObjectHover = [](objid scriptId, int32_t index, bool hoverOn) -> void {},
    .onKeyCallback = [](objid scriptId, void* data, int key, int scancode, int action, int mods) -> void {},
    .onKeyCharCallback = [](objid scriptId, unsigned int codepoint) -> void {},
    .onCameraSystemChange = [](objid scriptId, std::string, bool) -> void {},
    .onMessage = [](objid scriptId, void* data, std::string& topic, AttributeValue& value) -> void {},
    .onTcpMessage = [](objid scriptId, std::string&) -> void {},
    .onUdpMessage = [](objid scriptId, std::string&) -> void {},
    .onPlayerJoined = [](objid scriptId, std::string&) -> void {},
    .onPlayerLeave = [](objid scriptId, std::string&) -> void {},
    .render = [](void*) -> void { },
  };
  return defaultBinding;
}

