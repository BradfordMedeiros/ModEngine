#include "./customobj_binding.h"

CustomObjBinding createCustomBinding(const char* name){
  CustomObjBinding defaultBinding { 
    .name = name,
    .create = [](void) -> void* { return NULL; },
    .remove = [](void*) -> void { },
    .render = [](void*) -> void { },
  };
  return defaultBinding;
}