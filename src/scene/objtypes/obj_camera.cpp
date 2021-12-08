#include "./obj_camera.h"

GameObjectCamera createCamera(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto enableDof = attr.stringAttributes.find("dof") != attr.stringAttributes.end() ? attr.stringAttributes.at("dof") == "enabled" : false;
  auto minBlurDistance = attr.numAttributes.find("minblur") != attr.numAttributes.end() ? attr.numAttributes.at("minblur") : 0.05f;
  auto maxBlurDistance = attr.numAttributes.find("maxblur") != attr.numAttributes.end() ? attr.numAttributes.at("maxblur") : 0.1f;
  GameObjectCamera obj {
    .enableDof = enableDof,
    .minBlurDistance = minBlurDistance,
    .maxBlurDistance = maxBlurDistance,
  };
  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeCamera(GameObjectCamera obj, ObjectSerializeUtil& util){
  assert(false);
}
void cameraObjAttr(GameObjectCamera& meshObj, GameobjAttributes& _attributes){
  assert(false);
}
void setCameraAttributes(GameObjectCamera& meshObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  assert(false);
}