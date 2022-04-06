#ifndef MOD_SYSINTERFACE
#define MOD_SYSINTERFACE

#include <functional>
#include "./util.h"

struct SysInterface {
  std::function<void(std::string, objid, objid)> loadCScript;
  std::function<void(std::string, objid)> unloadCScript;
  std::function<void(objid)> stopAnimation;
  std::function<float()> getCurrentTime;
};

#endif
