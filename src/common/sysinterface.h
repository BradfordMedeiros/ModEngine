#ifndef MOD_SYSINTERFACE
#define MOD_SYSINTERFACE

#include <functional>
#include "./util.h"

struct SysInterface {
  std::function<void(std::string, objid, objid)> loadScript;
  std::function<void(std::string, objid)> unloadScript;
  std::function<void(objid)> stopAnimation;
  std::function<float()> getCurrentTime;
  std::function<void()> onCreateCustomElement;
  std::function<void()> onRemoveCustomElement;
};

#endif
