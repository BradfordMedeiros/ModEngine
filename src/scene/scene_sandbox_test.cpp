#include "./scene_sandbox_test.h"

// Should be able to update the pos of an element and get it instantaneously
// parent pos take 1 frame (update sandbox) to update

// but maybe this should check if a read for a child is dirty and then it could recalculate?

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
  updateSandbox(sandbox);

  auto posObjectTwoParented = fullTransformation(sandbox, objectTwoId);
  auto equalParented = aboutEqual(posObjectTwoParented.position, glm::vec3(6.f, 0.f, 0.f));
  if (!equalParented){
    throw std::logic_error(std::string("positions not equal: got ") + print(posObjectTwoParented.position));
  }

  auto posObjectTwoLocal = getGameObject(sandbox, objectTwoId).transformation;
  auto equalParentedLocal = aboutEqual(posObjectTwoLocal.position, glm::vec3(5.f, 0.f, 0.f));
  if (!equalParentedLocal){
    throw std::logic_error(std::string("local positions not equal: got ") + print(posObjectTwoLocal.position));
  }

}

void sandboxUpdateParentRelative(){
  SceneSandbox sandbox = createSceneSandbox({ LayerInfo{ .name = "", } });
  std::string sceneWithChild = std::string("") + 
  "object_one:position:-1 0 0\n" + 
  "object_one:child:object_two\n" + 
  "object_two:position:5 0 0\n";
  addSceneDataToScenebox(sandbox, "somefilename", 1, sceneWithChild);
  auto objectOneId = getGameObjectH(sandbox, "object_one", 1).id;
  auto objectTwoId = getGameObjectH(sandbox, "object_two", 1).id;

  updateRelativePosition(sandbox, objectOneId, glm::vec3(3.f, 1.f, 1.f));
  updateSandbox(sandbox);

  auto posObjectTwoParented = fullTransformation(sandbox, objectTwoId);
  auto equalParented = aboutEqual(posObjectTwoParented.position, glm::vec3(8.f, 1.f, 1.f));
  if (!equalParented){
    throw std::logic_error(std::string("positions not equal: got ") + print(posObjectTwoParented.position));
  } 
}

void sandboxUpdateParentAbsolute(){
  SceneSandbox sandbox = createSceneSandbox({ LayerInfo{ .name = "", } });
  std::string sceneWithChild = std::string("") + 
  "object_two:position:5 0 0\n" + 
    "object_one:position:-1 0 0\n" + 
  "object_one:child:object_two\n";
  addSceneDataToScenebox(sandbox, "somefilename", 1, sceneWithChild);
  auto objectOneId = getGameObjectH(sandbox, "object_one", 1).id;
  auto objectTwoId = getGameObjectH(sandbox, "object_two", 1).id;

  updateAbsolutePosition(sandbox, objectOneId, glm::vec3(3.f, 1.f, 1.f));
  updateSandbox(sandbox);

  auto posObjectTwoParented = fullTransformation(sandbox, objectTwoId);
  auto equalParented = aboutEqual(posObjectTwoParented.position, glm::vec3(8.f, 1.f, 1.f));
  if (!equalParented){
    throw std::logic_error(std::string("positions not equal: got ") + print(posObjectTwoParented.position));
  } 
}


void sandboxUpdateParentAndChildRelative(){
  SceneSandbox sandbox = createSceneSandbox({ LayerInfo{ .name = "", } });
  std::string sceneWithChild = std::string("") + 
  "object_one:position:-1 0 0\n" + 
  "object_one:child:object_two\n" + 
  "object_two:position:5 0 0\n";
  addSceneDataToScenebox(sandbox, "somefilename", 1, sceneWithChild);
  auto objectOneId = getGameObjectH(sandbox, "object_one", 1).id;
  auto objectTwoId = getGameObjectH(sandbox, "object_two", 1).id;

  updateRelativePosition(sandbox, objectOneId, glm::vec3(3.f, 1.f, 1.f));
  updateRelativePosition(sandbox, objectTwoId, glm::vec3(-4.f, -1.f, -1.f));

  updateSandbox(sandbox);

  auto posObjectTwoParented = fullTransformation(sandbox, objectTwoId);
  auto equalParented = aboutEqual(posObjectTwoParented.position, glm::vec3(-1.f, 0.f, 0.f));
  if (!equalParented){
    throw std::logic_error(std::string("positions not equal: got ") + print(posObjectTwoParented.position));
  } 
}