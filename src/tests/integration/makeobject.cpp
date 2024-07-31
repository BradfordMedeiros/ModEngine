#include "./makeobject.h"

extern CustomApiBindings* mainApi;

/////////////////////////////////////////////////////////
// Verifies we can make an object and it exists
struct BasicMakeObject {
  std::optional<objid> gameobjId;
};
IntegrationTest basicMakeObjectTest {
  .name = "basicMakeObjectTest",
  .createTestData = []() -> std::any {
    return BasicMakeObject {
      .gameobjId = std::nullopt,
    };
  },
  .test = [](std::any& value, objid sceneId) -> std::optional<TestRunReturn> {
    BasicMakeObject* makeObjectData = anycast<BasicMakeObject>(value);
    modassert(makeObjectData, "invalid type makeObjectData");
    if(!makeObjectData -> gameobjId.has_value()){
      GameobjAttributes attr { .attr = {} };
      std::map<std::string, GameobjAttributes> submodelAttributes;
      makeObjectData -> gameobjId = mainApi -> makeObjectAttr(sceneId, "test-object-test", attr, submodelAttributes);
      if (!mainApi -> gameobjExists(makeObjectData -> gameobjId.value())){
        return IntegTestResult { .passed = false, .reason = "object does not exist on same frame" };
      }    
      return std::nullopt;
    }
    if (!mainApi -> gameobjExists(makeObjectData -> gameobjId.value())){
      return IntegTestResult { .passed = false, .reason = "object does not exist on second frame"};
    }
    return IntegTestResult { .passed = true };
  }
};


/////////////////////////////////////////////////////////
// Verifies the scene doesn't orphan objects

struct CheckNumberGameObjects {
  std::optional<objid> sceneId;
  std::optional<int> numObjects;
};

int numberOfObjectsFrameStart(){
  auto objectCountAttr = mainApi -> statValue(mainApi -> stat("object-count"));
  auto objectCount = std::get_if<int>(&objectCountAttr);
  modassert(objectCount, "object count is not value");
  return *objectCount;
}

IntegrationTest checkUnloadSceneTest {
  .name = "checkUnloadSceneTest",
  .createTestData = []() -> std::any {
    return CheckNumberGameObjects {
      .sceneId = std::nullopt,
      .numObjects = std::nullopt,
    };
  },
  .test = [](std::any& value, objid sceneId) -> std::optional<TestRunReturn> {
    CheckNumberGameObjects* objectsData = anycast<CheckNumberGameObjects>(value);
    modassert(objectsData, "invalid type makeObjectData");

    if (objectsData -> numObjects.has_value() && objectsData -> sceneId.has_value()){
      mainApi -> unloadScene(objectsData -> sceneId.value());
      objectsData -> sceneId = std::nullopt;
      return std::nullopt;
    } 
    if (objectsData -> numObjects.has_value()){
      int startFrame = numberOfObjectsFrameStart();
      if (startFrame != objectsData -> numObjects.value()){
        return IntegTestResult { .passed = false, .reason = std::string("got = ") + std::to_string(startFrame) + std::string(", wanted = ") + std::to_string(objectsData -> numObjects.value()) };
      }
      return IntegTestResult { .passed = true };
    }
    if (!objectsData -> sceneId.has_value()){
      objectsData -> numObjects = numberOfObjectsFrameStart();
      objectsData -> sceneId = mainApi -> loadScene("./res/scenes/empty.p.rawscene",{}, std::nullopt, sceneTags);
      for (int i = 0; i < 15; i++){
        GameobjAttributes attr { .attr = {} };
        std::map<std::string, GameobjAttributes> submodelAttributes;
        mainApi -> makeObjectAttr(objectsData -> sceneId.value(), std::string("test-object-test-") + std::to_string(getUniqueObjId()), attr, submodelAttributes);
      }
      return std::nullopt;  
    }
    return IntegTestResult { .passed = false };
  }
};

/////////////////////////////////////////////////////////
// Verifies the element is parented 
IntegrationTest parentSceneTest {
  .name = "parentSceneTest",
  .createTestData = []() -> std::any {
    return CheckNumberGameObjects {
      .sceneId = std::nullopt,
      .numObjects = std::nullopt,
    };
  },
  .test = [](std::any& value, objid sceneId) -> std::optional<TestRunReturn> {
    CheckNumberGameObjects* objectsData = anycast<CheckNumberGameObjects>(value);
    modassert(objectsData, "invalid type makeObjectData");

    if (objectsData -> numObjects.has_value() && objectsData -> sceneId.has_value()){
      mainApi -> unloadScene(objectsData -> sceneId.value());
      objectsData -> sceneId = std::nullopt;
      return std::nullopt;
    } 
    if (objectsData -> numObjects.has_value()){
      int startFrame = numberOfObjectsFrameStart();
      if (startFrame != objectsData -> numObjects.value()){
        return IntegTestResult { .passed = false, .reason = std::string("got = ") + std::to_string(startFrame) + std::string(", wanted = ") + std::to_string(objectsData -> numObjects.value()) };
      }
      return IntegTestResult { .passed = true };
    }
    if (!objectsData -> sceneId.has_value()){
      objectsData -> numObjects = numberOfObjectsFrameStart();
      objectsData -> sceneId = mainApi -> loadScene("./res/scenes/empty.p.rawscene",{}, std::nullopt, sceneTags);
      for (int i = 0; i < 15; i++){
        GameobjAttributes attr { .attr = {} };
        std::map<std::string, GameobjAttributes> submodelAttributes;
        mainApi -> makeObjectAttr(objectsData -> sceneId.value(), std::string("test-object-test-") + std::to_string(getUniqueObjId()), attr, submodelAttributes);
      }
      return std::nullopt;  
    }
    return IntegTestResult { .passed = false };
  }
};

