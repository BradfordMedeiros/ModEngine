#include "./customobj_binding.h"

CustomObjBinding createCustomBinding(const char* name, CustomApiBindings& api){
  CustomObjBinding defaultBinding { 
    .name = name,
    .api = api,
    .create = [](void) -> void* { return NULL; },
    .remove = [](void*) -> void { },
    .render = [](void*) -> void { },
  };
  return defaultBinding;
}