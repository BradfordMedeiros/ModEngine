#include "./recorder.h"

Recording createRecording(){
  return Recording{};
}

int indexFromTime(float time){
  return (int)time;
}
void saveRecordingIndex(Recording& recording, std::string name, AttributeValue value, float time){
  auto index = indexFromTime(time);
  auto property = Property {
    .propertyName = name,
    .value = value,
  };
  for (auto frame : recording.keyframes){
    for (auto keyframe : recording.keyframes){
      if (keyframe.time == index){
        keyframe.properties.push_back(property);
        return;
      }
    }
  }
  recording.keyframes.push_back(
    Record{
      .time = index,
      .properties = { property }
    }
  );
}

Recording loadRecording(std::string name, std::function<AttributeValue(std::string, std::string)> parsePropertySuffix, std::function<std::string(std::string)> readFile){
  std::string serializedData = readFile(name);
  auto properties = filterWhitespace(split(serializedData, '\n'));

  std::map<int, std::vector<Property>> values;
  for (auto property : properties){
    auto attributeLine = filterWhitespace(split(property, ':'));
    auto time = std::atoi(attributeLine.at(0).c_str());
    if (values.find(time) == values.end()){
      values[time] = {};
    }
    values.at(time).push_back(
      Property{
        .propertyName = attributeLine.at(1),
        .value = parsePropertySuffix(attributeLine.at(1), attributeLine.at(2)),
      }
    );
  }
  std::vector<int> timestamps;
  for (auto [time, _] : values){
    timestamps.push_back(time);
  }
  std::sort(timestamps.begin(), timestamps.end());

  std::vector<Record> keyframes;
  for (auto timestamp : timestamps){
    Record record {
      .time = timestamp,
      .properties = values.at(timestamp),
    };
    keyframes.push_back(record);
  }

  Recording recording  {
    .keyframes = keyframes,
  };
  return recording;
}

std::string serializeRecording(Recording& recording, std::function<std::string(std::string, AttributeValue)> serializePropertySuffix){
  std::string data = "";
  for (auto keyframe : recording.keyframes){
    auto timestampPrefix = std::to_string(keyframe.time) + ":";
    for (auto property : keyframe.properties){
      data = data + timestampPrefix + serializePropertySuffix(property.propertyName, property.value) + "\n";
    }
  }
  return data;
}
void saveRecording(std::string name, Recording& recording, std::function<std::string(std::string, AttributeValue)> serializePropertySuffix){
  saveFile(name, serializeRecording(recording, serializePropertySuffix));
}

int indexForRecording(Recording& recording, float time){
  for (int i = 0; i < recording.keyframes.size(); i++){
    if (recording.keyframes.at(i).time > time){
      return i;
    }
  }
  return recording.keyframes.size() - 1;
}

struct PropertyIndexs {
  int lowIndex;
  int highIndex;
  float percentage;
};

PropertyIndexs indexsForRecording(Recording& recording, float time){
  auto highIndex = indexForRecording(recording, time);
  auto lowIndex = highIndex - 1;
  if (lowIndex < 0){
    lowIndex = 0;
  }

  auto lowTimestamp = recording.keyframes.at(lowIndex).time;
  auto highTimestamp = recording.keyframes.at(highIndex).time;

  auto elapsed = time - lowTimestamp;
  auto totalTime = highTimestamp - lowTimestamp;
  auto percentage = elapsed / totalTime;

  PropertyIndexs properties {
    .lowIndex = lowIndex,
    .highIndex = highIndex,
    .percentage = lowIndex == highIndex ? 1.f : (percentage > 1.f ? 1.f : percentage),
  };
  return properties;
}

std::optional<Property> maybeGetProperty(std::vector<Property>& properties, std::string propertyName){
  for (auto property : properties){
    if (property.propertyName == propertyName){
      return property;
    }
  }
  return std::nullopt;
}

std::vector<Property> recordingPropertiesInterpolated(Recording& recording, float time, std::function<AttributeValue(AttributeValue, AttributeValue, float)> interpolate){
  auto recordingIndexs = indexsForRecording(recording, time);
  auto lowProperty = recording.keyframes.at(recordingIndexs.lowIndex).properties;
  auto highProperty = recording.keyframes.at(recordingIndexs.highIndex).properties;

  std::vector<Property> properties;
  for (auto property : lowProperty){
    auto nextProperty = maybeGetProperty(highProperty, property.propertyName);
    if (nextProperty.has_value()){
      properties.push_back(Property{
        .propertyName = property.propertyName,
        .value = interpolate(property.value, nextProperty -> value, recordingIndexs.percentage),
      });
    }else{
      properties.push_back(property);
    }
  }
  return properties;
}

