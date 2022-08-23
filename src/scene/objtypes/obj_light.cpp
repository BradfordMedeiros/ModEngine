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
  AutoSerializeCustom {
    .structOffset = 0,
    .field = "angle",
    .fieldType = ATTRIBUTE_FLOAT,
    .deserialize = [](void* offset, void* fieldValue) -> void {
      GameObjectLight* light = static_cast<GameObjectLight*>(offset);
      float* field = static_cast<float*>(fieldValue);
      if (light != NULL && light -> type == LIGHT_SPOTLIGHT && field != NULL){
        light -> maxangle = *field;
      }else{
        light -> maxangle = -10.f;
      }
    },
    .setAttributes = [](void* offset, void* fieldValue) -> void {
      GameObjectLight* light = static_cast<GameObjectLight*>(offset);
      float* field = static_cast<float*>(fieldValue);
      if (light != NULL && light -> type == LIGHT_SPOTLIGHT && field != NULL){
        light -> maxangle = *field;
      }else {
        light -> maxangle = -10.f;
      }
    },
    .getAttribute = [](void* offset) -> AttributeValue {
      GameObjectLight* light = static_cast<GameObjectLight*>(offset);
      return light -> maxangle;
    },
  }

};

GameObjectLight createLight(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectLight obj {};
  createAutoSerializeWithTextureLoading((char*)&obj, lightAutoserializer, attr, util);
  return obj;
}

void lightObjAttr(GameObjectLight& lightObj, GameobjAttributes& _attributes){
  autoserializerGetAttr((char*)&lightObj, lightAutoserializer, _attributes);
}

std::vector<std::pair<std::string, std::string>> serializeLight(GameObjectLight& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&obj, lightAutoserializer, pairs);
  return pairs;
}

bool setLightAttributes(GameObjectLight& lightObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  autoserializerSetAttrWithTextureLoading((char*)&lightObj, lightAutoserializer, attributes, util);
  return false;
}