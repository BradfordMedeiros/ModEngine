#ifndef MOD_OBJ_OCTREE_MESH
#define MOD_OBJ_OCTREE_MESH

#include <vector>
#include "./octree_types.h"
#include "../../../common/util.h"
#include "../obj_util.h"

Mesh createOctreeMesh(Octree& octree, std::function<Mesh(MeshData&)> loadMesh);

#endif


