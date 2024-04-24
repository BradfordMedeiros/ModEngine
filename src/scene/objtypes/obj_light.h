#ifndef MOD_OBJ_LIGHT
#define MOD_OBJ_LIGHT

#include "../../common/util.h"
#include "./obj_util.h"

enum LightType { LIGHT_POINT, LIGHT_SPOTLIGHT, LIGHT_DIRECTIONAL };
struct GameObjectLight {
  glm::vec3 color;
  LightType type;
  float maxangle;
  float angledelta;
  glm::vec3 attenuation;
};

GameObjectLight createLight(GameobjAttributes& attr, ObjectTypeUtil& util);
std::optional<AttributeValuePtr> getLightAttribute(GameObjectLight& obj, const char* field);
void lightObjAttr(GameObjectLight& lightObj, GameobjAttributes& _attributes);

std::vector<std::pair<std::string, std::string>> serializeLight(GameObjectLight& obj, ObjectSerializeUtil& util);
bool setLightAttribute(GameObjectLight& lightObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util);

#endif