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

typedef std::variant<
  GameObjectMesh, 
  GameObjectCamera, 
  GameObjectPortal, 
  GameObjectSound, 
  GameObjectLight, 
  GameObjectOctree,
  GameObjectEmitter,
  GameObjectNavmesh,
  GameObjectUIText,
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

static Field emitterField {
  .prefix = '+',
  .type = "emitter",
};

static Field navmeshField {
  .prefix = ';',
  .type = "navmesh",
};

static Field uiTextField {
  .prefix = ')',
  .type = "text",
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
  emitterField, 
  navmeshField, 
  uiTextField,
  prefabField,
};

struct ObjectMapping {
  std::map<objid, GameObjectObj> objects;
};

ObjectMapping getObjectMapping();


void addObjectType(ObjectMapping& objectMapping, objid id, std::string objectType, GameobjAttributes& attr, ObjectTypeUtil util);

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
  glm::mat4 model,
  bool drawPoints,
  DefaultMeshes& defaultMeshes,
  bool selectionMode,
  bool drawBones,
  RenderObjApi api,
  glm::mat4& finalModelMatrix
);

std::vector<std::pair<std::string, std::string>> getAdditionalFields(objid id, std::map<objid, GameObjectObj>& mapping, std::function<std::string(int)> getTextureName, std::function<void(std::string, std::string&)> saveFile);
std::optional<AttributeValuePtr> getObjectAttributePtr(GameObjectObj& toRender, const char* field);
bool setObjectAttribute(ObjectMapping& objectMapping, objid id, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&);

template<typename T>
std::vector<objid> getGameObjectsIndex(ObjectMapping& mapping){   // putting templates have to be in header?
  std::vector<objid> indicies;
  for (auto [id, gameobject]: mapping.objects){
    auto gameobjectP = std::get_if<T>(&gameobject);
    if (gameobjectP != NULL){
      indicies.push_back(id);
    }
  }
  return indicies;
}

std::vector<Mesh>& getMeshesForId(std::map<objid, GameObjectObj>& mapping, objid id);

std::vector<std::string>& getMeshNames(std::map<objid, GameObjectObj>& mapping, objid id);
bool isNavmesh(std::map<objid, GameObjectObj>& mapping, objid id);
std::optional<Texture> textureForId(std::map<objid, GameObjectObj>& mapping, objid id);
void updateObjectPositions(std::map<objid, GameObjectObj>& mapping, objid, glm::vec3 position, Transformation& viewTransform);
void playSoundState(std::map<objid, GameObjectObj>& mapping, objid id, std::optional<float> volume, std::optional<glm::vec3> position);
void stopSoundState(std::map<objid, GameObjectObj>& mapping, objid id);

void onObjectSelected(objid id);
void onObjectUnselected();

#endif 
