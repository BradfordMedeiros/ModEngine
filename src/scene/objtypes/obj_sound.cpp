#include "./obj_sound.h"

std::vector<AutoSerialize> soundAutoserializer {
  AutoSerializeBool {
    .structOffset = offsetof(GameObjectSound, loop),
    .field = "loop",
    .onString = "true",
    .offString = "false",
    .defaultValue = false,
  },
  AutoSerializeRequiredString {
    .structOffset = offsetof(GameObjectSound, clip),
    .field = "clip",
  },  
};

GameObjectSound createSound(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectSound obj {};
  createAutoSerialize((char*)&obj, soundAutoserializer, attr, util);

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