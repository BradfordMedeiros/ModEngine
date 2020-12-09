#include "./recorder.h"

Recording loadRecording(std::string name, std::function<AttributeValue(std::string, std::string)> parsePropertySuffix){
  std::string serializedData = loadFile(name);
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

/*std::string serializeProperties(Properties& properties){
  std::string data = "position:0 0 0\n" ;
  data = data + "position:5 0 0\n";
  data = data + "position:5 1 0\n";
  data = data + "position:5 2 0\n";
  return data;
}
std::string serializeRecording(Recording& recording){
  int numTicks = 0;
  std::string serializedData = "";
  for (auto frame : recording.keyframes){
    serializedData = serializedData + "tick-placeholder(" + std::to_string(numTicks) + ")" + "\n";
    serializedData = serializedData +  serializeProperties(frame.properties) + "\n";
    serializedData = serializedData + "\n";
  }
  return serializedData;
}
void saveRecording(std::string name, Recording& recording){
  saveFile(recordingPath(name), serializeRecording(recording)); 
}*/

int indexForRecording(Recording& recording, float time){
  std::cout << "time: " << time << std::endl;
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
  auto lowTimestamp = recording.keyframes.at(lowIndex).time;
  auto highTimestamp = recording.keyframes.at(highIndex).time;

  auto elapsed = time - lowTimestamp;
  auto totalTime = highTimestamp - lowTimestamp;
  auto percentage = elapsed / totalTime;

  std::cout << "time is: " << time << std::endl;
  std::cout << "high time: " << highTimestamp << std::endl;
  std::cout << "low time: " << lowTimestamp << std::endl;
  std::cout << "percentage: " << percentage << std::endl;

  PropertyIndexs properties {
    .lowIndex = lowIndex,
    .highIndex = highIndex,
    .percentage = percentage,
  };
  return properties;
}

std::vector<Property> recordingProperties(Recording& recording, float time){
  auto recordingIndex = indexForRecording(recording, time);
  std::cout << "recording index: " << recordingIndex << std::endl;
  return recording.keyframes.at(recordingIndex).properties;
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
      std::cout << "interpolating!!!!!!!!!!!" << std::endl;
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

