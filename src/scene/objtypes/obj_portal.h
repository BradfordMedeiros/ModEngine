#ifndef MOD_OBJ_PORTAL
#define MOD_OBJ_PORTAL

#include "../../common/util.h"
#include "./obj_util.h"

struct GameObjectPortal {
  std::string camera;
  bool perspective;
};

GameObjectPortal createPortal(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializePortal(GameObjectPortal& obj, ObjectSerializeUtil& util);
bool setPortalAttribute(GameObjectPortal& portalObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&);
std::optional<AttributeValuePtr> getPortalAttribute(GameObjectPortal& obj, const char* field);

#endif