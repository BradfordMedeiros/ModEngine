#ifndef MOD_OBJ_PORTAL
#define MOD_OBJ_PORTAL

#include "../../common/util.h"

struct GameObjectPortal {
  std::string camera;
  bool perspective;
};

GameObjectPortal createPortal(GameobjAttributes& attr);

#endif