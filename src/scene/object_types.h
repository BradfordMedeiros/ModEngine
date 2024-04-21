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
#include "./objtypes/obj_light.h"
#include "./objtypes/obj_sound.h"
#include "./objtypes/obj_text.h"
#include "./objtypes/obj_navmesh.h"
#include "./objtypes/obj_heightmap.h"
#include "./objtypes/obj_emitter.h"
#include "./objtypes/obj_mesh.h"
#include "./objtypes/obj_octree.h"
#include "./objtypes/obj_nil.h"
#include "./objtypes/obj_prefab.h"
#include "./objtypes/obj_util.h"
#include "../colorselection.h"

#include <unistd.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

struct GameObjectRoot {};

typedef std::variant<
  GameObjectMesh, 
  GameObjectCamera, 
  GameObjectPortal, 
  GameObjectSound, 
  GameObjectLight, 
  GameObjectOctree,
  GameObjectRoot, 
  GameObjectEmitter,
  GameObjectHeightmap,
  GameObjectNavmesh,
  GameObjectUIText,
  GameObjectNil,
  GameObjectPrefab
> GameObjectObj;

// . reserved for prefix scene id
// : reserved for divider 
// attributes: mesh, disabled, textureoffset, texture
static Field obj = {
  .prefix = '`', 
  .type = "default",
};

// attributes: none
static Field camera = {
  .prefix = '>',
  .type = "camera",
};

// attributes: camera, perspective
static Field portal = {
  .prefix = '@',
  .type = "portal",
};

// attributes: clip
static Field sound = {
  .prefix = '&',
  .type = "sound",
};

// attributes: color
static Field light = {
  .prefix = '!',
  .type = "light",
};

static Field octreeField {
  .prefix = '*',
  .type = "octree",
};

static Field rootField {
  .prefix = '~',
  .type = "root",
};

static Field emitterField {
  .prefix = '+',
  .type = "emitter",
};

static Field heightmap {
  .prefix = '-',
  .type = "heightmap",
};

static Field navmeshField {
  .prefix = ';',
  .type = "navmesh",
};

static Field uiTextField {
  .prefix = ')',
  .type = "text",
};

static Field customField {
  .prefix = '|',
  .type = "custom",
};

static Field prefabField {
  .prefix = '[',
  .type = "prefab",
};

static std::vector fields = { 
  obj, 
  camera, 
  portal, 
  sound, 
  light, 
  octreeField,
  rootField, 
  emitterField, 
  heightmap, 
  navmeshField, 
  uiTextField,
  customField,
  prefabField,
};

std::string getType(std::string name);

std::map<objid, GameObjectObj> getObjectMapping();

struct ObjectType {
  std::string name;
  std::size_t variantType;
  std::function<GameObjectObj(GameobjAttributes&, ObjectTypeUtil&)> createObj;
  std::function<void(GameObjectObj&, GameobjAttributes&)> objectAttributes;
  std::function<std::optional<AttributeValuePtr>(GameObjectObj&, const char* field)> objectAttribute;
  std::function<bool(GameObjectObj&, GameobjAttributes&, ObjectSetAttribUtil& util)> setAttributes;
  std::function<std::vector<std::pair<std::string, std::string>>(GameObjectObj&, ObjectSerializeUtil&)> serialize;
  std::function<void(GameObjectObj&, ObjectRemoveUtil&)> removeObject;
};

GameObjectObj createObjectType(std::string objectType, GameobjAttributes& attr, ObjectTypeUtil util);
void addObjectType(std::map<objid, GameObjectObj>& mapping, GameObjectObj& gameobj, objid id);

void removeObject(
  std::map<objid, GameObjectObj>& mapping, 
  objid id, 
  std::function<void(std::string)> unbindCamera,
  std::function<void()> rmEmitter,
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
  Mesh* crosshairSprite;
};

int renderObject(
  GLint shaderProgram, 
  objid id, 
  std::map<objid, GameObjectObj>& mapping,
  int showDebugMask,
  unsigned int portalTexture,
  unsigned int navmeshTexture,
  glm::mat4 model,
  bool drawPoints,
  std::function<int(GLint, objid, std::string, unsigned int, AlignType, TextWrap, TextVirtualization, UiTextCursor, std::string, bool)> drawWord,
  std::function<int(glm::vec3)> drawSphere,
  DefaultMeshes& defaultMeshes,
  std::function<void(int)> onRender,
  bool selectionMode
);

std::vector<std::pair<std::string, std::string>> getAdditionalFields(objid id, std::map<objid, GameObjectObj>& mapping, std::function<std::string(int)> getTextureName);
void objectAttributes(GameObjectObj& toRender, GameobjAttributes& _attributes);
std::optional<AttributeValuePtr> getObjectAttributePtr(GameObjectObj& toRender, const char* field);

bool setObjectAttributes(std::map<objid, GameObjectObj>& mapping, objid id, GameobjAttributes& attributes, ObjectSetAttribUtil& util);

template<typename T>
std::vector<objid> getGameObjectsIndex(std::map<objid, GameObjectObj>& mapping){   // putting templates have to be in header?
  std::vector<objid> indicies;
  for (auto [id, gameobject]: mapping){
    auto gameobjectP = std::get_if<T>(&gameobject);
    if (gameobjectP != NULL){
      indicies.push_back(id);
    }
  }
  return indicies;
}

std::vector<objid> getGameObjectsIndex(std::map<objid, GameObjectObj>& mapping);

struct NameAndMesh {
  std::vector<std::string*> meshNames;
  std::vector<Mesh*> meshes;
};
NameAndMesh getMeshesForId(std::map<objid, GameObjectObj>& mapping, objid id);

std::vector<std::string> getMeshNames(std::map<objid, GameObjectObj>& mapping, objid id);
std::map<objid, GameObjectHeightmap*> getHeightmaps(std::map<objid, GameObjectObj>& mapping);
bool isNavmesh(std::map<objid, GameObjectObj>& mapping, objid id);
std::optional<Texture> textureForId(std::map<objid, GameObjectObj>& mapping, objid id);
void updateObjectPositions(std::map<objid, GameObjectObj>& mapping, objid, glm::vec3 position, Transformation& viewTransform);
void playSoundState(std::map<objid, GameObjectObj>& mapping, objid id, std::optional<float> volume, std::optional<glm::vec3> position);
void stopSoundState(std::map<objid, GameObjectObj>& mapping, objid id);
void onObjectFrame(std::map<objid, GameObjectObj>& mapping, std::function<void(std::string texturepath, unsigned char* data, int textureWidth, int textureHeight)> updateTextureData, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, float timestamp);

void onObjectSelected(objid id);
void onObjectUnselected();

#endif 
