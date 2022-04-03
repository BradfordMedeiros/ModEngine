#ifndef MOD_CUSTOMOBJ_BINDING
#define MOD_CUSTOMOBJ_BINDING

#include <string>
#include <functional>

struct CustomObjBinding {
  std::string name;
  std::function<void*()> create;
  std::function<void(void*)> remove;
  std::function<void(void*)> render;
};

CustomObjBinding createCustomBinding(const char* name);

#endif