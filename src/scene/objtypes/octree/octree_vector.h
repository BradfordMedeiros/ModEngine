#ifndef MOD_OBJ_OCTREE_VECTOR
#define MOD_OBJ_OCTREE_VECTOR

#include "../../../common/util.h"
#include "./octree_types.h"
#include "../../common/vectorgfx.h"

void drawOctreeSelectionGrid(Octree& octree, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine2, glm::mat4 modelMatrix);
void drawGridSelectionCube(int x, int y, int z, int numCellsWidth, int numCellsHeight, int numCellDepth, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, std::optional<OctreeSelectionFace> face);

#endif
