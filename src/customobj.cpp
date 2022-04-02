#include "./customobj.h"

/* This should be temporary to try out rendering in C++ 
   Should port the scheme scripting to C++, and then these just become ordinary api methods
*/

void* createBasicTest(){
  std::cout << "custom binding: create basic" << std::endl;
  return NULL;
}
void removeBasicTest(void* data){
  std::cout << "custom binding: remove basic" << std::endl;
}
void renderBasicTest(){
  std::cout << "custom binding: render basic" << std::endl;
}


struct CustomObjBinding {
  std::string name;
  std::function<void*()> create;
  std::function<void(void*)> remove;
  std::function<void()> render;
};

std::vector<CustomObjBinding> bindings = {
  CustomObjBinding {
    .name = "native:basic_test",
    .create = createBasicTest,
    .remove = removeBasicTest,
    .render = renderBasicTest,
  },
};

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

void createCustomObj(){
  // i guess pick a name, id pair
  auto id = 5;
  auto name = "native:basic_test";

  assert(customObjInstances.find(id) == customObjInstances.end());
  auto binding = getCustomObjBinding(name);
  auto data = binding -> create();
  customObjInstances[id] = CustomObjInstance {
    .name = name,
    .data = data,
  };
}
void removeCustomObj(){
  auto id = 5;
  auto objInstance = customObjInstances.at(id);
  auto binding = getCustomObjBinding(objInstance.name.c_str());
  binding -> remove(objInstance.data);
}
void renderCustomObj(){
  auto id = 5;
  auto objInstance = customObjInstances.at(id);
  auto binding = getCustomObjBinding(objInstance.name.c_str());
  binding -> render();
}




