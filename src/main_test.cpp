#include "./main_test.h"

extern CustomApiBindings* mainApi;

///Unit tests ///////////////
struct TestCase {
  const char* name;
  std::function<void()> test;
};

void sampleTest(){ 
  //throw std::logic_error("error loading buffer");
}

std::vector<TestCase> tests = { 
  TestCase{
    .name = "sample_test",
    .test = sampleTest,
  },
  TestCase {
    .name = "sandboxBasicDeserialization",
    .test = sandboxBasicDeserialization,
  },
  TestCase {
    .name = "sandboxParentPosition",
    .test = sandboxParentPosition,
  },
  TestCase {
    .name = "sandboxMakeParentPosition",
    .test = sandboxMakeParentPosition,
  },
  TestCase {
    .name = "sandboxUpdateParentRelative",
    .test = sandboxUpdateParentRelative,
  },
  TestCase {
    .name = "sandboxUpdateParentAbsolute",
    .test = sandboxUpdateParentAbsolute,
  },
  TestCase {
    .name = "sandboxUpdateParentAndChildRelative",
    .test = sandboxUpdateParentAndChildRelative,
  },
  TestCase {
    .name = "sandboxRelativeTransform",
    .test = sandboxRelativeTransform,
  },
  TestCase {
    .name = "moveRelativeIdentityTest",
    .test = moveRelativeIdentityTest,
  },
  TestCase {
    .name = "moveRelativeRotateRight",
    .test = moveRelativeRotateRight,
  },
  TestCase {
    .name = "calcLineIntersectionTest",
    .test = calcLineIntersectionTest,
  },
  TestCase {
    .name = "utilParseAndSerializeQuat",
    .test = utilParseAndSerializeQuatTest,
  },
  TestCase {
    .name = "orientationFromPosTest",
    .test = orientationFromPosTest,
  },
  TestCase {
    .name = "directionToQuatConversionTest",
    .test = directionToQuatConversionTest,
  },
  TestCase {
    .name = "modlayerPathTest",
    .test = modlayerPathTest,
  },
  TestCase {
    .name = "planeIntersectionTest",
    .test = planeIntersectionTest,
  },
};

int runTests(){
  int totalTests = tests.size();
  int numFailures = 0;
  for (int i = 0; i < tests.size(); i++){
    auto test = tests.at(i);
    try {
      test.test();
      std::cout << i << " : " << test.name << " : pass" << std::endl;
    }catch(std::logic_error ex){
      std::cout << i << " : " << test.name << " : fail - " << ex.what() << std::endl;
      numFailures++;
    }catch(...){
      std::cout << i << " : " << test.name << " : fail -- error unknown" << std::endl;
      numFailures++;
    }
  }
  return numFailures == 0 ? 0 : 1;
}


///Integration tests ///////////////
struct Wait600Frames {
  int currentFrame;
};
IntegrationTest wait600FramesTest {
  .name = "wait600FramesTest",
  .createTestData = []() -> std::any {
    return Wait600Frames {
      .currentFrame = 0,
    };
  },
  .test = [](std::any& value, objid sceneId) -> std::optional<IntegTestResult> {
    Wait600Frames* waitFrames = anycast<Wait600Frames>(value);
    modassert(waitFrames, "invalid type waitFrames");
    if (waitFrames -> currentFrame < 600){
      waitFrames -> currentFrame++;
      return std::nullopt;
    }
    return IntegTestResult {
      .passed = true,
    };
  }
};

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
  .test = [](std::any& value, objid sceneId) -> std::optional<IntegTestResult> {
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

struct CheckNumberGameObjects {

};
IntegrationTest checkUnloadSceneTest {
  .name = "checkUnloadSceneTest",
  .createTestData = []() -> std::any {
    return CheckNumberGameObjects {};
  },
  .test = [](std::any& value, objid sceneId) -> std::optional<IntegTestResult> {
    CheckNumberGameObjects* objectsData = anycast<CheckNumberGameObjects>(value);
    modassert(objectsData, "invalid type makeObjectData");
    return std::nullopt;
    //return IntegTestResult { .passed = true };
  }
};

std::vector<IntegrationTest> integrationTests {
  wait600FramesTest,
  basicMakeObjectTest,
  checkUnloadSceneTest,
};

TestRunInformation createIntegrationTest(){
  return TestRunInformation {
    .currentTestIndex = std::nullopt,
    .testStartTime = std::nullopt,
    .waitUntil = std::nullopt,
    .test = NULL,
    .testData = (void*)NULL,
    .sceneId = std::nullopt,
  };
}

void loadTest(TestRunInformation& runInformation, int testIndex){
  modlog("test integration loading", std::to_string(testIndex));
  runInformation.currentTestIndex = testIndex;
  runInformation.testStartTime = mainApi -> timeSeconds(true);
  runInformation.test = &integrationTests.at(runInformation.currentTestIndex.value());
  runInformation.testData = runInformation.test -> createTestData();

  static std::vector<std::string> sceneTags { "integ-test" };
  if (runInformation.sceneId.has_value()){
    runInformation.sceneId = std::nullopt;
    auto integrationTestingScenes = mainApi -> listScenes(sceneTags);
    for (auto sceneId : integrationTestingScenes){
      mainApi -> unloadScene(sceneId);
    }
  }
  runInformation.sceneId = mainApi -> loadScene("./res/scenes/empty.p.rawscene",{}, std::nullopt, sceneTags);

  auto allScenes = mainApi -> listScenes(sceneTags);
  modlog("test integration loaded scenes", print(allScenes));
}

bool runIntegrationTests(TestRunInformation& runInformation){
  if (runInformation.testResults.has_value()){
    return true;
  }
  if (!runInformation.currentTestIndex.has_value()){
    loadTest(runInformation, 0);
  }


  auto testResult = runInformation.test -> test(runInformation.testData, runInformation.sceneId.value());
  auto integrationWaitTime = (testResult.has_value() ? std::get_if<IntegTestWaitTime>(&testResult.value()) : NULL);
  
  if (runInformation.waitUntil.has_value()){
    if (mainApi -> timeSeconds(true) > runInformation.waitUntil.value()){
      runInformation.waitUntil = std::nullopt;
    }
  }else if (integrationWaitTime){
    runInformation.waitUntil = mainApi -> timeSeconds(true) + 5.f;
    return false;
  }

  auto currentTime = mainApi -> timeSeconds(true);
  bool timedOut = runInformation.test -> timeout.has_value() && (currentTime - runInformation.testStartTime.value() > runInformation.test -> timeout.value());

  bool testFinished = (testResult.has_value() && std::get_if<IntegTestResult>(&testResult.value())) || timedOut;
  bool moreTestsToLoad = (runInformation.currentTestIndex.value() + 1) < integrationTests.size();
  bool doneTesting = !moreTestsToLoad && testFinished;

  if (testFinished){
    runInformation.test = NULL;
    runInformation.testData = (void*)NULL;
    runInformation.sceneId = std::nullopt;
    runInformation.testStartTime = std::nullopt;
    if (testResult.has_value()){
      auto testResultValue = std::get_if<IntegTestResult>(&testResult.value());
      if (testResultValue && testResultValue -> passed){
        runInformation.totalPassed++;
      }
    }
  }

  if (moreTestsToLoad && testFinished){
    loadTest(runInformation, runInformation.currentTestIndex.value() + 1);
  }
  if (doneTesting){
    runInformation.testResults = TestResults {
      .totalTests = static_cast<int>(integrationTests.size()),
      .testsPassed = runInformation.totalPassed,
    };
  }
  return doneTesting;
}

std::string testResultsStr(TestResults& testResults){
  std::string value;
  value += std::string("test integration tests = ") + std::to_string(testResults.totalTests) + "\n";
  value += std::string("test integration passed = ") + std::to_string(testResults.testsPassed) + "\n";
  value += std::string("test integration failed = ") + std::to_string(testResults.totalTests - testResults.testsPassed) + "\n";
  return value;
}