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
  createAutoSerializeWithTextureLoading((char*)&obj, portalAutoserializer, attr, util);
  return obj;
}

void portalObjAttr(GameObjectPortal& portalObj, GameobjAttributes& _attributes){
  autoserializerGetAttr((char*)&portalObj, portalAutoserializer, _attributes);
}
std::vector<std::pair<std::string, std::string>> serializePortal(GameObjectPortal& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&obj, portalAutoserializer, pairs);
  return pairs;
}
bool setPortalAttributes(GameObjectPortal& portalObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  autoserializerSetAttrWithTextureLoading((char*)&portalObj, portalAutoserializer, attributes, util);
  return false;
}