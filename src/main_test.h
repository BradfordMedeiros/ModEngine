#ifndef MOD_TESTS
#define MOD_TESTS

#include <iostream>
#include <vector>
#include "./scene/scene_sandbox_test.h"
#include "./translations_test.h"
#include "./common/util_test.h"
#include "./modlayer_test.h"
#include "./cscript/cscript_binding.h"

int runTests();

struct TestResults {
  int totalTests;
  int testsPassed;
};

struct IntegTestResult {
  bool passed;
  std::optional<std::string> reason;
};

struct IntegTestWaitTime {
  float time;
};

typedef std::variant<IntegTestResult, IntegTestWaitTime> TestRunReturn;

struct IntegrationTest {
  const char* name;
  std::optional<float> timeout = 10.f;
  std::function<std::any()> createTestData;
  std::function<std::optional<TestRunReturn>(std::any&, objid)> test;
};

struct TestRunInformation {
  int totalPassed;
  std::optional<int> currentTestIndex;
  std::optional<float> testStartTime;
  std::optional<float> waitUntil;
  IntegrationTest* test;
  std::any testData;
  std::optional<objid> sceneId;
  std::optional<TestResults> testResults;
};
TestRunInformation createIntegrationTest();
bool runIntegrationTests(TestRunInformation& runInformation);
std::string testResultsStr(TestResults& testResults);

#endif
