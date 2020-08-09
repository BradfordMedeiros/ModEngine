#include "./keymapper.h"

KeyRemapper readMapping(std::string filemapping){
  if (filemapping == ""){
    return KeyRemapper{};
  }

  std::vector<KeyMapping> mapping;
  mapping.push_back(KeyMapping{
    .sourceKey = 65,
    .destinationKey = 68,
  });
  mapping.push_back(KeyMapping{
    .sourceKey = 68,
    .destinationKey = 65,
  });
  
  KeyRemapper remapper {
    .mapping = mapping,
  };
  return remapper;
}