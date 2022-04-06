#ifndef MOD_OBJ_NIL
#define MOD_OBJ_NIL

#include "../../common/util.h"
#include "./obj_util.h"

struct GameObjectNil {};
GameObjectNil createNil(GameobjAttributes& attr, ObjectTypeUtil& util);

#endif