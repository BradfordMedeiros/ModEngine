#include "./profiling.h"

struct LogProfile {
  double startTime;
  double endTime;
  const char* description;
};

std::vector<LogProfile> profiles;

bool shouldProfile = false;

void setShouldProfile(bool profile){
  shouldProfile = profile;
}

struct FrameStackInfo {
  const char* description;
  double startTime;
  std::optional<double> stopTime;
  std::vector<FrameStackInfo> breakdown;
  FrameStackInfo* up;
};

FrameStackInfo frameStack {
  .description = "TOP",
  .startTime = 0.f,
  .stopTime = 0.f,
  .breakdown = {},
  .up = NULL,
};
FrameStackInfo* currentStack = &frameStack;

void printFrameStack(FrameStackInfo& frame, int depth = 0){
  std::cout << std::endl;
  for (int i = 0; i < depth; i++){
    std::cout << " ";
  }
  std::cout << "framename: " << frame.description << ", start = " << std::to_string(frame.startTime) << ", stop " << (frame.stopTime.has_value() ? frame.stopTime.value() : -1) << std::endl;
  for (int i = 0; i < depth; i++){
    std::cout << " ";
  }
  std::cout << "breakdown: [ ";
  for (auto &newstack : frame.breakdown){
    printFrameStack(newstack, depth + 1);
  }
  for (int i = 0; i < depth; i++){
    std::cout << " ";
  }
  std::cout << " ]\n";
}

int startProfile(const char* description){
  if (!shouldProfile){
    return 0;
  }
  double currentTime = glfwGetTime();
  profiles.push_back(LogProfile{
    .startTime = currentTime,
    .endTime = -1,
    .description = description,
  });

  if (std::string(description) != currentStack -> description){
    currentStack -> breakdown.push_back(FrameStackInfo {
      .description = description,
      .startTime = currentTime,
      .stopTime = std::nullopt,
      .breakdown = {},
      .up = currentStack,
    });
    currentStack = &currentStack -> breakdown.at(currentStack -> breakdown.size() -1);
  }

  std::cout << "print stack frame start" << std::string(description) << std::endl;
  printFrameStack(frameStack);
  return profiles.size() - 1;
}
void stopProfile(int id){
  if (!shouldProfile){
    return;
  }
  LogProfile& profile = profiles.at(id);
  double currentTime = glfwGetTime();
  profile.endTime = currentTime;

  modassert(std::string(profile.description) == currentStack -> description, "invalid stop profile, doesn't match framestack: " + std::string(profile.description));
  currentStack -> stopTime = currentTime;
  currentStack = currentStack -> up;
  if (std::string(profile.description) == "FRAME"){
    currentStack -> breakdown = {};
  }
}

std::string dumpProfiling(){
  std::string content = "profiling information:  ";
  for (int i = 0; i < profiles.size(); i++){
    content = content + std::to_string(profiles.at(i).startTime) + " " + std::to_string(profiles.at(i).endTime) + " " + std::string(profiles.at(i).description) + "\n";
  }
  return content;
}



// mock for now, needs implementation
FrameInfo getFrameInfo(){
  //auto time = timeSeconds(true);

  // this just gets the relative frame times for the top level frame 
  std::cout << std::endl << std::endl;
  return FrameInfo {
    .currentTime = 1.f,
    .totalFrameTime = 1.f,
    .time = {},
    .labels = {},
  };
}