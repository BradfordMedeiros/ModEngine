#include "./recorder.h"

std::string recordingPath(std::string name){
  return std::string("./res/recordings/") + name;   // @TODO make this safter/better
}


Recording loadRecording(std::string name){
  std::string serializedData = loadFile(std::string("./res/recordings/" + name));
  auto properties = split(serializedData, '\n');

  std::vector<Record> keyframes;
  int index = 0;
  for (auto property : properties){
    auto attributeLine = split(property, ':');
    if (attributeLine.at(0) == "position"){
      Transformation transformation {
        .position = parseVec(attributeLine.at(1)),
      };
      Properties properties {
        .transformation = transformation,
      };
      Record record {
        time : index, 
        properties : properties,
      };
    }
  }

  Recording recording  {
    .keyframes = keyframes,
  };
  return recording;
}

std::string serializeProperties(Properties& properties){
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
}

int indexForRecording(Recording& recording, float time){
  int index = 0;
  for (int i = 0; i < recording.keyframes.size(); i++){
    if (recording.keyframes.at(i).time > time){
      index = i;
    }
  }
  return index;
}

Properties propertiesForRecording(Recording& recording, float time){
  return recording.keyframes.at(indexForRecording(recording, time)).properties;  
}
void recordPropertiesToRecording(Recording& recording, Properties& properties){
  
}

void startRecording(){
  
}