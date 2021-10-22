#ifndef MOD_OBJ_VOXEL
#define MOD_OBJ_VOXEL

#include "../../common/util.h"
#include "./obj_util.h"
#include "../types/voxels.h"

struct GameObjectVoxel {
  Voxels voxel;
};

GameObjectVoxel createVoxel(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeVoxel(GameObjectVoxel& obj, ObjectSerializeUtil& util);

#endif