#ifndef MOD_OBJ_CAMERA
#define MOD_OBJ_CAMERA

#include "../../common/util.h"
#include "./obj_util.h"

struct GameObjectCamera {};

GameObjectCamera createCamera(GameobjAttributes& attr, ObjectTypeUtil& util);

#endif