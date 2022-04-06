#include "./cscript_scheme.h"

void* createSchemeScript(){
  return NULL;
}

CustomObjBinding cscriptSchemeBinding(CustomApiBindings& api){
  auto binding = createCustomBinding("./res/scripts/color.scm", api);
  binding.create = createSchemeScript;
  binding.remove = [&api] (void* data) -> void {
    
  };
  return binding;
}