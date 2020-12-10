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

Recording createRecording();
void saveRecordingIndex(Recording& recording, std::string name, AttributeValue value, float time);

Recording loadRecording(std::string name, std::function<AttributeValue(std::string, std::string)> parsePropertySuffix);
void saveRecording(std::string name, Recording& recording, std::function<std::string(std::string, AttributeValue)> serializePropertySuffix);

std::vector<Property> recordingPropertiesInterpolated(Recording& recording, float time, std::function<AttributeValue(AttributeValue, AttributeValue, float)> interpolate);

#endif