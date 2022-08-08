#include "./obj_sound.h"

GameObjectSound createSound(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectSound obj {};
  attrSetRequired(attr, &obj.clip, "clip");
  attrSet(attr, &obj.loop, "true", "false", false, "loop", false);
  obj.source = loadSoundState(util.pathForModLayer(obj.clip), obj.loop);
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

void removeSound(GameObjectSound& soundObj, ObjectRemoveUtil& util){
  unloadSoundState(soundObj.source, soundObj.clip); 
}