#ifndef MOD_KEYMAPPER
#define MOD_KEYMAPPER

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <functional>
#include "./common/util.h"

enum DispatchType { BUTTON_PRESS, BUTTON_RELEASE, BUTTON_HOLD };

struct ViewportSettings;
struct InputDispatch {
  bool alwaysEnable;
  int sourceKey;
  DispatchType sourceType;
  int prereqKey;
  bool hasPreq;
  std::function<void(ViewportSettings& viewport)> fn;
};

struct KeyRemapper {
  std::vector<InputDispatch> inputFns;

  // Cached Data
  std::unordered_map<int, bool> lastFrameDown;
};

KeyRemapper readMapping(std::string filemapping, std::vector<InputDispatch> inputFns, std::function<std::string(std::string)> readFile);

#endif