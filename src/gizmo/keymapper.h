#ifndef MOD_KEYMAPPER
#define MOD_KEYMAPPER

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <functional>
#include "../common/util.h"

struct KeyMapping {
  int sourceKey;
  int destinationKey;
};

struct KeyAxisConfiguration {
  bool invert;
  float deadzonemin;
  float deadzonemax;
  bool shouldMapKey;
  float amount;
  int destinationKey;
  bool hasKeyMapping;
  KeyMapping mapping;
};

enum DispatchType { BUTTON_PRESS, BUTTON_RELEASE, BUTTON_HOLD };
struct InputDispatch {
  int sourceKey;
  DispatchType sourceType;
  int prereqKey;
  bool hasPreq;
  std::function<void()> fn;
};

struct KeyRemapper {
  std::vector<KeyMapping> mapping;                          // ascii value key to ascii value key
  std::map<int, KeyAxisConfiguration> axisConfigurations;
  std::vector<InputDispatch> inputFns;

  // Cached Data
  std::map<int, bool> lastFrameDown;
};

KeyRemapper readMapping(std::string filemapping, std::vector<InputDispatch> inputFns);
int getKeyRemapping(KeyRemapper& keymapper, int key);

#endif