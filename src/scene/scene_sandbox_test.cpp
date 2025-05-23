#include "./scene_sandbox_test.h"

// Should be able to update the pos of an element and get it instantaneously
// parent pos take 1 frame (update sandbox) to update

// but maybe this should check if a read for a child is dirty and then it could recalculate?


//std::string debugAllGameObjects(SceneSandbox& sandbox);
//std::string debugAllGameObjectsH(SceneSandbox& sandbox);

static std::set<std::string> testObjautoserializerFields(std::string& name){
  return {};
};

void sandboxBasicDeserialization(){
  auto sandbox = createSceneSandbox({}, testObjautoserializerFields);
  std::string twoItemScene = std::string("") + 
  "object_one:position:1 0 0\n" + 
  "object_two:position:5 0 0\n";
  addSceneDataToScenebox(sandbox, "somefilename", 1, twoItemScene, std::nullopt, std::nullopt, testObjautoserializerFields, std::nullopt, std::nullopt);

  auto numObjects = getNumberOfObjects(sandbox);
  if (numObjects != 3){  // global root, scene root, two objects in scene
    throw std::logic_error(std::string("incorrect number of objects: ") + std::to_string(numObjects));
  }
}

void sandboxParentPosition(){
  SceneSandbox sandbox = createSceneSandbox({ LayerInfo{ .name = "", } }, testObjautoserializerFields);
  std::string sceneWithChild = std::string("") + 
  "object_one:position:1 0 0\n" + 
  "object_one:child:object_two\n" + 
  "object_two:position:5 0 0\n";
  addSceneDataToScenebox(sandbox, "somefilename", 1, sceneWithChild, std::nullopt, std::nullopt, testObjautoserializerFields, std::nullopt, std::nullopt);
  auto objectTwoId = getGameObjectH(sandbox, "object_two", 1).id;
  auto posObjectTwo = fullTransformation(sandbox, objectTwoId);
  auto equal = aboutEqual(posObjectTwo.position, glm::vec3(6.f, 0.f, 0.f));
  if (!equal){
    throw std::logic_error(std::string("positions not equal: got ") + print(posObjectTwo.position));
  }
}

void sandboxMakeParentPosition(){
  SceneSandbox sandbox = createSceneSandbox({ LayerInfo{ .name = "", } }, testObjautoserializerFields);
  std::string sceneWithChild = std::string("") + 
  "object_one:position:1 0 0\n" + 
  "object_two:position:5 0 0\n";
  addSceneDataToScenebox(sandbox, "somefilename", 1, sceneWithChild, std::nullopt, std::nullopt, testObjautoserializerFields, std::nullopt, std::nullopt);
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
  SceneSandbox sandbox = createSceneSandbox({ LayerInfo{ .name = "", } }, testObjautoserializerFields);
  std::string sceneWithChild = std::string("") + 
  "object_one:position:-1 0 0\n" + 
  "object_one:child:object_two\n" + 
  "object_two:position:5 0 0\n";
  addSceneDataToScenebox(sandbox, "somefilename", 1, sceneWithChild, std::nullopt, std::nullopt, testObjautoserializerFields, std::nullopt, std::nullopt);
  auto objectOneId = getGameObjectH(sandbox, "object_one", 1).id;
  auto objectTwoId = getGameObjectH(sandbox, "object_two", 1).id;

  updateRelativePosition(sandbox, objectOneId, glm::vec3(3.f, 1.f, 1.f), Hint{});
  updateSandbox(sandbox);

  auto posObjectTwoParented = fullTransformation(sandbox, objectTwoId);
  auto equalParented = aboutEqual(posObjectTwoParented.position, glm::vec3(8.f, 1.f, 1.f));
  if (!equalParented){
    throw std::logic_error(std::string("positions not equal: got ") + print(posObjectTwoParented.position));
  } 
}

void sandboxUpdateParentAbsolute(){
  SceneSandbox sandbox = createSceneSandbox({ LayerInfo{ .name = "", } }, testObjautoserializerFields);
  std::string sceneWithChild = std::string("") + 
  "object_two:position:5 0 0\n" + 
    "object_one:position:-1 0 0\n" + 
  "object_one:child:object_two\n";
  addSceneDataToScenebox(sandbox, "somefilename", 1, sceneWithChild, std::nullopt, std::nullopt, testObjautoserializerFields, std::nullopt, std::nullopt);
  auto objectOneId = getGameObjectH(sandbox, "object_one", 1).id;
  auto objectTwoId = getGameObjectH(sandbox, "object_two", 1).id;

  updateAbsolutePosition(sandbox, objectOneId, glm::vec3(3.f, 1.f, 1.f), Hint{});
  updateSandbox(sandbox);

  auto posObjectTwoParented = fullTransformation(sandbox, objectTwoId);
  auto equalParented = aboutEqual(posObjectTwoParented.position, glm::vec3(8.f, 1.f, 1.f));
  if (!equalParented){
    throw std::logic_error(std::string("positions not equal: got ") + print(posObjectTwoParented.position));
  } 
}


void sandboxUpdateParentAndChildRelative(){
  SceneSandbox sandbox = createSceneSandbox({ LayerInfo{ .name = "", } }, testObjautoserializerFields);
  std::string sceneWithChild = std::string("") + 
  "object_one:position:-1 0 0\n" + 
  "object_one:child:object_two\n" + 
  "object_two:position:5 0 0\n";
  addSceneDataToScenebox(sandbox, "somefilename", 1, sceneWithChild, std::nullopt, std::nullopt, testObjautoserializerFields, std::nullopt, std::nullopt);
  auto objectOneId = getGameObjectH(sandbox, "object_one", 1).id;
  auto objectTwoId = getGameObjectH(sandbox, "object_two", 1).id;

  updateRelativePosition(sandbox, objectOneId, glm::vec3(3.f, 1.f, 1.f), Hint{});
  updateRelativePosition(sandbox, objectTwoId, glm::vec3(-4.f, -1.f, -1.f), Hint{});

  updateSandbox(sandbox);

  auto posObjectTwoParented = fullTransformation(sandbox, objectTwoId);
  auto equalParented = aboutEqual(posObjectTwoParented.position, glm::vec3(-1.f, 0.f, 0.f));
  if (!equalParented){
    throw std::logic_error(std::string("positions not equal: got ") + print(posObjectTwoParented.position));
  } 
}

void sandboxRelativeTransform(){
  struct RelativeTransformTest {
    Transformation child;
    Transformation parent;
    Transformation relative;
  };

  // there are potentially multiple answers with rotations, but these are one
  // easiest to reason about by drawing out, remember the coord system 
  std::vector<RelativeTransformTest> tests = {
    RelativeTransformTest { 
      .child = Transformation { .position = glm::vec3(0.f, 0.f, 0.f), .scale = glm::vec3(1.f, 1.f, 1.f), .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)) },
      .parent = Transformation { .position = glm::vec3(0.f, 0.f, 0.f), .scale = glm::vec3(1.f, 1.f, 1.f), .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)) },
      .relative = Transformation { .position = glm::vec3(0.f, 0.f, 0.f), .scale = glm::vec3(1.f, 1.f, 1.f), .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)) },
    },
    RelativeTransformTest { 
      .child = Transformation { .position = glm::vec3(1.f, 2.f, 3.f), .scale = glm::vec3(1.f, 1.f, 1.f), .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)) },
      .parent = Transformation { .position = glm::vec3(0.f, 0.f, 0.f), .scale = glm::vec3(1.f, 1.f, 1.f), .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)) },
      .relative = Transformation { .position = glm::vec3(1.f, 2.f, 3.f), .scale = glm::vec3(1.f, 1.f, 1.f), .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)) },
    },
    RelativeTransformTest { 
      .child = Transformation { .position = glm::vec3(1.f, 2.f, 3.f), .scale = glm::vec3(1.f, 2.f, -3.f), .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)) },
      .parent = Transformation { .position = glm::vec3(0.f, 0.f, 0.f), .scale = glm::vec3(1.f, 1.f, 1.f), .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)) },
      .relative = Transformation { .position = glm::vec3(1.f, 2.f, 3.f), .scale = glm::vec3(-1.f, -2.f, -3.f), .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 180.f)) },
    },
    RelativeTransformTest { 
      .child = Transformation { .position = glm::vec3(0.f, 0.f, 0.f), .scale = glm::vec3(1.f, 1.f, 1.f), .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)) },
      .parent = Transformation { .position = glm::vec3(30.f, -30.f, 0.f), .scale = glm::vec3(10.f, 10.f, 10.f), .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)) },
      .relative = Transformation { .position = glm::vec3(-3.f, 3.f, 0.f), .scale = glm::vec3(0.1f, 0.1f, 0.1f), .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)) },
    },
    RelativeTransformTest { 
      .child = Transformation { .position = glm::vec3(0.f, 0.f, 0.f), .scale = glm::vec3(1.f, 1.f, 1.f), .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)) },
      .parent = Transformation { .position = glm::vec3(30.f, -30.f, 0.f), .scale = glm::vec3(10.f, 10.f, 10.f), .rotation = parseQuat(glm::vec4(0.f, 0.f, 1.f, 0.f)) },
      .relative = Transformation { .position = glm::vec3(3.f, 3.f, 0.f), .scale = glm::vec3(0.1f, 0.1f, 0.1f), .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)) },
    },


  //update absolute transform: 133473277, position =  [pos =  , scale = 0.2 0.2 0.2, rot = -8.9407e-08 1 0 45]
  //parent absolute transform:  [pos = -20 -30 0, scale = 100 100 10, rot = 0 -1 -1.78814e-07 0]
  //update relative transform:  [pos = -61.5909 1 3.66592e-05, scale = 0 0.00366592 -27.9899, rot = 0 -2.80179e-07 -1 180]


  };

  for (auto &test : tests){
    auto actualRelative = calcRelativeTransform(test.child, test.parent);
    if (!testOnlyAboutEqual(actualRelative, test.relative)){
      throw std::logic_error("invalid, wanted = " + print(test.relative) + ", actual = " + print(actualRelative));
    }
  }
}

void sandboxRemoveSceneTest(){
  SceneSandbox sandbox = createSceneSandbox({ LayerInfo{ .name = "", } }, testObjautoserializerFields);
  std::string testScene1= std::string("") + 
  "testobject:position:-1 0 0\n";
  std::string testScene2 = std::string("") + 
  "testobject:position:-1 0 0\n";

  addSceneDataToScenebox(sandbox, "somefilename1", 1, testScene1, std::nullopt, std::nullopt, testObjautoserializerFields, std::nullopt, std::nullopt);
  addSceneDataToScenebox(sandbox, "somefilename2", 2, testScene2, std::nullopt, std::nullopt, testObjautoserializerFields, std::nullopt, std::nullopt);

  auto objectOneId = getGameObjectH(sandbox, "testobject", 1).id;
  auto objectTwoId = getGameObjectH(sandbox, "testobject", 2).id;

  {
    auto sceneIds = allSceneIds(sandbox, std::nullopt);
    if (sceneIds.size() != 2){
      throw std::logic_error(std::string("unexpected number of scenes: got ") + std::to_string(sceneIds.size()));
    }
  }
  removeScene(sandbox, 1);
  {
    auto sceneIds = allSceneIds(sandbox, std::nullopt);
    if (sceneIds.size() != 1){
      throw std::logic_error(std::string("unexpected number of scenes: got ") + std::to_string(sceneIds.size()));
    }
  }
  removeScene(sandbox, 2);

  {
    auto sceneIds = allSceneIds(sandbox, std::nullopt);
    if (sceneIds.size() != 0){
      throw std::logic_error(std::string("unexpected number of scenes: got ") + std::to_string(sceneIds.size()));
    }
  }
}

void sandboxRemoveSceneParentTest(){
  SceneSandbox sandbox = createSceneSandbox({ LayerInfo{ .name = "", } }, testObjautoserializerFields);
  std::string testScene1= std::string("") + 
  "testobject:position:-1 0 0\n";
  std::string testScene2 = std::string("") +
  "testobjectrun:position:-1 0 0\n" + 
  "testobject:position:-1 0 0\n";

  addSceneDataToScenebox(sandbox, "somefilename1", 1, testScene1, std::nullopt, std::nullopt, testObjautoserializerFields, std::nullopt, std::nullopt);
  addSceneDataToScenebox(sandbox, "somefilename2", 2, testScene2, std::nullopt, std::nullopt, testObjautoserializerFields, std::nullopt, std::nullopt);

  auto objectOneId = getGameObjectH(sandbox, "testobject", 1).id;
  auto objectTwoId = getGameObjectH(sandbox, "testobject", 2).id;

  makeParent(sandbox, objectTwoId, objectOneId);

  {
    auto sceneIds = allSceneIds(sandbox, std::nullopt);
    if (sceneIds.size() != 2){
      throw std::logic_error(std::string("unexpected number of scenes: got ") + std::to_string(sceneIds.size()));
    }
    {
      auto numObjectsInScene = listObjInScene(sandbox, 1).size();
      if (numObjectsInScene != 1){
        throw std::logic_error(std::string("unexpected number of scenes: got ") + std::to_string(numObjectsInScene));
      };      
    }
    {
      auto numObjectsInScene = listObjInScene(sandbox, 2).size();
      if (numObjectsInScene != 2){
        throw std::logic_error(std::string("unexpected number in scenes 2: got ") + std::to_string(numObjectsInScene));
      };      
    }
  }
  removeScene(sandbox, 1);

  {
    auto sceneIds = allSceneIds(sandbox, std::nullopt);
    if (sceneIds.size() != 1 || sceneIds.at(0) != 2){
      throw std::logic_error(std::string("after remove unexpected number of scenes: got ") + std::to_string(sceneIds.size()));
    }
    auto numObjectsInScene = listObjInScene(sandbox, 2).size();
    if (numObjectsInScene != 1){
      throw std::logic_error(std::string("after remove unexpected number in scenes 2: got ") + std::to_string(numObjectsInScene));
    };      
  }  


}