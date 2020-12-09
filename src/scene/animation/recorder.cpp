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
  auto lowIndex = indexForRecording(recording, time);
  auto highIndex = lowIndex + 1;
  auto lowTimestamp = recording.keyframes.at(lowIndex).time;
  auto highTimestamp = recording.keyframes.at(highIndex).time;
  auto percentage = 0.5f;
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

std::vector<Property> recordingPropertiesInterpolated(Recording& recording, float time){
  auto recordingIndexs = indexsForRecording(recording, time);
  auto lowProperty = recording.keyframes.at(recordingIndexs.lowIndex).properties;
  auto highProperty = recording.keyframes.at(recordingIndexs.highIndex).properties;
  return lowProperty;
}

/*
void recordPropertiesToRecording(Recording& recording, Properties& properties){
  
}

void startRecording(){
  
}*/