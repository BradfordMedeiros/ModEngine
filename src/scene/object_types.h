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
#include "./types/sound.h"
#include "./types/video.h"
#include "./types/emitter.h"
#include "./serialization.h"
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
struct GameObjectCamera {};
struct GameObjectPortal {
  std::string camera;
  bool perspective;
};
struct GameObjectSound{
  std::string clip;  
  ALuint source;
};

enum LightType { LIGHT_POINT, LIGHT_SPOTLIGHT, LIGHT_DIRECTIONAL };
struct GameObjectLight {
  glm::vec3 color;
  LightType type;
  float maxangle;
  glm::vec3 attenuation;
};
struct GameObjectVoxel {
  Voxels voxel;
};
struct GameObjectChannel {
  std::string from;
  std::string to;
  bool complete;
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

struct GameObjectUIText {
  std::string value;
  float deltaOffset;
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

struct GameObjectVideo {
  VideoContent video;
  std::string source;
  BufferedAudio sound;
};

enum GeoShapeType { GEODEFAULT, GEOSPHERE };
struct GameObjectGeo {
  std::vector<glm::vec3> points;
  GeoShapeType type;
};

typedef std::variant<
  GameObjectMesh, 
  GameObjectCamera, 
  GameObjectPortal, 
  GameObjectSound, 
  GameObjectLight, 
  GameObjectVoxel, 
  GameObjectChannel, 
  GameObjectRoot, 
  GameObjectEmitter,
  GameObjectHeightmap,
  GameObjectNavmesh,
  GameObjectNavConns,
  GameObjectUIButton,
  GameObjectUISlider,
  GameObjectUIText,
  GameObjectUILayout,
  GameObjectVideo,
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

// attributes: from, to
static Field channelField {
  .prefix = '%',
  .type = "channel",
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

static Field videoField {
  .prefix = '=',
  .type = "video",
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
  channelField, 
  rootField, 
  emitterField, 
  heightmap, 
  navmeshField, 
  navconnectionField, 
  uiButtonField, 
  uiSliderField,
  videoField,
  uiTextField,
  uiLayoutField,
  geoField,
};

std::map<objid, GameObjectObj> getObjectMapping();

void addObject(
  objid id, 
  std::string objectType, 
  GameobjAttributes& attr,
  std::map<objid, GameObjectObj>& mapping, 
  std::map<std::string, Mesh>& meshes, std::string defaultMesh, 
  std::function<bool(std::string, std::vector<std::string>)> ensureMeshLoaded,
  std::function<Texture(std::string)> ensureTextureLoaded,
  std::function<Texture(std::string filepath, unsigned char* data, int textureWidth, int textureHeight, int numChannels)> ensureTextureDataLoaded,
  std::function<void()> onCollisionChange,
  std::function<void(float, float, int, std::map<std::string, std::string>, std::vector<EmitterDelta> deltas, bool)> addEmitter,
  std::function<Mesh(MeshData&)> loadMesh
);

void removeObject(
  std::map<objid, GameObjectObj>& mapping, 
  objid id, 
  std::function<void(std::string)> unbindCamera,
  std::function<void()> rmEmitter
);

int renderObject(
  GLint shaderProgram, 
  objid id, 
  std::map<objid, GameObjectObj>& mapping,
  Mesh& nodeMesh,
  Mesh& cameraMesh, 
  Mesh& portalMesh, 
  Mesh& voxelCubeMesh,
  Mesh& unitXYRect,
  bool showDebug, 
  bool showBoneWeight,
  bool useBoneTransform,
  unsigned int portalTexture,
  glm::mat4 model,
  bool drawPoints,
  std::function<void(GLint, objid, std::string, unsigned int, float)> drawWord,
  std::function<int(glm::vec3)> drawSphere
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
std::map<std::string, std::vector<std::string>> getChannelMapping(std::map<objid, GameObjectObj>& mapping);
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
