#ifndef MOD_OBJ_NAVMESH
#define MOD_OBJ_NAVMESH

#include "../../common/util.h"
#include "./obj_util.h"
#include "../common/util/meshgen.h"

struct GameObjectNavmesh {
  Mesh mesh;
};

GameObjectNavmesh createNavmesh(GameobjAttributes& attr, ObjectTypeUtil& util);

#endif