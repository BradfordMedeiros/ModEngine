#include "./customobj_sample.h"

void* createBasicTest(){
  std::cout << "custom binding: create basic" << std::endl;
  int* value = new int;
  *value = random();
  return value;
}
void removeBasicTest(void* data){
  int* value = (int*)data;
  delete value;
  std::cout << "custom binding: remove basic" << std::endl;
}
void renderBasicTest(void* data){
  std::cout << "custom binding: render basic, value: " << *((int*)data) << std::endl;
}

CustomObjBinding sampleBindingPlugin(){
  auto binding = createCustomBinding("native/basic_test");
  binding.create = createBasicTest;
  binding.remove = removeBasicTest;
  binding.render = renderBasicTest;
  return binding;
}
