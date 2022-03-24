#ifndef MOD_OBJECTTYPES
#define MOD_OBJECTTYPES

#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>
#include <variant>
#include "./common/mesh.h"
#include "./types/ainav.h"
#include "./serialization.h"
#include "./objtypes/obj_geo.h"
#include "./objtypes/obj_camera.h"
#include "./objtypes/obj_portal.h"
#include "./objtypes/obj_light.h"
#include "./objtypes/obj_sound.h"
#include "./objtypes/obj_text.h"
#include "./objtypes/obj_uilayout.h"
#include "./objtypes/obj_uibutton.h"
#include "./objtypes/obj_uislider.h"
#include "./objtypes/obj_navconn.h"
#include "./objtypes/obj_navmesh.h"
#include "./objtypes/obj_heightmap.h"
#include "./objtypes/obj_emitter.h"
#include "./objtypes/obj_mesh.h"
#include "./objtypes/obj_voxel.h"
#include "./objtypes/obj_util.h"

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
  GameObjectVoxel, 
  GameObjectRoot, 
  GameObjectEmitter,
  GameObjectHeightmap,
  GameObjectNavmesh,
  GameObjectNavConns,
  GameObjectUIButton,
  GameObjectUISlider,
  GameObjectUIText,
  GameObjectUILayout,
  GameObjectGeo
> GameObjectObj;

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

// attributes: from
static Field voxelField = {
  .prefix = ']',
  .type = "voxel",
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

static Field navconnectionField {
  .prefix = '\'',
  .type = "navconnection",
};

static Field uiButtonField {
  .prefix = '*',
  .type = "ui",
};

static Field uiSliderField {
  .prefix = '/',
  .type = "slider",
};

static Field uiTextField {
  .prefix = ')',
  .type = "text",
};

static Field uiLayoutField {
  .prefix = '(',
  .type = "layout",
};

static Field geoField {
  .prefix = '<',
  .type = "geo",
};

static std::vector fields = { 
  obj, 
  camera, 
  portal, 
  sound, 
  light, 
  voxelField, 
  rootField, 
  emitterField, 
  heightmap, 
  navmeshField, 
  navconnectionField, 
  uiButtonField, 
  uiSliderField,
  uiTextField,
  uiLayoutField,
  geoField,
};

std::string getType(std::string name);

std::map<objid, GameObjectObj> getObjectMapping();

struct ObjectType {
  std::string name;
  std::size_t variantType;
  std::function<GameObjectObj(GameobjAttributes&, ObjectTypeUtil&)> createObj;
  std::function<void(GameObjectObj&, GameobjAttributes&)> objectAttributes;
  std::function<void(GameObjectObj&, GameobjAttributes&, ObjectSetAttribUtil& util)> setAttributes;
  std::function<std::vector<std::pair<std::string, std::string>>(GameObjectObj&, ObjectSerializeUtil&)> serialize;
  std::function<void(GameObjectObj&, ObjectRemoveUtil&)> removeObject;
};

GameObjectObj createObjectType(std::string objectType, GameobjAttributes& attr, ObjectTypeUtil util);
void addObjectType(std::map<objid, GameObjectObj>& mapping, GameObjectObj& gameobj, objid id);

void removeObject(
  std::map<objid, GameObjectObj>& mapping, 
  objid id, 
  std::function<void(std::string)> unbindCamera,
  std::function<void()> rmEmitter
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
};

int renderObject(
  GLint shaderProgram, 
  objid id, 
  std::map<objid, GameObjectObj>& mapping,
  bool showDebug, 
  bool showBoneWeight,
  bool useBoneTransform,
  unsigned int portalTexture,
  glm::mat4 model,
  bool drawPoints,
  std::function<int(GLint, objid, std::string, unsigned int, float, AlignType)> drawWord,
  std::function<int(glm::vec3)> drawSphere,
  DefaultMeshes& defaultMeshes
);

std::vector<std::pair<std::string, std::string>> getAdditionalFields(objid id, std::map<objid, GameObjectObj>& mapping, std::function<std::string(int)> getTextureName);
void objectAttributes(GameObjectObj& toRender, GameobjAttributes& _attributes);
void setObjectAttributes(std::map<objid, GameObjectObj>& mapping, objid id, GameobjAttributes& attributes, std::function<void(bool)> setEmitterEnabled);

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
NameAndMesh getMeshesForId(std::map<objid, GameObjectObj>& mapping, objid id);
std::vector<std::string> getMeshNames(std::map<objid, GameObjectObj>& mapping, objid id);
std::map<objid, GameObjectHeightmap*> getHeightmaps(std::map<objid, GameObjectObj>& mapping);
bool isNavmesh(std::map<objid, GameObjectObj>& mapping, objid id);
std::optional<Texture> textureForId(std::map<objid, GameObjectObj>& mapping, objid id);
void applyFocusUI(std::map<objid, GameObjectObj>& mapping, objid id, std::function<void(std::string, std::string)> sendNotify);
void applyKey(std::map<objid, GameObjectObj>& mapping, char key, std::function<void(std::string)> applyText);
void applyUICoord(std::map<objid, GameObjectObj>& mapping, std::function<void(std::string, float)> onSliderPercentage, objid id, float uvx, float uvy);
void updatePosition(std::map<objid, GameObjectObj>& mapping, objid, glm::vec3 position);
void playSoundState(std::map<objid, GameObjectObj>& mapping, objid id);
void onObjectFrame(std::map<objid, GameObjectObj>& mapping, std::function<void(std::string texturepath, unsigned char* data, int textureWidth, int textureHeight)> updateTextureData, float timestamp);

#endif 
