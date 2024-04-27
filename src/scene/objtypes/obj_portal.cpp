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

std::vector<std::pair<std::string, std::string>> serializePortal(GameObjectPortal& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&obj, portalAutoserializer, pairs);
  return pairs;
}

bool setPortalAttribute(GameObjectPortal& portalObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&){
  return autoserializerSetAttrWithTextureLoading((char*)&portalObj, portalAutoserializer, field, value, util);
}

std::optional<AttributeValuePtr> getPortalAttribute(GameObjectPortal& obj, const char* field){
  return getAttributePtr((char*)&obj, portalAutoserializer, field);
}