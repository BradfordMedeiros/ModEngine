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

Recording loadRecording(std::string name, std::function<AttributeValue(std::string, std::string)> parsePropertySuffix, std::function<std::string(std::string)> readFile);
void saveRecording(std::string name, Recording& recording, std::function<std::string(std::string, AttributeValue)> serializePropertySuffix);

std::vector<Property> recordingPropertiesInterpolated(Recording& recording, float time, std::function<AttributeValue(AttributeValue, AttributeValue, float)> interpolate, float recordingStartTime, RecordingPlaybackType type, bool* _isComplete);

// property suffix looks like the parts of the tokens on the right hand side
// eg position 10
// eg tint 0.9 0.2 0.4
AttributeValue parsePropertySuffix(std::string key, std::string value);

#endif