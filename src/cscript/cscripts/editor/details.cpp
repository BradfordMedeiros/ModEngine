#include "./details.h"

CScriptBinding cscriptDetailsBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/details", api);

  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    return NULL;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
  };

  binding.onMessage = [](objid scriptId, void* data, std::string& topic, AttributeValue& value) -> void {
  };

  return binding;
}
