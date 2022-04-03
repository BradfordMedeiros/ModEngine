#ifndef MOD_CUSTOMOBJ_BINDING
#define MOD_CUSTOMOBJ_BINDING

#include <string>
#include <functional>

struct CustomApiBindings {
  int32_t (*listSceneId)(int32_t objid);
  int32_t (*loadScene)(std::string, std::vector<std::vector<std::string>>);
  void (*unloadScene)(int32_t id);
  void (*unloadAllScenes)();
};

struct CustomObjBinding {
  std::string name;
  CustomApiBindings& api;
  std::function<void*()> create;
  std::function<void(void*)> remove;
  std::function<void(void*)> render;
};

CustomObjBinding createCustomBinding(const char* name, CustomApiBindings& api);

#endif