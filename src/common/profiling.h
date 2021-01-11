#ifndef MOD_PROFILING
#define MOD_PROFILING

#include <iostream>
#include <vector>
#include "./util.h"
#include <GLFW/glfw3.h>

#define PROFILE( PROFILE_NAME, BODY ) \
  {int value =  startProfile(PROFILE_NAME); \
  BODY \
  stopProfile(value); }
  

int startProfile(std::string description);
void stopProfile(int id);
std::string dumpLogInfo();

#endif
