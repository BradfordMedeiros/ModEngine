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
      std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
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
        std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
        mainApi -> makeObjectAttr(objectsData -> sceneId.value(), std::string("test-object-test-") + std::to_string(getUniqueObjId()), attr, submodelAttributes);
      }
      return std::nullopt;  
    }
    return IntegTestResult { .passed = false };
  }
};

/////////////////////////////////////////////////////////
// Verifies the element is parented correctly
IntegrationTest parentSceneTest {
  .name = "parentSceneTest",
  .createTestData = []() -> std::any {
    return std::nullopt;
  },
  .test = [](std::any& value, objid sceneId) -> std::optional<TestRunReturn> {
    modassert(false, "not yet implemented");
    //auto idForScene = mainApi -> loadScene("./res/scenes/empty.p.rawscene",{}, std::nullopt, sceneTags);
    //auto parentId = mainApi -> getParent(mainApi -> rootIdForScene(idForScene));
    //if (!parentId.has_value()){
    //  return IntegTestResult { .passed = false, .reason = "missing parent id for scene" };
    //}
    //auto mainRootId = mainApi -> rootIdForScene(mainApi -> rootSceneId());
    //if (mainRootId != parentId.value()){
    //  return IntegTestResult { .passed = false, .reason = "scene not parented correctly" };
    //}
    //mainApi -> unloadScene(idForScene);
    //return IntegTestResult { .passed = true };
  }
};

/////////////////////////////////////////////////////////
// Verifies can remove an object that was added in a different location during scene unload
struct MakeObjectAttrData {
  int stage;
  std::optional<objid> sceneId;
  std::optional<int> numObjectsStart;
  std::optional<int> numObjectsAfterCreated;
};

IntegrationTest prefabParentingTest {
  .name = "removeSceneMakeObjectAttrTest",
  .createTestData = []() -> std::any {
    return MakeObjectAttrData {
      .stage = 0,
      .sceneId = std::nullopt,
      .numObjectsStart = std::nullopt,
      .numObjectsAfterCreated = std::nullopt,
    };
  },
  .test = [](std::any& value, objid sceneId) -> std::optional<TestRunReturn> {
    MakeObjectAttrData* objectsData = anycast<MakeObjectAttrData>(value);
    modassert(objectsData, "invalid type objectsData");

    if (objectsData -> stage == 0){
      objectsData -> stage = 1;
      objectsData -> numObjectsStart = numberOfObjectsFrameStart();
      objectsData -> sceneId = mainApi -> loadScene("./res/scenes/empty.p.rawscene",{}, std::nullopt, sceneTags);

      GameobjAttributes attr = { .attr = {} };
      std::unordered_map<std::string, GameobjAttributes> submodelAttributes = {};
      auto prefabId = mainApi -> makeObjectAttr(
          objectsData -> sceneId.value(), 
          std::string("test-child") + std::to_string(getUniqueObjId()), 
          attr, 
          submodelAttributes
      ).value();

      return std::nullopt;
    }else if (objectsData -> stage == 1){
      objectsData -> stage = 2;
      objectsData -> numObjectsAfterCreated = numberOfObjectsFrameStart();
      mainApi -> unloadScene(objectsData -> sceneId.value());
      return std::nullopt;
    }

    auto finalCount = numberOfObjectsFrameStart();
    if (finalCount != objectsData -> numObjectsStart.value()){
        return IntegTestResult { .passed = false, .reason = std::string("got = ") + std::to_string(finalCount) + std::string(", wanted = ") + std::to_string(objectsData -> numObjectsStart.value()) };
    }
    return IntegTestResult { .passed = true };
  }
};

std::string dumpDebugInfo(bool fullInfo);


struct MakeObjectAttrData2 {
  int stage;
  std::optional<int> numObjectsStart;
  std::optional<objid> mainSceneId;
  std::optional<int> numObjectsMainScene;
  std::optional<objid> orphanSceneId;
  std::optional<int> numObjectsOrphanScene;
};

// Verifies we unload the child ids in different scenes when unload scene
IntegrationTest prefabParentingTest2 {
  .name = "removeSceneMakeObjectAttrTest2",
  .createTestData = []() -> std::any {
    return MakeObjectAttrData2 {
      .stage = 0,
      .numObjectsStart = std::nullopt,
      .mainSceneId = std::nullopt,
      .numObjectsMainScene = std::nullopt,
      .orphanSceneId = std::nullopt,
      .numObjectsOrphanScene = std::nullopt,
    };
  },
  .test = [](std::any& value, objid sceneId) -> std::optional<TestRunReturn> {
    MakeObjectAttrData2* objectsData = anycast<MakeObjectAttrData2>(value);
    modassert(objectsData, "invalid type objectsData");

    if (objectsData -> stage == 0){
      objectsData -> numObjectsStart = numberOfObjectsFrameStart();
      objectsData -> mainSceneId = mainApi -> loadScene("./res/scenes/empty.p.rawscene",{}, std::nullopt, sceneTags);
      GameobjAttributes attr = { .attr = {} };
      std::unordered_map<std::string, GameobjAttributes> submodelAttributes = {};
      auto testChildId = mainApi -> makeObjectAttr(
          objectsData -> mainSceneId.value(), 
          std::string("test-child") + std::to_string(getUniqueObjId()), 
          attr, 
          submodelAttributes
      ).value();

      objectsData -> stage = 1;
      return std::nullopt;
    }else if (objectsData -> stage == 1){
      objectsData -> numObjectsMainScene = numberOfObjectsFrameStart() ;
      auto orphanSceneId = mainApi -> loadScene("./res/scenes/empty.p.rawscene",{}, std::nullopt, sceneTags);
      GameobjAttributes attr = { .attr = {} };
      std::unordered_map<std::string, GameobjAttributes> submodelAttributes = {};
      auto testChildId2 = mainApi -> makeObjectAttr(
          orphanSceneId, 
          std::string("test-child2") + std::to_string(getUniqueObjId()), 
          attr, 
          submodelAttributes
      ).value();
      objectsData -> orphanSceneId = orphanSceneId;
     // mainApi -> makeParent(testChildId2, testChildId);
      objectsData -> stage = 2;
      return std::nullopt;
    }else if (objectsData -> stage == 2){
      objectsData -> numObjectsOrphanScene = numberOfObjectsFrameStart();
      mainApi -> unloadScene(objectsData -> mainSceneId.value());
      objectsData -> stage = 3;
      return std::nullopt;
    }


    std::cout << dumpDebugInfo(false) << std::endl;
    modassert(false, "-");
    //        return IntegTestResult { .passed = false, .reason = std::string("got = ") + std::to_string(finalCount) + std::string(", wanted = ") + std::to_string(objectsData -> numObjectsStart.value()) };

    return IntegTestResult { .passed = true };
  }
};