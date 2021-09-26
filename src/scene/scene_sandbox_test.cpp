#include "./scene_sandbox_test.h"

bool aboutEqual(float one, float two){
  float delta = 0.00001f;
  return one > (two - delta) && one < (two + delta);
}
bool aboutEqual(glm::vec3 one, glm::vec3 two){
  return aboutEqual(one.x, two.x) && aboutEqual(one.y, two.y) && aboutEqual(one.z, two.z);
}

void sandboxBasicDeserialization(){
  auto sandbox = createSceneSandbox({});
  std::string twoItemScene = std::string("") + 
  "object_one:position:1 0 0\n" + 
  "object_two:position:5 0 0\n";
  addSceneDataToScenebox(sandbox, "somefilename", 1, twoItemScene);

  auto numObjects = getNumberOfObjects(sandbox);
  if (numObjects != 4){  // global root, scene root, two objects in scene
    throw std::logic_error(std::string("incorrect number of objects: ") + std::to_string(numObjects));
  }
}

void sandboxParentPosition(){
  SceneSandbox sandbox = createSceneSandbox({ LayerInfo{ .name = "", } });
  std::string sceneWithChild = std::string("") + 
  "object_one:position:1 0 0\n" + 
  "object_one:child:object_two\n" + 
  "object_two:position:5 0 0\n";
  addSceneDataToScenebox(sandbox, "somefilename", 1, sceneWithChild);
  auto objectTwoId = getGameObjectH(sandbox, "object_two", 1).id;
  auto posObjectTwo = fullTransformation(sandbox, objectTwoId);
  auto equal = aboutEqual(posObjectTwo.position, glm::vec3(6.f, 0.f, 0.f));
  if (!equal){
    throw std::logic_error(std::string("positions not equal: got ") + print(posObjectTwo.position));
  }
}

void sandboxMakeParentPosition(){
  SceneSandbox sandbox = createSceneSandbox({ LayerInfo{ .name = "", } });
  std::string sceneWithChild = std::string("") + 
  "object_one:position:1 0 0\n" + 
  "object_two:position:5 0 0\n";
  addSceneDataToScenebox(sandbox, "somefilename", 1, sceneWithChild);
  auto objectOneId = getGameObjectH(sandbox, "object_one", 1).id;
  auto objectTwoId = getGameObjectH(sandbox, "object_two", 1).id;
  auto posObjectTwo = fullTransformation(sandbox, objectTwoId);
  auto equalUnparented = aboutEqual(posObjectTwo.position, glm::vec3(5.f, 0.f, 0.f));
  if (!equalUnparented){
    throw std::logic_error(std::string("positions not equal: got ") + print(posObjectTwo.position));
  }

  makeParent(sandbox, objectTwoId, objectOneId);

  auto posObjectTwoParented = fullTransformation(sandbox, objectTwoId);
  auto equalParented = aboutEqual(posObjectTwoParented.position, glm::vec3(6.f, 0.f, 0.f));
  if (!equalParented){
    throw std::logic_error(std::string("positions not equal: got ") + print(posObjectTwoParented.position));
  }
}
