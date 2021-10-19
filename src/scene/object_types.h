#ifndef MOD_OBJECTTYPES
#define MOD_OBJECTTYPES

#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>
#include <variant>
#include "./common/mesh.h"
#include "./types/voxels.h"
#include "./types/heightmap.h"
#include "./types/ainav.h"
#include "./types/emitter.h"
#include "./serialization.h"
#include "./objtypes/obj_geo.h"
#include "./objtypes/obj_camera.h"
#include "./objtypes/obj_portal.h"
#include "./objtypes/obj_light.h"
#include "./objtypes/obj_sound.h"
#include "./objtypes/obj_text.h"

#include <unistd.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

struct TextureInformation {
  glm::vec2 textureoffset;
  glm::vec2 texturetiling;
  glm::vec2 texturesize;
  std::string textureOverloadName;
  int textureOverloadId;
};

struct GameObjectMesh {
  std::vector<std::string> meshNames;
  std::vector<Mesh> meshesToRender;   // @TODO  I shouldn't be storing the actual mesh here.  Instead I should just be referencing global meshes
  bool isDisabled;
  bool nodeOnly;
  std::string rootMesh;
  TextureInformation texture;
  float discardAmount;
  float emissionAmount;
  glm::vec3 tint;
};

struct GameObjectVoxel {
  Voxels voxel;
};

struct GameObjectRoot {};

struct GameObjectEmitter{};
struct GameObjectHeightmap{
  HeightMapData heightmap;
  Mesh mesh;
  TextureInformation texture;
};

struct GameObjectNavmesh {
  Mesh mesh;
};

struct GameObjectNavConns {
  NavGraph navgraph;
};

struct GameObjectUICommon {
  Mesh mesh;
  bool isFocused;
  std::string onFocus;
  std::string onBlur;
};

struct GameObjectUIButton {
  GameObjectUICommon common;
  bool initialState;
  bool toggleOn;
  bool canToggle;
  std::string onTextureString;
  int onTexture;
  std::string offTextureString;
  int offTexture;
  std::string onToggleOn;
  std::string onToggleOff;
  bool hasOnTint;
  glm::vec3 onTint;
};

struct GameObjectUISlider {
  GameObjectUICommon common;
  float min;
  float max;
  float percentage;
  int texture;
  int opacityTexture;
  std::string onSlide;
};

enum UILayoutType { LAYOUT_HORIZONTAL, LAYOUT_VERTICAL };
struct GameObjectUILayout {
  UILayoutType type;
  float spacing;
  std::vector<std::string> elements;
  int order;
  BoundInfo boundInfo;
  glm::vec3 boundOrigin;
  bool showBackpanel;
  glm::vec3 tint;
  float margin;
};

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

std::map<objid, GameObjectObj> getObjectMapping();

struct ObjectType {
  std::string name;
  std::size_t variantType;
  std::function<GameObjectObj(GameobjAttributes&)> createObj;
  std::function<void(GameObjectObj&, GameobjAttributes&)> objectAttributes;
  std::function<std::vector<std::pair<std::string, std::string>>(GameObjectObj&)> serialize;
};



void addObject(
  objid id, 
  std::string objectType, 
  GameobjAttributes& attr,
  std::map<objid, GameObjectObj>& mapping, 
  std::map<std::string, MeshRef>& meshes,
  std::function<std::vector<std::string>(std::string, std::vector<std::string>)> ensureMeshLoaded,
  std::function<Texture(std::string)> ensureTextureLoaded,
  std::function<Texture(std::string filepath, unsigned char* data, int textureWidth, int textureHeight, int numChannels)> ensureTextureDataLoaded,
  std::function<void()> onCollisionChange,
  std::function<void(float, float, int, GameobjAttributes&, std::vector<EmitterDelta>, bool, EmitterDeleteBehavior)> addEmitter,
  std::function<Mesh(MeshData&)> loadMesh
);

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
  std::function<void(GLint, objid, std::string, unsigned int, float)> drawWord,
  std::function<int(glm::vec3)> drawSphere,
  DefaultMeshes& defaultMeshes
);

std::vector<std::pair<std::string, std::string>> getAdditionalFields(objid id, std::map<objid, GameObjectObj>& mapping, std::function<std::string(int)> getTextureName);
void objectAttributes(std::map<objid, GameObjectObj>& mapping, objid id, GameobjAttributes& _attributes);
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
