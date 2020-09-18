#ifndef MOD_OBJECTTYPES
#define MOD_OBJECTTYPES

#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>
#include <variant>
#include "./common/mesh.h"
#include "./scenegraph.h"
#include "./voxels.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

struct GameObjectMesh {
  std::vector<std::string> meshNames;
  std::vector<Mesh> meshesToRender;   // @TODO  I shouldn't be storing the actual mesh here.  Instead I should just be referencing global meshes
  bool isDisabled;
  bool nodeOnly;
  std::string rootMesh;
  glm::vec2 textureoffset;
  std::string textureOverloadName;
  int textureOverloadId;;
};
struct GameObjectCamera {};
struct GameObjectPortal {
  std::string camera;
  bool perspective;
};
struct GameObjectSound{
  std::string clip;  
};

enum LightType { LIGHT_POINT, LIGHT_SPOTLIGHT };
struct GameObjectLight {
  /*!light:type:directional
  !light:type:directional 
  !light:maxangle:50*/
  glm::vec3 color;
  LightType type;
  float maxangle;
};
struct GameObjectVoxel {
  Voxels voxel;
};
struct GameObjectChannel {
  std::string from;
  std::string to;
  bool complete;
};

struct RailConnection {
  std::string from;
  std::string to;
};
struct GameObjectRail {
  objid id;
  RailConnection connection;
};

struct GameObjectScene {
  std::string scenefile;
};

struct GameObjectRoot {};

struct GameObjectEmitter{};

typedef std::variant<GameObjectMesh, GameObjectCamera, GameObjectPortal, GameObjectSound, GameObjectLight, GameObjectVoxel, GameObjectChannel, GameObjectRail, GameObjectScene, GameObjectRoot, GameObjectEmitter> GameObjectObj;

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

// attributes: from, to
static Field railField {
  .prefix = '^',
  .type = "rail",
};

static Field sceneField {
  .prefix = '$',
  .type = "scene",
};

static Field rootField {
  .prefix = '~',
  .type = "root",
};

static Field emitterField {
  .prefix = '+',
  .type = "emitter",
};

static std::vector fields = { obj, camera, portal, sound, light, voxelField, channelField, railField, sceneField, rootField, emitterField };

std::map<objid, GameObjectObj> getObjectMapping();

void addObject(
  objid id, 
  std::string objectType, 
  std::map<std::string, std::string> additionalFields,
  std::map<objid, GameObjectObj>& mapping, 
  std::map<std::string, Mesh>& meshes, std::string defaultMesh, 
  std::function<void(std::string)> loadClip,
  std::function<bool(std::string, std::vector<std::string>)> ensureMeshLoaded,
  std::function<int(std::string)> ensureTextureLoaded,
  std::function<void()> onVoxelBoundInfoChanged,
  std::function<void(objid id, std::string from, std::string to)> onRail,
  std::function<void(std::string)> loadScene,
  std::function<void(float, float, int, std::map<std::string, std::string>)> addEmitter
);

void removeObject(
  std::map<objid, GameObjectObj>& mapping, 
  objid id, 
  std::function<void(std::string)> unloadClip, 
  std::function<void()> removeRail, 
  std::function<void(std::string)> unbindCamera,
  std::function<void()> rmEmitter
);

void renderObject(
  GLint shaderProgram, 
  objid id, 
  std::map<objid, GameObjectObj>& mapping,
  Mesh& nodeMesh,
  Mesh& cameraMesh, 
  bool showDebug, 
  bool showBoneWeight,
  bool useBoneTransform,
  unsigned int portalTexture
);

std::vector<std::pair<std::string, std::string>> getAdditionalFields(objid id, std::map<objid, GameObjectObj>& mapping);
std::map<std::string, std::string> objectAttributes(std::map<objid, GameObjectObj>& mapping, objid id);
void setObjectAttributes(std::map<objid, GameObjectObj>& mapping, objid id, std::map<std::string, std::string> attributes);

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
std::map<objid, RailConnection> getRails(std::map<objid, GameObjectObj>& mapping);

#endif 
