#ifndef MOD_OBJ_OCTREE_RAYCAST
#define MOD_OBJ_OCTREE_RAYCAST

#include <vector>
#include "./octree_types.h"
#include "../../../common/util.h"
#include "../obj_util.h"

void handleOctreeRaycast(Octree& octree, glm::vec3 fromPos, glm::vec3 toPosDirection, bool secondarySelection, objid id);

#endif