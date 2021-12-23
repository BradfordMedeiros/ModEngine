#include "./obj_light.h"

LightType getLightType(std::string type){
  if (type == "spotlight"){
    return LIGHT_SPOTLIGHT;
  }
  if (type == "directional"){
    return LIGHT_DIRECTIONAL;
  }
  return LIGHT_POINT;
}
GameObjectLight createLight(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto color = attr.vecAttributes.find("color") == attr.vecAttributes.end() ? glm::vec3(1.f, 1.f, 1.f) : attr.vecAttributes.at("color");
  auto lightType = attr.stringAttributes.find("type") == attr.stringAttributes.end() ? LIGHT_POINT : getLightType(attr.stringAttributes.at("type"));
  auto maxangle = (lightType != LIGHT_SPOTLIGHT || attr.numAttributes.find("angle") == attr.numAttributes.end()) ? -10.f : attr.numAttributes.at("angle");

  // constant, linear, quadratic
  // in shader =>  float attenuation = 1.0 / (constant + (linear * distanceToLight) + (quadratic * (distanceToLight * distanceToLight)));  
  // physically accurate ish would be to attenuate based on 1 / r^2  hence the 0 0 1 default
  auto attenuation = attr.vecAttributes.find("attenuation") == attr.vecAttributes.end() ? glm::vec3(0, 0, 1) : attr.vecAttributes.at("attenuation");

  GameObjectLight obj {
    .color = color,
    .type = lightType,
    .maxangle = maxangle, 
    .attenuation = attenuation,
  };
  return obj;
}

void lightObjAttr(GameObjectLight& lightObj, GameobjAttributes& _attributes){
  _attributes.vecAttributes["color"] = lightObj.color;
}

std::vector<std::pair<std::string, std::string>> serializeLight(GameObjectLight& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  pairs.push_back(std::pair<std::string, std::string>("color", serializeVec(obj.color)));
  return pairs;
}

void setLightAttributes(GameObjectLight& lightObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  lightObj.color = attributes.vecAttributes.at("color");
}