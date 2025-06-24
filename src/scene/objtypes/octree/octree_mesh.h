#ifndef MOD_OBJ_OCTREE_MESH
#define MOD_OBJ_OCTREE_MESH

#include <vector>
#include "./octree_types.h"
#include "../../../common/util.h"
#include "../obj_util.h"
#include "../../../resources.h"
#include "./octree_vector.h"

struct OctreeMeshes {
  Mesh mesh;
  std::optional<Mesh> waterMesh;
};
OctreeMeshes createOctreeMesh(Octree& octree, std::function<Mesh(MeshData&)> loadMesh);

void visualizeTags(Octree& octree, int tag, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine2);

#endif


