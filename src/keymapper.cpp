#include "./keymapper.h"

KeyRemapper readMapping(std::string filemapping){
  std::vector<KeyMapping> mapping;
  KeyRemapper remapper {
    .mapping = mapping,
  };
  if (filemapping == ""){
    return remapper;
  }

  // swap A and D
  mapping.push_back(KeyMapping{
    .sourceKey = 65,
    .destinationKey = 68,
  });
  mapping.push_back(KeyMapping{
    .sourceKey = 68,
    .destinationKey = 65,
  });

  return remapper;
}