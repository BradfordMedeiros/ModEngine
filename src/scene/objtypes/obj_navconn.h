#ifndef MOD_OBJ_NAVCONN
#define MOD_OBJ_NAVCONN

#include "../../common/util.h"
#include "../types/ainav.h"
#include "./obj_util.h"

struct GameObjectNavConns {
  NavGraph navgraph;
};

GameObjectNavConns createNavConns(GameobjAttributes& attr, ObjectTypeUtil& util);

#endif