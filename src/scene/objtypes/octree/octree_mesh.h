#ifndef MOD_OBJ_OCTREE_MESH
#define MOD_OBJ_OCTREE_MESH

#include <vector>
#include "./octree_types.h"
#include "../../../common/util.h"
#include "../obj_util.h"

glm::vec2 calcNdiCoordAtlasCoord(glm::vec2 ndiCoord, int imageIndex);
glm::vec2 meshTexBottomRight(FaceTexture& faceTexture);
glm::vec2 meshTexBottomLeft(FaceTexture& faceTexture);
glm::vec2 meshTexTopLeft(FaceTexture& faceTexture);
glm::vec2 meshTexTopRight(FaceTexture& faceTexture);

struct OctreeVertex {
  glm::vec3 position;
  glm::vec2 coord;
};

void addCubePointsFront(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f);
void addCubePointsBack(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f, float depth = 1.f);
void addCubePointsLeft(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f);
void addCubePointsRight(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f);
void addCubePointsTop(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f);
void addCubePointsBottom(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float depth = 1.f, float width = 1.f);

OctreeDivision* getOctreeSubdivisionIfExists2(Octree& octree, int x, int y, int z, int subdivision);

struct FillStatus {
  FillType fill;
  std::optional<OctreeDivision*> mixed;
};
FillStatus octreeFillStatus(Octree& octree, int subdivisionLevel, glm::ivec3 division);
bool shouldShowCubeSide(FillStatus fillStatus, OctreeSelectionFace side /*  { FRONT, BACK, LEFT, RIGHT, UP, DOWN }*/);

#endif


