#ifndef MOD_OBJ_OCTREE_SERIALIZATION
#define MOD_OBJ_OCTREE_SERIALIZATION

#include <vector>
#include "./octree_types.h"
#include "../../../common/util.h"
#include "../obj_util.h"


std::string serializeOctree(Octree& octree);
Octree deserializeOctree(std::string& value);

#endif