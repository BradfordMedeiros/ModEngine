#include "./cscript_sample.h"

void* createBasicTest(std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript){
  std::cout << "custom binding: create basic" << std::endl;
  int* value = new int;
  *value = random();
  return value;
}

void renderBasicTest(void* data){
  std::cout << "custom binding: render basic, value: " << *((int*)data) << std::endl;
}

CScriptBinding sampleBindingPlugin(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/samplescript", api);
  binding.create = createBasicTest;
  binding.remove = [&api] (void* data) -> void {
    int* value = (int*)data;
    delete value;
    std::cout << "custom binding: remove basic" << std::endl;
    api.loadScene("./res/scenes/features/textures/scrolling.p.rawscene", {});
  };
  binding.render = renderBasicTest;
  return binding;
}
