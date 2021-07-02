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
#include "../common/sysinterface.h"
#include "./types/ainav.h"
#include "./sprites/sprites.h"
#include "./common/util/meshgen.h"

struct World {
  physicsEnv physicsEnvironment;
  std::map<objid, btRigidBody*> rigidbodys;
  std::map<objid, GameObjectObj> objectMapping;
  std::map<std::string, Mesh> meshes;
  std::map<std::string, Texture> textures;
  std::map<objid, std::vector<Animation>> animations;
  EmitterSystem emitters;
  std::function<void(GameObject&)> onObjectUpdate;
  std::function<void(GameObject&)> onObjectCreate;
  std::function<void(objid, bool)> onObjectDelete;
  std::set<objid> entitiesToUpdate;
  SceneSandbox sandbox;
};

World createWorld(
  collisionPairPosFn onObjectEnter, 
  collisionPairFn onObjectLeave, 
  std::function<void(GameObject&)> onObjectUpdate, 
  std::function<void(GameObject&)> onObjectCreate,
  std::function<void(objid, bool)> onObjectDelete,
  btIDebugDraw* debugDrawer,
  std::vector<LayerInfo> layers,
  SysInterface interface
);

void addSerialObjectsToWorld(World& world, objid sceneId, std::vector<objid> idsAdded, std::function<objid()> getNewObjectId, SysInterface interface, std::map<std::string, GameobjAttributes> additionalFields);
Texture loadTextureWorld(World& world, std::string texturepath);

objid addObjectToScene(World& world, objid sceneId, std::string name, GameobjAttributes attributes, SysInterface interface);

objid addSceneToWorld(World& world, std::string sceneFile, SysInterface interface);
objid addSceneToWorldFromData(World& world, std::string sceneFileName, objid sceneId, std::string sceneData, SysInterface interface);
void removeSceneFromWorld(World& world, objid sceneId, SysInterface interface);
void removeAllScenesFromWorld(World& world, SysInterface interface);

objid addObjectToScene(World& world, objid sceneId, std::string serializedObj, objid id, bool useObjId, SysInterface interface);
void removeObjectFromScene(World& world, objid id, SysInterface interface);
void copyObjectToScene(World& world, objid id, SysInterface interface);

std::map<std::string, std::string> getAttributes(World& world, objid id);
void setAttributes(World& world, objid id, GameobjAttributes& attr);
void setProperty(World& world, objid id, std::vector<Property>& properties);
AttributeValue interpolateAttribute(AttributeValue key1, AttributeValue key2, float percentage);
AttributeValue parsePropertySuffix(std::string key, std::string value);
std::string serializePropertySuffix(std::string key, AttributeValue value);

std::string serializeScene(World& world, objid sceneId, bool includeIds);
std::string serializeObject(World& world, objid id, std::string overridename = "");

void physicsTranslateSet(World& world, objid index, glm::vec3 pos);

void physicsRotateSet(World& world, objid index, glm::quat rotation);  // this sets to rotation

void physicsScaleSet(World& world, objid index, glm::vec3 scale);

void applyPhysicsTranslation(World& world, objid index, glm::vec3 position, float offsetX, float offsetY, Axis manipulatorAxis);
void applyPhysicsRotation(World& world, objid index, glm::quat currentOrientation, float offsetX, float offsetY, Axis manipulatorAxis);
void applyPhysicsScaling(World& world, objid index, glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, Axis manipulatorAxis);

void updatePhysicsBody(World& world, objid id);

void onWorldFrame(World& world, float timestep, float timeElapsed, bool enablePhysics, bool dumpPhysics, SysInterface interface);

NameAndMesh getMeshesForGroupId(World& world, objid id);

std::optional<objid> getGameObjectByName(World& world, std::string name, objid sceneId);
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
void loadMeshData(World& world, std::string meshPath, MeshData& meshData);

struct Properties {
  Transformation transformation;
};
Properties getProperties(World& world, objid id);
void setProperties(World& world, objid id, Properties& properties);

std::string sceneNameForSceneId(World& world, objid sceneId);

#endif
