#include "./util_test.h"

void utilParseAndSerializeQuat() {
  std::vector<std::string> rawQuatsTests = {
    "0 0 -1 45",
    "3 3 3 10",
    "1 2 3 30",
    "3 3 3 10"
    "1 0 -1 0",
    "4 3 34 0",
    "4 3 34 70",
    "0 1 0 30",
    "0 0 -1 30",

  };
  int numFailingTests = 0;
  std::string errorStr = "\n";
  for (int i = 0; i < rawQuatsTests.size(); i++){
    auto rawquat = rawQuatsTests.at(i);
    auto rawParsed = parseVec4(rawquat);
    auto normalizedRaw = glm::normalize(glm::vec3(rawParsed.x, rawParsed.y, rawParsed.z));
    std::cout << "\nrawParsed: " << print(rawParsed) + "\n";
    auto normalizedRaw4 = glm::vec4(normalizedRaw.x, normalizedRaw.y, normalizedRaw.z, rawParsed.w);
    auto serializedParsed = parseVec4(serializeQuat(parseQuat(rawQuatsTests.at(i))));
    if (!aboutEqual(normalizedRaw4, serializedParsed)){
      numFailingTests++;
      errorStr = errorStr + "test: " + std::to_string(i) + " - " + "got : " + print(serializedParsed) + " but wanted: " + print(normalizedRaw4) + "\n";
    }   
  }
  if (errorStr != ""){
    throw std::logic_error("num failing tests: " + std::to_string(numFailingTests) + errorStr);
  }
}