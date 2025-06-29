#ifndef MOD_OBJ_OCTREE_TYPES
#define MOD_OBJ_OCTREE_TYPES

#include "../../../common/util.h"

enum OctreeSelectionFace { FRONT, BACK, LEFT, RIGHT, UP, DOWN };

struct FaceTexture {
  int textureIndex;
  glm::vec2 texCoordsTopLeft;
  glm::vec2 texCoordsTopRight;
  glm::vec2 texCoordsBottomLeft;
  glm::vec2 texCoordsBottomRight;
};

enum FillType { FILL_FULL, FILL_EMPTY, FILL_MIXED };

struct ShapeBlock { };

enum RampDirection { RAMP_RIGHT, RAMP_LEFT, RAMP_FORWARD, RAMP_BACKWARD };
struct ShapeRamp { 
  RampDirection direction;
  float startHeight;
  float endHeight;
  float startDepth;
  float endDepth;
};
typedef std::variant<ShapeBlock, ShapeRamp> OctreeShape;

struct OctreeDivision {
  // -x +y -z 
  // +x +y -z
  // -x +y +z
  // +x +y +z
  // -x -y -z 
  // +x -y -z
  // -x -y +z
  // +x -y +z
  FillType fill;
  OctreeMaterial material;
  OctreeShape shape;
  glm::vec3 color = glm::vec3(0.f, 0.f, 0.f);
  std::vector<FaceTexture> faces;
  std::vector<OctreeDivision> divisions;
  std::vector<TagInfo> tags;
};

struct Octree {
  OctreeDivision rootNode;
};

struct Faces {
  float XYClose;
  glm::vec3 XYClosePoint;
  float XYFar;
  glm::vec3 XYFarPoint;
  float YZLeft;
  glm::vec3 YZLeftPoint;
  float YZRight;
  glm::vec3 YZRightPoint;
  float XZTop;
  glm::vec3 XZTopPoint;
  float XZBottom;
  glm::vec3 XZBottomPoint;
  glm::vec3 center;
};

glm::ivec3 flatIndexToXYZ(int index);
int xyzIndexToFlatIndex(glm::ivec3 index);

glm::vec3 offsetForFlatIndex(int index, float subdivisionSize, glm::vec3 rootPos);


glm::ivec3 indexForSubdivision(int x, int y, int z, int sourceSubdivision, int targetSubdivision);
glm::ivec3 localIndexForSubdivision(int x, int y, int z, int sourceSubdivision, int targetSubdivision);
std::vector<glm::ivec3> octreePath(int x, int y, int z, int subdivision);

struct ValueAndSubdivision {
  glm::ivec3 value;
  int subdivisionLevel;
};

ValueAndSubdivision indexForOctreePath(std::vector<int> path);

struct PhysicsShapes {
  std::vector<PositionAndScale> blocks;
  std::vector<PositionAndScaleVerts> shapes;
};
struct AtlasDimensions {
  std::vector<std::string> textureNames;
};

struct FaceIntersection {
  OctreeSelectionFace face;
  glm::vec3 position;
};
struct RaycastIntersection {
  int index;
  glm::ivec3 blockOffset;
  std::vector<FaceIntersection> faceIntersections;
};
struct RaycastResult {
  glm::vec3 fromPos;
  glm::vec3 toPosDirection;
  int subdivisionDepth;
  std::vector<RaycastIntersection> intersections;
};
struct ClosestIntersection {
  OctreeSelectionFace face;
  glm::vec3 position;
  glm::ivec3 xyzIndex;
  int subdivisionDepth;
};

struct Intersection {
  int index;
  std::vector<FaceIntersection> faceIntersections;
};

std::string debugInfo(PhysicsShapes& physicsShapes);


#endif
