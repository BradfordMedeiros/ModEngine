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
  OctreeShape shape;
  std::vector<FaceTexture> faces;
  std::vector<OctreeDivision> divisions;
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


#endif
