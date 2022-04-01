#ifndef MOD_OBJ_CUSTOM
#define MOD_OBJ_CUSTOM

#include "../../common/util.h"
#include "./obj_util.h"

struct GameObjectCustom {
};

GameObjectCustom createCustom(GameobjAttributes& attr, ObjectTypeUtil& util);
void removeCustom(GameObjectCustom& customObj, ObjectRemoveUtil& util);

#endif