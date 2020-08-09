#ifndef MOD_KEYMAPPER
#define MOD_KEYMAPPER

#include <vector>
#include <string>

struct KeyRemapping {
  int sourceKey;
  int destinationKey;
};

std::vector<KeyRemapping> readMapping(std::string filemapping);

#endif