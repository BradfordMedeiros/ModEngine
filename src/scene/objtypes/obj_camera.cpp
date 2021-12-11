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
  if (attr.stringAttributes.find("target") != attr.stringAttributes.end()){
    cameraObj.target = attr.stringAttributes.at("target");
  }
}

GameObjectCamera createCamera(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectCamera obj {
    .enableDof = false,
    .minBlurDistance = 0.05f,
    .maxBlurDistance = 0.1f,
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
}

void setCameraAttributes(GameObjectCamera& cameraObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  setCameraAttr(cameraObj, attributes);
}