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
    .onFrameAfterUpdate = [](objid scriptId, void* data) -> void { },
    .onCollisionEnter = [](objid scriptId, void* data, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal, float force) -> void {},
    .onCollisionExit = [](objid scriptId, void* data, int32_t obj1, int32_t obj2) -> void { },
    .onMouseCallback = [](objid scriptId, void* data, int button, int action, int mods) -> void { },
    .onMouseMoveCallback = [](objid scriptId, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { },
    .onScrollCallback = [](objid scriptId, void* data, double amount) -> void{ },
    .onObjectSelected = [](objid scriptId, void* data, int32_t index, glm::vec3 color, int selectIndex) -> void {},
    .onObjectUnselected = [](objid scriptId, void* data) -> void {},
    .onObjectHover = [](objid scriptId, void* data, int32_t index, bool hoverOn) -> void {},
    .onMapping = [](int32_t id, void* data, int32_t index) -> void {},
    .onKeyCallback = [](objid scriptId, void* data, int key, int scancode, int action, int mods) -> void {},
    .onKeyCharCallback = [](objid scriptId, void* data, unsigned int codepoint) -> void {},
    .onController = [](objid scriptId, void* data, int joystick, bool) -> void { },
    .onControllerKey = [](objid scriptId, void* data, int joystick, BUTTON_TYPE button, bool down) -> void {},
    .onCameraSystemChange = [](objid scriptId, std::string, bool) -> void {},
    .onMessage = [](objid scriptId, void* data, std::string& topic, std::any& value) -> void {},
    .onTcpMessage = [](objid scriptId, std::string&) -> void {},
    .onUdpMessage = [](objid scriptId, std::string&) -> void {},
    .onPlayerJoined = [](objid scriptId, std::string&) -> void {},
    .onPlayerLeave = [](objid scriptId, std::string&) -> void {},
    .onObjectAdded = [](objid scriptId, void* data, objid idAdded) -> void {},
    .onObjectRemoved = [](objid scriptId, void* data, objid idRemoved) -> void {},
    .render = [](void*) -> void { },
  };
  return defaultBinding;
}

