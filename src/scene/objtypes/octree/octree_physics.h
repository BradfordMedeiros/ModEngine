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

int maxSubdivision(std::vector<PhysicsShapeData>& shapeData);
std::vector<SparseShape> joinSparseShapes(std::vector<SparseShape>& shapes);

std::string print(SparseShape& sparseShape);

struct ValueAndSubdivision {
  glm::ivec3 value;
  int subdivisionLevel;
};

ValueAndSubdivision indexForOctreePath(std::vector<int> path);


#endif


