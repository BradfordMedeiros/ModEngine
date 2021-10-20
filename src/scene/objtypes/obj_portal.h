#ifndef MOD_OBJ_PORTAL
#define MOD_OBJ_PORTAL

#include "../../common/util.h"
#include "./obj_util.h"

struct GameObjectPortal {
  std::string camera;
  bool perspective;
};

GameObjectPortal createPortal(GameobjAttributes& attr, ObjectTypeUtil& util);

#endif