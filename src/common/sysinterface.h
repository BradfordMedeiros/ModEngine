#ifndef MOD_SYSINTERFACE
#define MOD_SYSINTERFACE

#include <functional>

struct SysInterface {
  std::function<void(std::string)> loadClip;
  std::function<void(std::string)> unloadClip;
};

#endif
