#include "./main_test.h"

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


struct IntegTestResult {
  bool passed;
};
struct IntegrationTest {
  const char* name;
  std::function<std::any()> createTestData;
  std::function<std::optional<IntegTestResult>()> test;
};

struct TestRunInformation {
  std::optional<int> currentTestIndex;
  IntegrationTest* test;
  std::any testData;
};

struct TestOneInformation {
  bool madeObject;
  bool deletedObject;
};

int framecount = 0;
IntegrationTest sampleTestIntegration {
  .name = "make object test",
  .createTestData = []() -> std::any {
    return TestOneInformation {
      .madeObject = false,
      .deletedObject = false,
    };
  },
  .test = []() -> std::optional<IntegTestResult> {
    if (framecount > 1000){
      return IntegTestResult {
        .passed = true,
      };
    }
    return std::nullopt;
  }
};

std::vector<IntegrationTest> integrationTests {
  sampleTestIntegration,
  sampleTestIntegration,
  sampleTestIntegration,
};

TestRunInformation runInformation {
  .currentTestIndex = std::nullopt,
  .test = NULL,
  .testData = (void*)NULL,
};


int totalPassed = 0;

void loadTest(TestRunInformation& runInformation, int testIndex){
  modlog("test integration loading", std::to_string(testIndex));

  runInformation.currentTestIndex = testIndex;
  runInformation.test = &integrationTests.at(runInformation.currentTestIndex.value());
  runInformation.testData = runInformation.test -> createTestData();
}

bool runIntegrationTests(TestResults* _testResults){
  framecount++;

  if (!runInformation.currentTestIndex.has_value()){
    loadTest(runInformation, 0);
  }
  
  auto testResult = runInformation.test -> test();
  bool testFinished = testResult.has_value();
  bool moreTestsToLoad = (runInformation.currentTestIndex.value() + 1) < integrationTests.size();
  bool doneTesting = !moreTestsToLoad && testFinished;

  if (testFinished){
    modlog("test integration finished", std::string("test passed: ") + print(testResult.value().passed));
    runInformation.test = NULL;
    runInformation.testData = (void*)NULL;
    if (testResult.value().passed){
      totalPassed++;
    }
  }

  if (moreTestsToLoad && testFinished){
    loadTest(runInformation, runInformation.currentTestIndex.value() + 1);
  }
  if (doneTesting){
    _testResults -> totalTests = integrationTests.size(),
    _testResults -> testsPassed = totalPassed;
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