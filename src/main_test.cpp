#include "./main_test.h"

typedef void (*func_t)();
struct TestCase {
  const char* name;
  func_t test;
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
