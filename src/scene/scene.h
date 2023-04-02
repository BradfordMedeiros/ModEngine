#ifndef MOD_SCENE
#define MOD_SCENE

#include <iostream>
#include <map>
#include "./scene_sandbox.h"
#include "./physics.h"
#include "./object_types.h"
#include "./types/emitter.h"
#include "./scene_debug.h"
#include "./common/mesh.h"
#include "../translations.h"
#include "../common/util.h"
#include "./sysinterface.h"
#include "./types/ainav.h"
#include "./sprites/sprites.h"
#include "./common/util/meshgen.h"

struct World {
  physicsEnv physicsEnvironment;
  std::map<objid, PhysicsValue> rigidbodys;
  std::map<objid, GameObjectObj> objectMapping;
  std::map<std::string, MeshRef> meshes;
  std::map<std::string, TextureRef> textures;
  std::map<objid, std::vector<Animation>> animations;
  EmitterSystem emitters;
  std::function<void(GameObject&)> onObjectUpdate;
  std::function<void(GameObject&)> onObjectCreate;
  std::function<void(objid, bool)> onObjectDelete;
  std::set<objid> entitiesToUpdate;
  SceneSandbox sandbox;

  SysInterface interface;
};

std::set<std::string> getObjautoserializerFields(std::string& name);

World createWorld(
  collisionPairPosFn onObjectEnter, 
  collisionPairFn onObjectLeave, 
  std::function<void(GameObject&)> onObjectUpdate, 
  std::function<void(GameObject&)> onObjectCreate,
  std::function<void(objid, bool)> onObjectDelete,
  btIDebugDraw* debugDrawer,
  std::vector<LayerInfo> layers,
  SysInterface interface,
  std::vector<std::string> defaultMeshes,
  std::vector<std::string> spriteMeshes
);

void addSerialObjectsToWorld(World& world, objid sceneId, std::vector<objid>& idsAdded, std::function<objid()> getNewObjectId, std::map<std::string, GameobjAttributesWithId> additionalFields, bool returnObjectOnly, std::vector<GameObjectObj>& gameobjObjs, std::map<std::string, GameobjAttributes>& submodelAttributes);
Texture loadTextureWorld(World& world, std::string texturepath, objid ownerId);
Texture loadTextureWorldEmpty(World& world, std::string texturepath, objid ownerId, int textureWidth, int textureHeight, std::optional<objid> mappingTexture);

objid addSceneToWorld(World& world, std::string sceneFile, std::vector<Token>& addedTokens, std::optional<std::string> name, std::optional<std::vector<std::string>> tags, std::optional<objid> sceneId = std::nullopt);
objid addSceneToWorldFromData(World& world, std::string sceneFileName, objid sceneId, std::string sceneData, std::optional<std::string> name, std::optional<std::vector<std::string>> tags);
void removeSceneFromWorld(World& world, objid sceneId);
void removeAllScenesFromWorld(World& world);

struct GameObjPair {
  GameObject gameobj;
  GameObjectObj gameobjObj;
};
GameObjPair createObjectForScene(World& world, objid sceneId, std::string& name, std::string& serializedObj);
objid addObjectToScene(World& world, objid sceneId, std::string name, AttrChildrenPair attrWithChildren, std::map<std::string, GameobjAttributes>& submodelAttributes);
objid addObjectToScene(World& world, objid sceneId, std::string serializedObj, objid id, bool useObjId);

void removeObjectFromScene(World& world, objid id);
void copyObjectToScene(World& world, objid id);

GameobjAttributes objectAttributes(GameObjectObj& gameobjObj, GameObject& gameobj);
GameobjAttributes objectAttributes(World& world, objid id);
bool objectHasAttribute(World& world, objid id, std::string type, std::optional<AttributeValue> value);
void setAttributes(World& world, objid id, GameobjAttributes& attr);
void setProperty(World& world, objid id, std::vector<Property>& properties);
AttributeValue interpolateAttribute(AttributeValue key1, AttributeValue key2, float percentage);
AttributeValue parsePropertySuffix(std::string key, std::string value);
std::string serializePropertySuffix(std::string key, AttributeValue value);

std::optional<std::string> getTextureById(World& world, int id);
std::string serializeScene(World& world, objid sceneId, bool includeIds);
std::string serializeObject(World& world, objid id, bool includeSubmodelAttr, std::string overridename = "");

void physicsTranslateSet(World& world, objid index, glm::vec3 pos, bool relative);
void physicsRotateSet(World& world, objid index, glm::quat rotation, bool relative);  
void physicsScaleSet(World& world, objid index, glm::vec3 scale);

void applyPhysicsTranslation(World& world, objid index, float offsetX, float offsetY, Axis manipulatorAxis);
void applyPhysicsRotation(World& world, objid index, float offsetX, float offsetY, Axis manipulatorAxis);
void applyPhysicsScaling(World& world, objid index, float lastX, float lastY, float offsetX, float offsetY, Axis manipulatorAxis);
void physicsLocalTransformSet(World& world, objid index, Transformation transform);

bool hasPhysicsBody(World& world, objid id);
void updatePhysicsBody(World& world, objid id);

void onWorldFrame(World& world, float timestep, float timeElapsed, bool enablePhysics, bool dumpPhysics, bool paused);

NameAndMeshObjName getMeshesForGroupId(World& world, objid id);

std::optional<objid> getGameObjectByNamePrefix(World& world, std::string name, objid sceneId, bool sceneIdExplicit);
GameObject& getGameObject(World& world, objid id);
GameObject& getGameObject(World& world, std::string name, objid sceneId);

int getNumberOfObjects(World& world);

struct GroupPhysicsInfo {
  bool isRoot;
  PhysicsInfo physicsInfo;
  physicsOpts physicsOptions;
};
GroupPhysicsInfo getPhysicsInfoForGroup(World& world, objid id);
PhysicsInfo getPhysicsInfoForGameObject(World& world, objid index);
void loadMeshData(World& world, std::string meshPath, MeshData& meshData, objid ownerId);

struct Properties {
  Transformation transformation;
};
Properties getProperties(World& world, objid id);
void setProperties(World& world, objid id, Properties& properties);

std::string sceneFileForSceneId(World& world, objid sceneId);
std::optional<std::string> sceneNameForSceneId(World& world, objid sceneId);
std::vector<std::string> sceneTagsForSceneId(World& world, objid sceneId);

void loadSkybox(World& world, std::string skyboxpath);

std::string getType(std::string name);

std::optional<objid> getMappingTexture(World& world, std::string& texture);
void freeTextureRefsIdByOwner(World& world, int ownerId, std::optional<int> id);
void freeTextureRefsByOwner(World& world, int ownerId);

#endif
