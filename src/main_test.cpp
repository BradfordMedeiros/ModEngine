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

struct IntegrationTest {
  const char* name;
  std::function<std::any()> createTestData;
  std::function<bool()> test;
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
// this test should make an object, and then it will verify it exists
// and then it will delete it, and verify it does not exist
IntegrationTest sampleTestIntegration {
  .name = "make object test",
  .createTestData = []() -> std::any {
    return TestOneInformation {
      .madeObject = false,
      .deletedObject = false,
    };
  },
  .test = []() -> bool {
    return false;
  }
};

TestRunInformation runInformation {
  .currentTestIndex = std::nullopt,
  .test = NULL,
  .testData = (void*)NULL,
};


int framecount = 0;

bool runIntegrationTests(TestResults* _testResults){
  if (!runInformation.currentTestIndex.has_value()){
    runInformation.test = &sampleTestIntegration;
    runInformation.testData = runInformation.test -> createTestData();
    runInformation.currentTestIndex = 0;
  }

  framecount++;
  if (runInformation.test){
    auto testFinished = runInformation.test -> test();
    if (testFinished){
      bool testPassed = true;
      modlog("test finished", std::string("test passed: ") + print(testPassed));
    
      runInformation.test = NULL;
      runInformation.testData = (void*)NULL;
    }    
  }
  bool complete = framecount > 760;
  if (complete){
    _testResults -> totalTests = 10,
    _testResults -> testsPassed = 5;
  }
  return complete;
}

std::string testResultsStr(TestResults& testResults){
  std::string value;
  value += std::string("tests = ") + std::to_string(testResults.totalTests) + "\n";
  value += std::string("passed = ") + std::to_string(testResults.testsPassed) + "\n";
  value += std::string("failed = ") + std::to_string(testResults.totalTests - testResults.testsPassed) + "\n";
  return value;
}