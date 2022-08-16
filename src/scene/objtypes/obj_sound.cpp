#include "./obj_sound.h"

std::vector<AutoSerialize> soundAutoserializer {
  AutoSerializeBool {
    .structOffset = offsetof(GameObjectSound, loop),
    .field = "loop",
    .onString = "true",
    .offString = "false",
    .defaultValue = false,
  },
  AutoSerializeCustom {
    .structOffset = 0,
    .field = "clip",
    .fieldType = ATTRIBUTE_STRING,
    .deserialize = [](ObjectTypeUtil& util, void* offset, void* fieldValue) -> void {
      GameObjectSound* obj = static_cast<GameObjectSound*>(offset);
      std::string* clip = static_cast<std::string*>(fieldValue);
      if (fieldValue == NULL){
        modassert(false, "clip must not be unspecified");
      }else{
        obj -> clip = *clip;
        obj -> source = loadSoundState(util.pathForModLayer(obj -> clip), obj -> loop);     
      }
    },
    .setAttributes = [](ObjectSetAttribUtil& util, void* offset, void* fieldValue) -> void {
      GameObjectSound* obj = static_cast<GameObjectSound*>(offset);
      std::string* clip = static_cast<std::string*>(fieldValue);
      if (fieldValue != NULL){
        unloadSoundState(obj -> source, obj -> clip); 
        obj -> clip = *clip;
        obj -> source = loadSoundState(util.pathForModLayer(obj -> clip), obj -> loop);     
      }
    },
    .getAttribute = [](void* offset) -> AttributeValue {
      GameObjectSound* obj = static_cast<GameObjectSound*>(offset);
      return obj -> clip;
    },
  },
};

GameObjectSound createSound(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectSound obj {};
  createAutoSerialize((char*)&obj, soundAutoserializer, attr, util);
  return obj;
}

void soundObjAttr(GameObjectSound& soundObj, GameobjAttributes& _attributes){
   autoserializerGetAttr((char*)&soundObj, soundAutoserializer, _attributes);
}

std::vector<std::pair<std::string, std::string>> serializeSound(GameObjectSound& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&obj, soundAutoserializer, pairs);
  return pairs;
}  

void removeSound(GameObjectSound& soundObj, ObjectRemoveUtil& util){
  unloadSoundState(soundObj.source, soundObj.clip); 
}

void setSoundAttributes(GameObjectSound& soundObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  autoserializerSetAttr((char*)&soundObj, soundAutoserializer, attributes, util);
}