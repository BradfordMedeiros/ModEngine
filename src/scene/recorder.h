#ifndef MOD_RECORDER
#define MOD_RECORDER

#include <string>
#include <vector>
#include "./common/util/types.h"
#include "../common/util.h"

struct Record {
  float time;
  Properties properties;
};

struct Recording {
  std::vector<Record> keyframes;
};

Recording loadRecording(std::string name); 
void saveRecording(std::string name, Recording recording);

Properties propertiesForRecording(Recording& recording, float currentTime);
void recordPropertiesToRecording(Recording& recording, Properties& properties);

#endif