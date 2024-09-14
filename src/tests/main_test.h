#ifndef MOD_TESTS
#define MOD_TESTS

#include <iostream>
#include <vector>
#include "../scene/scene_sandbox_test.h"
#include "../translations_test.h"
#include "../common/util_test.h"
#include "../modlayer_test.h"
#include "../scene/animation/recorder_test.h"
#include "../cscript/cscript_binding.h"
#include "./integration/common.h"
#include "./integration/makeobject.h"
#include "./features/selection_binding.h"

int runTests();

struct TestResults {
  int totalTests;
  int testsPassed;
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

struct FeatureScene {
  std::optional<std::string> sceneFile;
  std::optional<std::function<CScriptBinding(CustomApiBindings& api)>> createBinding;
  bool scriptAuto = false;
};

void printFeatureSceneHelp();

FeatureScene& getFeatureScene(std::string name);
void runFeatureScene(std::string name);

#endif
