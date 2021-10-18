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
GameObjectLight createLight(GameobjAttributes& attr){
  auto color = attr.vecAttributes.find("color") == attr.vecAttributes.end() ? glm::vec3(1.f, 1.f, 1.f) : attr.vecAttributes.at("color");
  auto lightType = attr.stringAttributes.find("type") == attr.stringAttributes.end() ? LIGHT_POINT : getLightType(attr.stringAttributes.at("type"));
  auto maxangle = (lightType != LIGHT_SPOTLIGHT || attr.numAttributes.find("angle") == attr.numAttributes.end()) ? -10.f : attr.numAttributes.at("angle");

  // constant, linear, quadratic
  // in shader =>  float attenuation = 1.0 / (constant + (linear * distanceToLight) + (quadratic * (distanceToLight * distanceToLight)));  
  auto attenuation = attr.vecAttributes.find("attenuation") == attr.vecAttributes.end() ? glm::vec3(1.0, 0.007, 0.0002) : attr.vecAttributes.at("attenuation");

  GameObjectLight obj {
    .color = color,
    .type = lightType,
    .maxangle = maxangle, 
    .attenuation = attenuation,
  };
  return obj;
}