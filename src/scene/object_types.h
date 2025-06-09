#ifndef MOD_OBJECTTYPES
#define MOD_OBJECTTYPES

#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>
#include <variant>
#include "./common/mesh.h"
#include "./serialization.h"
#include "./objtypes/obj_camera.h"
#include "./objtypes/obj_portal.h"
#include "./objtypes/lighting/obj_light.h"
#include "./objtypes/sound/obj_sound.h"
#include "./objtypes/obj_text.h"
#include "./objtypes/obj_navmesh.h"
#include "./objtypes/emitter/obj_emitter.h"
#include "./objtypes/obj_mesh.h"
#include "./objtypes/obj_octree.h"
#include "./objtypes/obj_prefab.h"
#include "./objtypes/obj_util.h"
#include "./objtypes/video/obj_video.h"
#include "../colorselection.h"
#include "../shaders.h"

#include <unistd.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

// . reserved for prefix scene id
// : reserved for divider 
// attributes: mesh, disabled, textureoffset, texture
static Field obj = {
  .prefix = '`', 
  .objectType = OBJ_MESH,
};

// attributes: none
static Field camera = {
  .prefix = '>',
  .objectType = OBJ_CAMERA,
};

// attributes: camera, perspective
static Field portal = {
  .prefix = '@',
  .objectType = OBJ_PORTAL,
};

// attributes: clip
static Field sound = {
  .prefix = '&',
  .objectType = OBJ_SOUND,
};

// attributes: color
static Field light = {
  .prefix = '!',
  .objectType = OBJ_LIGHT,
};

static Field octreeField {
  .prefix = '*',
  .objectType = OBJ_OCTREE,
};

static Field emitterField {
  .prefix = '+',
  .objectType = OBJ_EMITTER,
};

static Field navmeshField {
  .prefix = ';',
  .objectType = OBJ_NAVMESH,
};

static Field uiTextField {
  .prefix = ')',
  .objectType = OBJ_TEXT,
};

static Field prefabField {
  .prefix = '[',
  .objectType = OBJ_PREFAB,
};

static Field videoField {
  .prefix = ']',
  .objectType = OBJ_VIDEO,
};

static std::vector fields = { 
  obj, 
  camera, 
  portal, 
  sound, 
  light, 
  octreeField,
  emitterField, 
  navmeshField, 
  uiTextField,
  prefabField,
  videoField,
};

ObjectType getObjectType(std::string& name);

struct Object {
  objid id;
  ObjectType type;
  unsigned int index;
};

struct ObjectMapping {
  std::unordered_map<objid, GameObjectMesh> mesh;
  std::unordered_map<objid, GameObjectCamera> camera;
  std::unordered_map<objid, GameObjectPortal> portal;
  std::unordered_map<objid, GameObjectSound> sound;
  std::unordered_map<objid, GameObjectLight> light;
  std::unordered_map<objid, GameObjectOctree> octree;
  std::unordered_map<objid, GameObjectEmitter> emitter;
  std::unordered_map<objid, GameObjectNavmesh> navmesh;
  std::unordered_map<objid, GameObjectUIText> text;
  std::unordered_map<objid, GameObjectPrefab> prefab;
  std::unordered_map<objid, GameObjectVideo> video;
};

ObjectMapping getObjectMapping();


void addObjectType(ObjectMapping& objectMapping, objid id, std::string objectType, GameobjAttributes& attr, ObjectTypeUtil util, ObjTypeLookup* _objtypeLookup);

void removeObject(
  ObjectMapping& objectMapping, 
  objid id, 
  std::function<void(std::string)> unbindCamera,
  std::function<void(objid)> unloadScene
);

struct DefaultMeshes {
  Mesh* nodeMesh;
  Mesh* portalMesh;
  Mesh* cameraMesh;
  Mesh* voxelCubeMesh; 
  Mesh* unitXYRect; // unit xy rect is a 1x1 2d plane along the xy axis, centered at the origin
  Mesh* soundMesh;
  Mesh* lightMesh;
  Mesh* emitter;
  Mesh* nav;
  Mesh* defaultCrosshairSprite;
};

struct RenderObjApi {
  std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine;
  std::function<void(glm::vec3)> drawSphere;
  std::function<int(GLint, objid, std::string, unsigned int, AlignType, TextWrap, TextVirtualization, UiTextCursor, std::string, bool)> drawWord;
  std::function<bool(objid)> isBone;
  std::function<std::optional<objid>(objid)> getParentId;
  std::function<Transformation(objid)> getTransform;
};

int renderObject(
  GLint shaderProgram,
  bool isSelectionShader,
  objid id, 
  ObjectMapping& objectMapping,
  int showDebugMask,
  unsigned int portalTexture,
  unsigned int navmeshTexture,
  glm::mat4& model,
  bool drawPoints,
  DefaultMeshes& defaultMeshes,
  bool selectionMode,
  bool drawBones,
  glm::mat4& finalModelMatrix,
  ObjTypeLookup& lookup
);

std::vector<std::pair<std::string, std::string>> getAdditionalFields(objid id, ObjectMapping& objectMapping, std::function<std::string(int)> getTextureName, std::function<void(std::string, std::string&)> saveFile);
std::optional<AttributeValuePtr> getObjectAttributePtr(ObjectMapping& objectMapping, objid id, const char* field);
bool setObjectAttribute(ObjectMapping& objectMapping, objid id, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&);

std::vector<Mesh>& getMeshesForId(ObjectMapping& mapping, objid id);

std::vector<std::string>& getMeshNames(ObjectMapping& mapping, objid id);
bool isNavmesh(ObjectMapping& mapping, objid id);
std::optional<Texture> textureForId(ObjectMapping& mapping, objid id);
void updateObjectPositions(ObjectMapping& mapping, objid, glm::vec3 position, Transformation& viewTransform);
void playSoundState(ObjectMapping& mapping, objid id, std::optional<float> volume, std::optional<glm::vec3> position);
void stopSoundState(ObjectMapping& mapping, objid id);

void onObjectSelected(objid id);
void onObjectUnselected();

GameObjectOctree* getOctree(ObjectMapping& mapping, objid id);
GameObjectNavmesh* getNavmesh(ObjectMapping& mapping, objid id);
GameObjectLight* getLight(ObjectMapping& mapping, objid id);
GameObjectPortal* getPortal(ObjectMapping& mapping, objid id);
GameObjectMesh* getMesh(ObjectMapping& mapping, objid id);
GameObjectPrefab* getPrefab(ObjectMapping& mapping, objid id);
GameObjectCamera* getCameraObj(ObjectMapping& mapping, objid id);
GameObjectUIText* getUIText(ObjectMapping& mapping, objid id);
GameObjectSound* getSoundObj(ObjectMapping& mapping, objid id);
GameObjectEmitter* getEmitter(ObjectMapping& mapping, objid id);
GameObjectVideo* getVideo(ObjectMapping& mapping, objid id);

std::vector<objid> getAllLightsIndexs(ObjectMapping& mapping);
std::vector<objid> getAllPortalIndexs(ObjectMapping& mapping);
std::vector<objid> getAllCameraIndexs(ObjectMapping& mapping);

bool objExists(ObjectMapping& mapping, objid id);



#endif 
