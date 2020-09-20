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

struct KeyRemapper {
  std::vector<KeyMapping> mapping;
};

KeyRemapper readMapping(std::string filemapping);
int getKeyRemapping(KeyRemapper& keymapper, int key);

#endif