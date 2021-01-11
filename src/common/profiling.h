#ifndef MOD_PROFILING
#define MOD_PROFILING

#include <iostream>
#include <vector>
#include "./util.h"

#define PROFILE( PROFILE_NAME, BODY ) \
  startProfile(PROFILE_NAME, 0); \
  BODY \
  stopProfile(0);

void startProfile(std::string description, int id);
void stopProfile(int id);

#endif
