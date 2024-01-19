#ifndef MOD_OBJ_OCTREE
#define MOD_OBJ_OCTREE

#include "../../common/util.h"
#include "./obj_util.h"
#include "../common/vectorgfx.h"

struct GameObjectOctree {
	Mesh mesh;
};

GameObjectOctree createOctree(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeOctree(GameObjectOctree& obj, ObjectSerializeUtil& util);
Mesh* getOctreeMesh(GameObjectOctree& octree);

void drawOctreeSelectionGrid(std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine);
void handleOctreeRaycast(glm::vec3 fromPos, glm::vec3 toPosDirection);
void handleOctreeScroll(bool upDirection);

#endif