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
struct IntegTestResult {
  bool passed;
};
struct IntegrationTest {
  const char* name;
  std::function<std::any()> createTestData;
  std::function<std::optional<IntegTestResult>(std::any&)> test;
};

struct TestRunInformation {
  int totalPassed;
  std::optional<int> currentTestIndex;
  IntegrationTest* test;
  std::any testData;
};

objid testSceneId(){
  return mainApi -> rootSceneId();
}

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
  .test = [](std::any& value) -> std::optional<IntegTestResult> {
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
  bool madeObject;
  std::optional<objid> gameobjId;
};
IntegrationTest basicMakeObjectTest {
  .name = "basicMakeObjectTest",
  .createTestData = []() -> std::any {
    return BasicMakeObject {
      .madeObject = false,
    };
  },
  .test = [](std::any& value) -> std::optional<IntegTestResult> {
    BasicMakeObject* makeObjectData = anycast<BasicMakeObject>(value);
    modassert(makeObjectData, "invalid type makeObjectData");
    if(!makeObjectData -> madeObject){
      makeObjectData -> madeObject = true;
      GameobjAttributes attr { .attr = {} };
      std::map<std::string, GameobjAttributes> submodelAttributes;
      makeObjectData -> gameobjId = mainApi -> makeObjectAttr(testSceneId(), "test-object-test", attr, submodelAttributes);
      if (!mainApi -> gameobjExists(makeObjectData -> gameobjId.value())){
        return IntegTestResult { .passed = false };
      }    
      return std::nullopt;
    }
    if (!mainApi -> gameobjExists(makeObjectData -> gameobjId.value())){
      return IntegTestResult { .passed = false };
    }

    return IntegTestResult { .passed = true };
  }
};

std::vector<IntegrationTest> integrationTests {
  wait600FramesTest,
  basicMakeObjectTest,
};

TestRunInformation createIntegrationTest(){
  return TestRunInformation {
    .currentTestIndex = std::nullopt,
    .test = NULL,
    .testData = (void*)NULL,
  };
}
TestRunInformation runInformation = createIntegrationTest();

void loadTest(TestRunInformation& runInformation, int testIndex){
  modlog("test integration loading", std::to_string(testIndex));
  runInformation.currentTestIndex = testIndex;
  runInformation.test = &integrationTests.at(runInformation.currentTestIndex.value());
  runInformation.testData = runInformation.test -> createTestData();
}

bool runIntegrationTests(TestResults* _testResults){
  if (!runInformation.currentTestIndex.has_value()){
    loadTest(runInformation, 0);
  }
  
  auto testResult = runInformation.test -> test(runInformation.testData);
  bool testFinished = testResult.has_value();
  bool moreTestsToLoad = (runInformation.currentTestIndex.value() + 1) < integrationTests.size();
  bool doneTesting = !moreTestsToLoad && testFinished;

  if (testFinished){
    modlog("test integration finished", std::string("test passed: ") + print(testResult.value().passed));
    runInformation.test = NULL;
    runInformation.testData = (void*)NULL;
    if (testResult.value().passed){
      runInformation.totalPassed++;
    }
  }

  if (moreTestsToLoad && testFinished){
    loadTest(runInformation, runInformation.currentTestIndex.value() + 1);
  }
  if (doneTesting){
    _testResults -> totalTests = integrationTests.size(),
    _testResults -> testsPassed = runInformation.totalPassed;
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