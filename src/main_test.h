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
bool runIntegrationTests(TestResults* _testResults);
std::string testResultsStr(TestResults& testResults);

#endif
