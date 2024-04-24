#include "./obj_sound.h"

std::vector<AutoSerialize> soundAutoserializer {
  AutoSerializeBool {
    .structOffset = offsetof(GameObjectSound, loop),
    .field = "loop",
    .onString = "true",
    .offString = "false",
    .defaultValue = false,
  },
  AutoSerializeBool {
    .structOffset = offsetof(GameObjectSound, center),
    .field = "center",
    .onString = "true",
    .offString = "false",
    .defaultValue = false,
  },
  AutoSerializeCustom {
    .structOffset = 0,
    .field = "clip",
    .fieldType = ATTRIBUTE_STRING,
    .deserialize = [](void* offset, void* fieldValue) -> void {
      GameObjectSound* obj = static_cast<GameObjectSound*>(offset);
      std::string* clip = static_cast<std::string*>(fieldValue);
      if (fieldValue == NULL){
        modassert(false, "clip must not be unspecified");
      }else{
        obj -> clip = *clip;
        obj -> source = loadSoundState(obj -> clip);     
      }
    },
    .setAttributes = [](void* offset, void* fieldValue) -> void {
      GameObjectSound* obj = static_cast<GameObjectSound*>(offset);
      std::string* clip = static_cast<std::string*>(fieldValue);
      if (fieldValue != NULL){
        unloadSoundState(obj -> source, obj -> clip); 
        obj -> clip = *clip;
        obj -> source = loadSoundState(obj -> clip);     
      }
    },
    .getAttribute = [](void* offset) -> AttributeValue {
      GameObjectSound* obj = static_cast<GameObjectSound*>(offset);
      return obj -> clip;
    },
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectSound, volume),
    .structOffsetFiller = std::nullopt,
    .field = "volume",
    .defaultValue = 1.f,
  },
};

GameObjectSound createSound(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectSound obj {};
  createAutoSerializeWithTextureLoading((char*)&obj, soundAutoserializer, attr, util);
  setSoundVolume(obj.source, obj.volume);
  setSoundLooping(obj.source, obj.loop);
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

std::optional<AttributeValuePtr> getSoundAttribute(GameObjectSound& soundObj, const char* field){
  return getAttributePtr((char*)&soundObj, soundAutoserializer, field);
}

bool setSoundAttribute(GameObjectSound& soundObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util){
  auto updated = autoserializerSetAttrWithTextureLoading((char*)&soundObj, soundAutoserializer, field, value, util);
  setSoundVolume(soundObj.source, soundObj.volume);
  setSoundLooping(soundObj.source, soundObj.loop);
  return updated;
}