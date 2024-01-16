#ifndef MOD_OBJ_OCTREE
#define MOD_OBJ_OCTREE

#include "../../common/util.h"
#include "./obj_util.h"

struct GameObjectOctree {
	Mesh mesh;
};

GameObjectOctree createOctree(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeOctree(GameObjectOctree& obj, ObjectSerializeUtil& util);
Mesh* getOctreeMesh(GameObjectOctree& octree);

#endif