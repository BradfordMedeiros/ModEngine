#include "./keymapper.h"

KeyRemapper readMapping(std::string filemapping, std::vector<InputDispatch> inputFns, std::function<std::string(std::string)> readFile){
  if (filemapping == ""){
    return KeyRemapper{
      .inputFns = inputFns,
    };
  }

  KeyRemapper remapper {
    .inputFns = inputFns,
  };
  return remapper;
}

