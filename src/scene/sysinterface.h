#ifndef MOD_SYSINTERFACE
#define MOD_SYSINTERFACE

#include <functional>
#include "../common/util.h"
#include "./sprites/sprites.h"

struct SysInterface {
  std::function<void(std::string, objid, objid)> loadCScript;
  std::function<void(std::string, objid)> unloadCScript;
  std::function<void(objid)> stopAnimation;
  std::function<float()> getCurrentTime;
  std::function<std::string(std::string)> readFile;
  std::function<std::string(std::string)> modlayerPath;
  std::function<FontFamily&(std::string)> fontFamilyByName;
};

#endif
