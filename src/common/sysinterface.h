#ifndef MOD_SYSINTERFACE
#define MOD_SYSINTERFACE

#include <functional>
#include "./util.h"
#include "../sounds/sound.h"

struct SysInterface {
  std::function<ALuint(std::string)> loadClip;
  std::function<void(std::string)> unloadClip;
  std::function<void(std::string, objid)> loadScript;
  std::function<void(std::string, objid)> unloadScript;
  std::function<float()> getCurrentTime;
};

#endif
