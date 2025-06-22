#ifndef MOD_OBJ_OCTREE_MESH
#define MOD_OBJ_OCTREE_MESH

#include <vector>
#include "./octree_types.h"
#include "../../../common/util.h"
#include "../obj_util.h"

struct OctreeMeshes {
  Mesh mesh;
  std::optional<Mesh> waterMesh;
};
OctreeMeshes createOctreeMesh(Octree& octree, std::function<Mesh(MeshData&)> loadMesh);

#endif


