#ifndef MOD_OBJ_OCTREE_VECTOR
#define MOD_OBJ_OCTREE_VECTOR

#include "../../../common/util.h"
#include "./octree_types.h"

void drawPhysicsBlock(PositionAndScale& physicShape, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine);
void drawPhysicsShape(std::vector<glm::vec3>& verts, glm::vec3& centeringOffset, Transformation& transform, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine);

struct PhysicsShapes {
  std::vector<PositionAndScale> blocks;
  std::vector<PositionAndScaleVerts> shapes;
};
void drawPhysicsShapes(PhysicsShapes& physicsShapes, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine);

void drawGridSelectionXY(int x, int y, int z, int numCellsWidth, int numCellsHeight, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, std::optional<OctreeSelectionFace> face);
void drawGridSelectionYZ(int x, int y, int z, int numCellsHeight, int numCellsDepth, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, std::optional<OctreeSelectionFace> face);
void drawGridSelectionCube(int x, int y, int z, int numCellsWidth, int numCellsHeight, int numCellDepth, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, std::optional<OctreeSelectionFace> face);

void visualizeFaces(Faces& faces, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine);

#endif
