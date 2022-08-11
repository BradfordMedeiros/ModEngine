#include "./obj_light.h"

std::vector<AutoSerialize> lightAutoserializer {
  AutoSerializeEnums {
    .structOffset = offsetof(GameObjectLight, type),
    .enums = { LIGHT_POINT, LIGHT_SPOTLIGHT, LIGHT_DIRECTIONAL },
    .enumStrings = { "point", "spotlight", "directional" },
    .field = "type",
    .defaultValue = LIGHT_POINT,
  },
  AutoSerializeVec3 {
    .structOffset = offsetof(GameObjectLight, color),
    .structOffsetFiller = std::nullopt,
    .field = "color",
    .defaultValue = glm::vec3(1.f, 1.f, 1.f),
  },
  // constant, linear, quadratic
  // in shader =>  float attenuation = 1.0 / (constant + (linear * distanceToLight) + (quadratic * (distanceToLight * distanceToLight)));  
  // physically accurate ish would be to attenuate based on 1 / r^2  hence the 0 0 1 default
  AutoSerializeVec3 {
    .structOffset = offsetof(GameObjectLight, attenuation),
    .structOffsetFiller = std::nullopt,
    .field = "attenuation",
    .defaultValue = glm::vec3(0, 0, 1),
  },
};

GameObjectLight createLight(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectLight obj {};
  createAutoSerialize((char*)&obj, lightAutoserializer, attr, util);
  auto maxangle = (obj.type != LIGHT_SPOTLIGHT || attr.numAttributes.find("angle") == attr.numAttributes.end()) ? -10.f : attr.numAttributes.at("angle");
  obj.maxangle = maxangle;
  return obj;
}

void lightObjAttr(GameObjectLight& lightObj, GameobjAttributes& _attributes){
  autoserializerGetAttr((char*)&lightObj, lightAutoserializer, _attributes);
}

std::vector<std::pair<std::string, std::string>> serializeLight(GameObjectLight& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  pairs.push_back(std::pair<std::string, std::string>("color", serializeVec(obj.color)));
  return pairs;
}

void setLightAttributes(GameObjectLight& lightObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  autoserializerSetAttr((char*)&lightObj, lightAutoserializer, attributes);
}