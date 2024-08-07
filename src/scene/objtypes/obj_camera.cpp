#include "./obj_camera.h"

std::vector<AutoSerialize> cameraAutoserializer {
  AutoSerializeBool {
    .structOffset = offsetof(GameObjectCamera, enableDof),
    .field = "dof",
    .onString = "enabled",
    .offString = "disabled",
    .defaultValue = false,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectCamera, minBlurDistance),
    .structOffsetFiller = std::nullopt,
    .field = "minblur",
    .defaultValue = 0.f,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectCamera, maxBlurDistance),
    .structOffsetFiller = std::nullopt,
    .field = "maxblur",
    .defaultValue = 10.f,
  },
  AutoSerializeString {
    .structOffset = offsetof(GameObjectCamera, target),
    .field = "target",
    .defaultValue = "",
  },
  AutoSerializeUInt {
    .structOffset = offsetof(GameObjectCamera, blurAmount),
    .field = "bluramount",
    .defaultValue = 1,
  }, 
};

GameObjectCamera createCamera(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectCamera obj {};
  createAutoSerializeWithTextureLoading((char*)&obj, cameraAutoserializer, attr, util);
  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeCamera(GameObjectCamera obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&obj, cameraAutoserializer, pairs);
  return pairs;
}

std::optional<AttributeValuePtr> getCameraAttribute(GameObjectCamera& obj, const char* field){
  return getAttributePtr((char*)&obj, cameraAutoserializer, field);
}

bool setCameraAttribute(GameObjectCamera& cameraObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&){
  return autoserializerSetAttrWithTextureLoading((char*)&cameraObj, cameraAutoserializer, field, value, util);
}