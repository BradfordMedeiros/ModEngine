#ifndef MOD_OBJ_OCTREE
#define MOD_OBJ_OCTREE

#include "../../common/util.h"
#include "./obj_util.h"
#include "../common/vectorgfx.h"
#include "../../resources.h"

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

struct GameObjectOctree {
  std::string map;
  Mesh mesh;
  Octree octree;
};

GameObjectOctree createOctree(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeOctree(GameObjectOctree& obj, ObjectSerializeUtil& util);
Mesh* getOctreeMesh(GameObjectOctree& octree);

struct AtlasDimensions {
  std::vector<std::string> textureNames;
};
void setAtlasDimensions(AtlasDimensions newAtlasDimensions);

void drawOctreeSelectionGrid(Octree& octree, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, glm::mat4 modelMatrix);
void handleOctreeRaycast(Octree& octree, glm::vec3 fromPos, glm::vec3 toPosDirection, bool secondarySelection, objid id);

enum OctreeDimAxis { OCTREE_XAXIS, OCTREE_YAXIS, OCTREE_ZAXIS, OCTREE_NOAXIS };
void handleOctreeScroll(GameObjectOctree& gameobjOctree, Octree& octree, bool upDirection, std::function<Mesh(MeshData&)> loadMesh, bool holdingShift, bool holdingCtrl, OctreeDimAxis axis);

enum OctreeEditorMove { X_NEG, X_POS, Y_NEG, Y_POS, Z_NEG, Z_POS };
void handleMoveOctreeSelection(OctreeEditorMove direction);

int getCurrentSubdivisionLevel();
void handleChangeSubdivisionLevel(int subdivisionLevel);
void increaseSelectionSize(int width, int height, int depth);

void insertSelectedOctreeNodes(GameObjectOctree& gameobjOctree, Octree& octree, std::function<Mesh(MeshData&)> loadMesh);
void deleteSelectedOctreeNodes(GameObjectOctree& gameobjOctree, Octree& octree, std::function<Mesh(MeshData&)> loadMesh);

enum TextureOrientation { TEXTURE_UP, TEXTURE_RIGHT, TEXTURE_DOWN, TEXTURE_LEFT };
void writeOctreeTexture(GameObjectOctree& gameobjOctree, Octree& octree, std::function<Mesh(MeshData&)> loadMesh, bool unitTexture, TextureOrientation texOrientation = TEXTURE_UP);
int getOctreeTextureId();
void setOctreeTextureId(int textureId);

void makeOctreeCellRamp(GameObjectOctree& gameobjOctree, Octree& octree, std::function<Mesh(MeshData&)> loadMesh, RampDirection direction);

void loadOctree(GameObjectOctree& octree, std::function<std::string(std::string)> loadFile, std::function<Mesh(MeshData&)> loadMesh);
void saveOctree(GameObjectOctree& octree, std::function<void(std::string, std::string&)> saveFile);
void optimizeOctree(GameObjectOctree& octree, std::function<Mesh(MeshData&)> loadMesh);

struct PhysicsShapes {
  std::vector<PositionAndScale> blocks;
  std::vector<PositionAndScaleVerts> shapes;
};

PhysicsShapes getPhysicsShapes(Octree& octree);
std::string debugInfo(PhysicsShapes& physicsShapes);
void setSelectedOctreeId(std::optional<objid> id);
std::optional<objid> getSelectedOctreeId();
Octree deserializeOctree(std::string& value);

std::optional<AttributeValuePtr> getOctreeAttribute(GameObjectOctree& obj, const char* field);

#endif