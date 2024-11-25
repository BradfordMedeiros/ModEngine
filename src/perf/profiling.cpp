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
FrameStackInfo lastFrame = FrameStackInfo {
  .description = "",
  .startTime = 0.f,
  .stopTime = 0.f,
  .breakdown = {},
  .up = NULL,
};

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

void shaderStartSection(const char* str);
void shaderEndSection();

int startProfile(const char* description){
  shaderStartSection(description);
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

  //std::cout << "print stack frame start" << std::string(description) << std::endl;
  //printFrameStack(frameStack);
  return profiles.size() - 1;
}
void stopProfile(int id){
  shaderEndSection();
  if (!shouldProfile){
    return;
  }
  LogProfile& profile = profiles.at(id);
  double currentTime = glfwGetTime();
  profile.endTime = currentTime;

  assert(std::string(profile.description) == currentStack -> description);
  currentStack -> stopTime = currentTime;

  bool isStopFrame = std::string(profile.description) == "FRAME";
  if (isStopFrame){
    lastFrame = *currentStack;
  }
  currentStack = currentStack -> up;
  if (isStopFrame){
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


FrameInfo getFrameInfo(){
  auto currentTime = lastFrame.startTime;
  auto totalFrameTime = lastFrame.stopTime.value() - lastFrame.startTime;

  std::vector<double> time;
  std::vector<const char*> labels;
  for (int i = 0; i < lastFrame.breakdown.size(); i++){
    FrameStackInfo& frameInfo = lastFrame.breakdown.at(i);
    time.push_back(frameInfo.stopTime.value() - frameInfo.startTime);
    labels.push_back(frameInfo.description);
  }

  //printFrameStack(lastFrame, 0);
  // this just gets the relative frame times for the top level frame -
  //std::cout << std::endl << std::endl;
  return FrameInfo {
    .currentTime = currentTime,
    .totalFrameTime = totalFrameTime,
    .time = time,
    .labels = labels,
  };
}

