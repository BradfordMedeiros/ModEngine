#include "./profiling.h"

struct LogProfile {
  double startTime;
  double endTime;
  bool complete;
  std::string description;
};

std::vector<LogProfile> profiles;

struct CurrentFrame {
  const char* frame;
};

bool shouldProfile = false;

void setShouldProfile(bool profile){
  shouldProfile = profile;
}

int startProfile(const char* description){
  if (!shouldProfile){
    return 0;
  }
  profiles.push_back(LogProfile{
    .startTime = glfwGetTime(),
    .endTime = -1,
    .complete = false,
    .description = description,
  });
  return profiles.size() - 1;
}
void stopProfile(int id){
  if (!shouldProfile){
    return;
  }
  LogProfile& profile = profiles.at(id);
  profile.endTime = glfwGetTime();
  profile.complete = true;
}

std::string dumpProfiling(){
  std::string content = "profiling information:  ";
  for (int i = 0; i < profiles.size(); i++){
    content = content + std::to_string(profiles.at(i).startTime) + " " + std::to_string(profiles.at(i).endTime) + " " + profiles.at(i).description + "\n";
  }
  return content;
}

// mock for now, needs implementation
FrameInfo getFrameInfo(){
  //auto time = timeSeconds(true);
  double totalFrameTime = 0.015;  // ~60fps 
  return FrameInfo {
    .currentTime = 5.f,
    .totalFrameTime = totalFrameTime,
    .time = { 0.01, 0.005 },
  };
}