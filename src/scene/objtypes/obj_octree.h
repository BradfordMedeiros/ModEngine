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

void handleOctreeScroll(GameObjectOctree& octree, bool upDirection, std::function<Mesh(MeshData&)> loadMesh, bool holdingShift, bool holdingCtrl);

enum OctreeEditorMove { X_NEG, X_POS, Y_NEG, Y_POS, Z_NEG, Z_POS };
void handleMoveOctreeSelection(OctreeEditorMove direction);

int getCurrentSubdivisionLevel();
void handleChangeSubdivisionLevel(int subdivisionLevel);
void increaseSelectionSize();
void decreaseSelectionSize();

void deleteSelectedOctreeNodes(GameObjectOctree& octree, std::function<Mesh(MeshData&)> loadMesh);

#endif