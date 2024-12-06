#include "./util_test.h"


void utilParseAndSerializeQuatTest() {
  std::vector<std::string> rawQuatsTests = {
    "0 0 -1 45",  // 0 
    "3 3 3 10",   // 1
    "3 3 3 250",  // 2
    "1 2 3 30",   // 3
    "3 3 3 10",    // 4
    "1 0 -1 0",   // 5
    "4 3 34 0",   // 6
    "4 3 34 70",  // 7 
    "0 0 -1 30",  // 8
    "2 -1 0 250", // 9
    "0 0 -1 30",  // 10
    "0 1 0 30",   // 11
    "0 -1 0 0",   // 12
    "0 1 0 0",   // 13
    "0 1 0 354",   // 14
  };
  int numFailingTests = 0;
  std::string errorStr = "\n";
  for (int i = 0; i < rawQuatsTests.size(); i++){
    auto rawquat = rawQuatsTests.at(i);
    auto rawParsed = parseVec4(rawquat);
    auto normalizedRaw = glm::normalize(glm::vec3(rawParsed.x, rawParsed.y, rawParsed.z));
    auto normalizedRaw4 = glm::vec4(normalizedRaw.x, normalizedRaw.y, normalizedRaw.z, rawParsed.w);
    auto serializedParsed = parseVec4(serializeQuat(parseQuat(parseVec4(rawQuatsTests.at(i)))));
    if (!aboutEqual(normalizedRaw4, serializedParsed)){
      numFailingTests++;
      errorStr = errorStr + "test: " + std::to_string(i) + " - " + "got : " + print(serializedParsed) + " but wanted: " + print(normalizedRaw4) + " - original: " + print(rawParsed) + "\n";
    }   
  }
  if (errorStr != ""){
    throw std::logic_error("num failing tests: " + std::to_string(numFailingTests) + errorStr);
  }
}

struct orientationPosTestPair {
  glm::vec3 fromPos;
  glm::vec3 toPos;
  glm::vec3 expectedPos;
};
void orientationFromPosTest(){
  std::vector<orientationPosTestPair> tests = {
    orientationPosTestPair {
      .fromPos = glm::vec3(0.f, 0.f, 0.f),
      .toPos = glm::vec3(1.f, 0.f, 0.f),
      .expectedPos = glm::vec3(1.f, 0.f, 0.f),
    },
    orientationPosTestPair {
      .fromPos = glm::vec3(1.f, 1.f, 0.f),
      .toPos = glm::vec3(2.f, 2.f, 0.f),
      .expectedPos = glm::vec3(0.707107, 0.707107, 0.f),
    }, 
    orientationPosTestPair {
      .fromPos = glm::vec3(2.f, 2.f, 0.f),
      .toPos = glm::vec3(1.f, 1.f, 0.f), 
      .expectedPos = glm::vec3(-0.707107, -0.707107, 0.f),
    },
    orientationPosTestPair {
      .fromPos = glm::vec3(0.f, 0.f, 0.f),
      .toPos = glm::vec3(0.f, 2.f, 0.f),
      .expectedPos = glm::vec3(0.f, 1.f, 0.f),
    },
    orientationPosTestPair {
      .fromPos = glm::vec3(0.f, 2.f, 0.f),
      .toPos = glm::vec3(0.f, 0.f, 0.f),
      .expectedPos = glm::vec3(0.f, -1.f, 0.f),
    },  
  };
  for (auto &test : tests){
    auto direction = orientationFromPos(test.fromPos, test.toPos);
    auto newPosition = direction * glm::vec3(0.f, 0.f, -1.f);
    if (!aboutEqual(newPosition, test.expectedPos)){
      throw std::logic_error("unexpected position: expected: " + print(test.expectedPos) + " got: " + print(newPosition));
    }
  }
}


struct envSubstTestContent {
  std::string content;
  std::string result;
  std::unordered_map<std::string, std::string> templateArgs;
};

void envSubstTest(){
  std::vector<envSubstTestContent> tests = {
    envSubstTestContent {
      .content = "$number",
      .result = "123",
      .templateArgs = { {"number", "123" }},
    },
    envSubstTestContent {
      .content = "$number",
      .result = "456",
      .templateArgs = { {"number", "456" }},
    },
    envSubstTestContent {
      .content = "$another",
      .result = "hello",
      .templateArgs = { {"another", "hello" }},
    },
    envSubstTestContent {
      .content = "$one ",
      .result = "this is one ",
      .templateArgs = { {"one", "this is one" } },
    },
    envSubstTestContent {
      .content = "$three$four",
      .result = "anotherhello",
      .templateArgs = { {"three", "another" }, { "four", "hello" }},
    },
    envSubstTestContent {
      .content = "$five\n$six",
      .result = "hello\nworld",
      .templateArgs = { {"five", "hello" }, {"six", "world" }},
    },
    envSubstTestContent {
      .content = "$five [value]\n$six",
      .result = "hello [value]\nworld",
      .templateArgs = { {"five", "hello" }, {"six", "world" }},
    },
  };

  for (auto &test : tests){
    std::string templatedValue = envSubst(test.content, test.templateArgs).result;
    if (test.result != templatedValue){
      throw std::logic_error("unexpected substitution: expected: " + test.result + "(" + std::to_string(test.result.size()) + ")" + " got: " + templatedValue + "(" +  std::to_string(templatedValue.size()) + ")");
    }
  }
}