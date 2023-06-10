#include "./profiling.h"

struct LogProfile {
  double startTime;
  double endTime;
  bool complete;
  const char* description;
};

std::vector<LogProfile> profiles;

bool shouldProfile = false;

void setShouldProfile(bool profile){
  shouldProfile = profile;
}

struct CurrentFrameData {
  std::vector<float> times;
  std::vector<const char*> labels;
};

CurrentFrameData currentFrame {
  .times = {},
  .labels = {},
};
CurrentFrameData lastFrame {
  .times = {},
  .labels = {},
};

int startProfile(const char* description){
  if (!shouldProfile){
    return 0;
  }

  double currentTime = glfwGetTime();

  if (std::string(description) == "FRAME"){
    lastFrame = currentFrame;
    currentFrame.times = {};
    currentFrame.labels = {};
  }
  currentFrame.times.push_back(currentTime); 
  currentFrame.labels.push_back(description);


  profiles.push_back(LogProfile{
    .startTime = currentTime,
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
    content = content + std::to_string(profiles.at(i).startTime) + " " + std::to_string(profiles.at(i).endTime) + " " + std::string(profiles.at(i).description) + "\n";
  }
  return content;
}

// mock for now, needs implementation
FrameInfo getFrameInfo(){
  //auto time = timeSeconds(true);

  if (lastFrame.times.size() == 0){
    return FrameInfo {
      .currentTime = 0.f,
      .totalFrameTime = 0.f,
      .time = {},
      .labels = {},
    };
  }

  double currentTime = lastFrame.times.at(0);
  double totalFrameTime = lastFrame.times.at(lastFrame.times.size() -1) - currentTime;
  std::vector<double> times = {};
  std::vector<const char*> labels = {};
  for (int i = 1; i < lastFrame.times.size(); i++){
    float length = lastFrame.times.at(i) - lastFrame.times.at(i - 1);
    times.push_back(length);
    labels.push_back(lastFrame.labels.at(i));
  }

  std::cout << "Frame info: " << std::endl;
  std::cout << "current time: " << std::to_string(currentTime) << std::endl;
  std::cout << "times: [";
  for (auto time : times) {
    std::cout << time << " ";
  }
  std::cout << "]";
  std::cout << std::endl << std::endl;
  return FrameInfo {
    .currentTime = currentTime,
    .totalFrameTime = totalFrameTime,
    .time = times,
    .labels = labels,
  };
}