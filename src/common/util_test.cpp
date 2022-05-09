#include "./util_test.h"

void utilParseAndSerializeQuat() {
  std::vector<std::string> rawQuatsTests = {
    "0 0 -1 0",
    "0 0 -1 45",
    "1 2 3 30",
    "1 0 -1 0",
  };
  for (auto &rawquat : rawQuatsTests){
    auto rawParsed = parseVec4(rawquat);
    auto serializedParsed = parseVec4(serializeQuat(parseQuat(rawquat)));
    if (!aboutEqual(rawParsed, serializedParsed)){
      throw std::logic_error("extracted vectors are different raw, parsed - " + print(rawParsed) + " vs " + print(serializedParsed) + "\n");
    }
  }
}