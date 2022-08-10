#include "./obj_light.h"

std::vector<AutoSerialize> lightAutoserializer {
 
};

GameObjectLight createLight(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectLight obj {};
  createAutoSerialize((char*)&obj, lightAutoserializer, attr, util);

  attrSet(attr, (int*)&obj.type, { LIGHT_POINT, LIGHT_SPOTLIGHT, LIGHT_DIRECTIONAL }, { "point", "spotlight", "directional" }, LIGHT_POINT, "type", true);

  auto maxangle = (obj.type != LIGHT_SPOTLIGHT || attr.numAttributes.find("angle") == attr.numAttributes.end()) ? -10.f : attr.numAttributes.at("angle");
  obj.maxangle = maxangle;

  attrSet(attr, &obj.color, glm::vec3(1.f, 1.f, 1.f), "color");
  // constant, linear, quadratic
  // in shader =>  float attenuation = 1.0 / (constant + (linear * distanceToLight) + (quadratic * (distanceToLight * distanceToLight)));  
  // physically accurate ish would be to attenuate based on 1 / r^2  hence the 0 0 1 default
  attrSet(attr, &obj.attenuation, glm::vec3(0, 0, 1), "attenuation");
  createAutoSerialize((char*)&obj, lightAutoserializer, attr, util);
  return obj;
}

void lightObjAttr(GameObjectLight& lightObj, GameobjAttributes& _attributes){
  _attributes.vecAttr.vec3["color"] = lightObj.color;
}

std::vector<std::pair<std::string, std::string>> serializeLight(GameObjectLight& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  pairs.push_back(std::pair<std::string, std::string>("color", serializeVec(obj.color)));
  return pairs;
}

void setLightAttributes(GameObjectLight& lightObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  maybeSetVec3FromAttr(&lightObj.color, "color", attributes);
}