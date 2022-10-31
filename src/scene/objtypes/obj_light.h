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
void lightObjAttr(GameObjectLight& lightObj, GameobjAttributes& _attributes);
std::vector<std::pair<std::string, std::string>> serializeLight(GameObjectLight& obj, ObjectSerializeUtil& util);
bool setLightAttributes(GameObjectLight& lightObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);

#endif