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
void cameraObjAttr(GameObjectCamera& cameraObj, GameobjAttributes& _attributes){
  autoserializerGetAttr((char*)&cameraObj, cameraAutoserializer, _attributes);
}

std::optional<AttributeValuePtr> getCameraAttribute(GameObjectCamera& obj, const char* field){
  return getAttributePtr((char*)&obj, cameraAutoserializer, field);
}

bool setCameraAttributes(GameObjectCamera& cameraObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  autoserializerSetAttrWithTextureLoading((char*)&cameraObj, cameraAutoserializer, attributes, util);
  return false;
}