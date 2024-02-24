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

struct AtlasDimensions {
  int numTexturesWide;
  int numTexturesHeight;
  int totalTextures;
};
void setAtlasDimensions(AtlasDimensions newAtlasDimensions);

void drawOctreeSelectionGrid(std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, glm::mat4 modelMatrix);
void handleOctreeRaycast(glm::vec3 fromPos, glm::vec3 toPosDirection, bool secondarySelection, objid id);

enum OctreeDimAxis { OCTREE_XAXIS, OCTREE_YAXIS, OCTREE_ZAXIS, OCTREE_NOAXIS };
void handleOctreeScroll(GameObjectOctree& octree, bool upDirection, std::function<Mesh(MeshData&)> loadMesh, bool holdingShift, bool holdingCtrl, OctreeDimAxis axis);

enum OctreeEditorMove { X_NEG, X_POS, Y_NEG, Y_POS, Z_NEG, Z_POS };
void handleMoveOctreeSelection(OctreeEditorMove direction);

int getCurrentSubdivisionLevel();
void handleChangeSubdivisionLevel(int subdivisionLevel);
void increaseSelectionSize(int width, int height, int depth);

void insertSelectedOctreeNodes(GameObjectOctree& octree, std::function<Mesh(MeshData&)> loadMesh);
void deleteSelectedOctreeNodes(GameObjectOctree& octree, std::function<Mesh(MeshData&)> loadMesh);

enum TextureOrientation { TEXTURE_UP, TEXTURE_RIGHT, TEXTURE_DOWN, TEXTURE_LEFT };
void writeOctreeTexture(GameObjectOctree& octree, std::function<Mesh(MeshData&)> loadMesh, bool unitTexture, TextureOrientation texOrientation = TEXTURE_UP);
int getOctreeTextureId();
void setOctreeTextureId(int textureId);

enum RampDirection { RAMP_RIGHT, RAMP_LEFT, RAMP_FORWARD, RAMP_BACKWARD };
void makeOctreeCellRamp(GameObjectOctree& octree, std::function<Mesh(MeshData&)> loadMesh, RampDirection direction);


void loadOctree(GameObjectOctree& octree, std::function<Mesh(MeshData&)> loadMesh);
void saveOctree();
void optimizeOctree(GameObjectOctree& octree, std::function<Mesh(MeshData&)> loadMesh);

struct PhysicsShapes {
  std::vector<PositionAndScale> blocks;
  std::vector<PositionAndScaleVerts> shapes;
};

PhysicsShapes getPhysicsShapes();
std::optional<objid> getSelectedOctreeId();

#endif