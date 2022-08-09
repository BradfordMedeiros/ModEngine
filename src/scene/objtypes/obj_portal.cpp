#include "./obj_portal.h"

std::vector<AutoSerialize> autoserializerData {
  AutoSerializeBool {
    .structOffset = offsetof(GameObjectPortal, perspective),
    .field = "perspective", 
    .onString = "true",
    .offString = "false",
    .defaultValue = false,
  }
};


GameObjectPortal createPortal(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectPortal obj {};
  createAutoSerialize((char*)&obj, autoserializerData, attr);
  attrSet(attr, &obj.camera, "", "camera");
  return obj;
}