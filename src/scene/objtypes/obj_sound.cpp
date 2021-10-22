#include "./obj_sound.h"

GameObjectSound createSound(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto clip = attr.stringAttributes.at("clip");
  auto loop = (attr.stringAttributes.find("loop") != attr.stringAttributes.end()) && (attr.stringAttributes.at("loop") == "true");
  auto source = loadSoundState(clip, loop);
  GameObjectSound obj {
    .clip = clip,
    .source = source,
    .loop = loop,
  };
  return obj;
}

void soundObjAttr(GameObjectSound& soundObj, GameobjAttributes& _attributes){
  _attributes.stringAttributes["clip"] = soundObj.clip;
  _attributes.stringAttributes["loop"] = soundObj.loop ? "true" : "false";
}

std::vector<std::pair<std::string, std::string>> serializeSound(GameObjectSound& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  if (obj.clip != ""){
    pairs.push_back(std::pair<std::string, std::string>("clip", obj.clip));
  }
  if (obj.loop){
    pairs.push_back(std::pair<std::string, std::string>("loop", "true"));
  }
  return pairs;
}  

void removeSound(GameObjectSound& soundObj){
  unloadSoundState(soundObj.source, soundObj.clip); 
}