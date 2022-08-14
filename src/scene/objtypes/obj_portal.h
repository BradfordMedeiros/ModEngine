#ifndef MOD_OBJ_PORTAL
#define MOD_OBJ_PORTAL

#include "../../common/util.h"
#include "./obj_util.h"

struct GameObjectPortal {
  std::string camera;
  bool perspective;
};

GameObjectPortal createPortal(GameobjAttributes& attr, ObjectTypeUtil& util);
void portalObjAttr(GameObjectPortal& soundObj, GameobjAttributes& _attributes);
std::vector<std::pair<std::string, std::string>> serializePortal(GameObjectPortal& obj, ObjectSerializeUtil& util);
void setPortalAttributes(GameObjectPortal& soundObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);

#endif