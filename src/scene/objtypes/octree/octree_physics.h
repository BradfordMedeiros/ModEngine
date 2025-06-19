#ifndef MOD_OBJ_OCTREE_PHYSICS
#define MOD_OBJ_OCTREE_PHYSICS

#include <vector>
#include "./octree_types.h"

struct PhysicsShapeData {
  OctreeShape* shape;
  std::vector<int> path;
};

struct SparseShape {
  OctreeShape* shape;
  std::vector<int> path;
  bool deleted;
  int minX;
  int maxX;
  int minY;
  int maxY;
  int minZ;
  int maxZ;
};

std::string print(SparseShape& sparseShape);


struct FinalShapeData {
  OctreeShape* shape;
  glm::vec3 position;
  glm::vec3 shapeSize;
};
std::vector<FinalShapeData> optimizePhysicsShapeData(std::vector<PhysicsShapeData>& shapeData);


#endif


