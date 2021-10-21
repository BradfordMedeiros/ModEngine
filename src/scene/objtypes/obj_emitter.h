#ifndef MOD_OBJ_EMITTER
#define MOD_OBJ_EMITTER

#include "../../common/util.h"
#include "./obj_util.h"
#include "../types/emitter.h"

struct GameObjectEmitter{};

GameObjectEmitter createEmitter(GameobjAttributes& attributes, ObjectTypeUtil& util);

#endif