#ifndef MOD_OBJ_LIGHT
#define MOD_OBJ_LIGHT

#include "../../../common/util.h"
#include "../obj_util.h"
#include "./scene_lighting.h"

enum LightType { LIGHT_POINT, LIGHT_SPOTLIGHT, LIGHT_DIRECTIONAL };
struct GameObjectLight {
  glm::vec3 color;
  LightType type;
  float maxangle;
  float angledelta;
  glm::vec3 attenuation;
};

GameObjectLight createLight(GameobjAttributes& attr, ObjectTypeUtil& util);
void removeLight(GameObjectLight& lightObj, ObjectRemoveUtil& util);
std::optional<AttributeValuePtr> getLightAttribute(GameObjectLight& obj, const char* field);
std::vector<std::pair<std::string, std::string>> serializeLight(GameObjectLight& obj, ObjectSerializeUtil& util);
bool setLightAttribute(GameObjectLight& lightObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&);

#endif