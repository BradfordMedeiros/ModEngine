#ifndef MOD_PROFILING
#define MOD_PROFILING

#include <iostream>
#include <vector>
#include "../common/util.h"
#include <GLFW/glfw3.h>

#define PROFILE( PROFILE_NAME, BODY ... ) \
  {int value =  startProfile(PROFILE_NAME); \
  BODY \
  stopProfile(value); }
  
void setShouldProfile(bool profile);
int startProfile(const char* description);
void stopProfile(int id);
std::string dumpProfiling();

FrameInfo getFrameInfo();

#endif
