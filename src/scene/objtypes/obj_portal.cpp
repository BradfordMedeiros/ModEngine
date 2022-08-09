#include "./obj_portal.h"

std::vector<AutoSerialize> portalAutoserializer {
  AutoSerializeBool {
    .structOffset = offsetof(GameObjectPortal, perspective),
    .field = "perspective", 
    .onString = "true",
    .offString = "false",
    .defaultValue = false,
  },
  AutoSerializeString {
    .structOffset = offsetof(GameObjectPortal, camera),
    .field = "camera",
    .defaultValue = "",
  }
};


GameObjectPortal createPortal(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectPortal obj {};
  createAutoSerialize((char*)&obj, portalAutoserializer, attr);
  return obj;
}