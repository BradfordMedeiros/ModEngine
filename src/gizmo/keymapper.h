#ifndef MOD_KEYMAPPER
#define MOD_KEYMAPPER

#include <vector>
#include <map>
#include <string>
#include <iostream>
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

struct KeyRemapper {
  std::vector<KeyMapping> mapping;                          // ascii value key to ascii value key
  std::map<int, KeyAxisConfiguration> axisConfigurations;
};

KeyRemapper readMapping(std::string filemapping);
int getKeyRemapping(KeyRemapper& keymapper, int key);

#endif