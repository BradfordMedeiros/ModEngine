#ifndef MOD_TEST_INTEGRATION_COMMON
#define MOD_TEST_INTEGRATION_COMMON

#include <optional>
#include <functional>
#include <variant>
#include <map>
#include "../../scene/scene_sandbox_test.h"
#include "../../translations_test.h"
#include "../../common/util_test.h"
#include "../../modlayer_test.h"
#include "../../cscript/cscript_binding.h"


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

extern std::vector<std::string> sceneTags;

#endif
