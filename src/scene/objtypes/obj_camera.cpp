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
    .field = "minblur",
    .defaultValue = 0.f,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectCamera, maxBlurDistance),
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
  return pairs;
}
void cameraObjAttr(GameObjectCamera& cameraObj, GameobjAttributes& _attributes){
  assert(false); 
  _attributes.stringAttributes["dof"] = cameraObj.enableDof ? "enabled" : "disabled";
  _attributes.numAttributes["minblur"] = cameraObj.minBlurDistance;
  _attributes.numAttributes["maxblur"] = cameraObj.maxBlurDistance;
  _attributes.stringAttributes["target"] = cameraObj.target;
  _attributes.numAttributes["bluramount"] = cameraObj.blurAmount;
}

void setCameraAttributes(GameObjectCamera& cameraObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  attrSet(attributes, &cameraObj.enableDof, "dof");
  attrSet(attributes, &cameraObj.minBlurDistance, "minblur");
  attrSet(attributes, &cameraObj.maxBlurDistance, "maxblur");
  attrSet(attributes, &cameraObj.blurAmount, "bluramount");
  attrSet(attributes, &cameraObj.target, "target");
}