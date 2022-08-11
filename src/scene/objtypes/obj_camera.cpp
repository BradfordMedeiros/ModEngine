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
  createAutoSerialize((char*)&obj, cameraAutoserializer, attr, util);
  std::cout << "camera: " << obj.enableDof << std::endl;
  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeCamera(GameObjectCamera obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  if (obj.enableDof){
    pairs.push_back(std::pair<std::string, std::string>("dof", "enabled"));
  }
  if (!aboutEqual(obj.minBlurDistance, 0.05f)){
    pairs.push_back(std::pair<std::string, std::string>("minblur", std::to_string(obj.minBlurDistance)));
  }
  if (!aboutEqual(obj.maxBlurDistance, 0.1f)){
    pairs.push_back(std::pair<std::string, std::string>("maxblur", std::to_string(obj.maxBlurDistance)));
  }
  if (obj.blurAmount != 1){
    pairs.push_back(std::pair<std::string, std::string>("bluramount", std::to_string(obj.blurAmount)));
  }
  if (obj.target != ""){
    pairs.push_back(std::pair<std::string, std::string>("target", obj.target));
  }
  autoserializerSerialize((char*)&obj, cameraAutoserializer, pairs);

  return pairs;
}
void cameraObjAttr(GameObjectCamera& cameraObj, GameobjAttributes& _attributes){
  autoserializerGetAttr((char*)&cameraObj, cameraAutoserializer, _attributes);
}

void setCameraAttributes(GameObjectCamera& cameraObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  attrSet(attributes, &cameraObj.minBlurDistance, "minblur");
  attrSet(attributes, &cameraObj.maxBlurDistance, "maxblur");
  attrSet(attributes, &cameraObj.blurAmount, "bluramount");
  autoserializerSetAttr((char*)&cameraObj, cameraAutoserializer, attributes);
}