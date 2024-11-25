#ifndef MOD_PROFILING
#define MOD_PROFILING

#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>
#include <optional>
#include <assert.h>

#define PROFILE( PROFILE_NAME, BODY ... ) \
  {int value =  startProfile(PROFILE_NAME); \
  BODY \
  stopProfile(value); }
  
void setShouldProfile(bool profile);
int startProfile(const char* description);
void stopProfile(int id);
std::string dumpProfiling();

struct FrameInfo {
  double currentTime;
  double totalFrameTime;
  std::vector<double> time;
  std::vector<const char*> labels;
};

FrameInfo getFrameInfo();
std::optional<float> getLastDuration(const char* description);

#endif
