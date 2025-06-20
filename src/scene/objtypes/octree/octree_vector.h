#ifndef MOD_OBJ_OCTREE_VECTOR
#define MOD_OBJ_OCTREE_VECTOR

#include "../../../common/util.h"
#include "./octree_types.h"

void drawPhysicsShapes(PhysicsShapes& physicsShapes, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine);
void drawGridSelectionXY(int x, int y, int z, int numCellsWidth, int numCellsHeight, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, std::optional<OctreeSelectionFace> face);
void drawGridSelectionCube(int x, int y, int z, int numCellsWidth, int numCellsHeight, int numCellDepth, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, std::optional<OctreeSelectionFace> face);
void visualizeFaces(Faces& faces, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine);

#endif
