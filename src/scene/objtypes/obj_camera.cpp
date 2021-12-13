#include "./obj_camera.h"

void setCameraAttr(GameObjectCamera& cameraObj, GameobjAttributes& attr){
  if (attr.stringAttributes.find("dof") != attr.stringAttributes.end()){
    cameraObj.enableDof = attr.stringAttributes.at("dof") == "enabled";
  }
  if (attr.numAttributes.find("minblur") != attr.numAttributes.end()){
    cameraObj.minBlurDistance = attr.numAttributes.at("minblur");
  }
  if (attr.numAttributes.find("maxblur") != attr.numAttributes.end()){
    cameraObj.maxBlurDistance = attr.numAttributes.at("maxblur");
  }
  if (attr.numAttributes.find("bluramount") != attr.numAttributes.end()){
    cameraObj.blurAmount = static_cast<unsigned int>(attr.numAttributes.at("bluramount"));
  }
  if (attr.stringAttributes.find("target") != attr.stringAttributes.end()){
    cameraObj.target = attr.stringAttributes.at("target");
  }
}

GameObjectCamera createCamera(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectCamera obj {
    .enableDof = false,
    .minBlurDistance = 0.f,
    .maxBlurDistance = 10.f,
    .blurAmount = 1,
    .target = "",
  };
  setCameraAttr(obj, attr);
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
  setCameraAttr(cameraObj, attributes);
}