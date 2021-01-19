#ifndef MOD_KEYMAPPER
#define MOD_KEYMAPPER

#include <vector>
#include <string>
#include <iostream>
#include "../common/util.h"

struct KeyMapping {
  int sourceKey;
  int destinationKey;
};

struct KeyAxisConfiguration {
  int index;
  bool invert;
  float deadzonemin;
  float deadzonemax;
  bool shouldMapKey;
  float amount;
  int destinationKey;
};

struct KeyRemapper {
  std::vector<KeyMapping> mapping;                          // ascii value key to ascii value key
  std::vector<KeyMapping> buttonMappings;                   // index of the controller to ascii value key
  std::vector<KeyAxisConfiguration> axisConfigurations; 
};

KeyRemapper readMapping(std::string filemapping);
int getKeyRemapping(KeyRemapper& keymapper, int key);
KeyAxisConfiguration getAxisConfig(KeyRemapper& keymapper, int index);

#endif