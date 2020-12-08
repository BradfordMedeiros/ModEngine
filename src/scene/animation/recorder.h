#ifndef MOD_RECORDER
#define MOD_RECORDER

#include <string>
#include <vector>
#include <algorithm>   
#include <functional>
#include "../common/util/types.h"
#include "../../common/util.h"

struct Record {
  int time;
  std::vector<Property> properties;
};

struct Recording {
  std::vector<Record> keyframes;
};


Recording loadRecording(std::string name, std::function<AttributeValue(std::string, std::string)> parsePropertySuffix); 
//void saveRecording(std::string name, Recording recording);
std::vector<Property> recordingProperties(Recording& recording, float time);

/*std::vector<AttributeValue> propertiesForRecording(Recording& recording, float currentTime);
void recordPropertiesToRecording(Recording& recording, Properties& properties);*/

#endif