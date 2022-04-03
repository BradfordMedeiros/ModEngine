#include "./customobj.h"

/* This should be temporary to try out rendering in C++ 
   Should port the scheme scripting to C++, and then these just become ordinary api methods
*/

std::vector<CustomObjBinding> bindings = {};

void registerAllBindings(std::vector<CustomObjBinding> pluginBindings){
  std::set<std::string> names;
  for (auto &plugin : pluginBindings){
    bindings.push_back(plugin);
    if (names.count(plugin.name) > 0){
      std::cout << "plugin name duplicate: " << plugin.name << std::endl;
      assert(false);
    }
    names.insert(plugin.name);
  }
}

struct CustomObjInstance {
  std::string name;
  void* data;
};
std::map<int, CustomObjInstance> customObjInstances = {};


CustomObjBinding* getCustomObjBinding(const char* name){
  for (auto &customObj : bindings){
    if (customObj.name == name){
      return &customObj;
    }
  }
  assert(false);
  return NULL;
}

void createCustomObj(int id, const char* name){
  assert(customObjInstances.find(id) == customObjInstances.end());
  auto binding = getCustomObjBinding(name);
  auto data = binding -> create();
  customObjInstances[id] = CustomObjInstance {
    .name = name,
    .data = data,
  };
}
void removeCustomObj(int id){
  auto instanceExists = customObjInstances.find(id) != customObjInstances.end();
  if (instanceExists){
    auto objInstance = customObjInstances.at(id);
    auto binding = getCustomObjBinding(objInstance.name.c_str());
    binding -> remove(objInstance.data);   
  }
}
void renderCustomObj(int id){
  auto instanceExists = customObjInstances.find(id) != customObjInstances.end();
  if (instanceExists){
    auto objInstance = customObjInstances.at(id);
    auto binding = getCustomObjBinding(objInstance.name.c_str());
    binding -> render(objInstance.data);   
  }
}




