#ifndef MOD_KEYMAPPER
#define MOD_KEYMAPPER

#include <vector>
#include <string>

struct KeyMapping {
  int sourceKey;
  int destinationKey;
};

struct KeyRemapper {
  std::vector<KeyMapping> mapping;
};

KeyRemapper readMapping(std::string filemapping);


#endif