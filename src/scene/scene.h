#ifndef MOD_SCENE
#define MOD_SCENE

#include <iostream>
#include <map>
#include "./scenegraph.h"
#include "./physics.h"
#include "./object_types.h"
#include "./types/rails.h"
#include "./types/emitter.h"
#include "./scene_debug.h"
#include "./common/mesh.h"
#include "../translations.h"
#include "../common/util.h"
#include "../common/sysinterface.h"

struct World {
  physicsEnv physicsEnvironment;
  std::map<objid, btRigidBody*> rigidbodys;
  std::map<objid, GameObjectObj> objectMapping;
  std::map<std::string, Mesh> meshes;
  std::map<std::string, Texture> textures;
  std::map<objid, std::vector<Animation>> animations;
  std::map<objid, Scene> scenes;
  std::map<objid, objid> idToScene;
  std::map<std::string, std::map<std::string, std::string>> meshnameToBoneToParent;
  RailSystem rails;
  EmitterSystem emitters;
  std::function<void(GameObject&)> onObjectUpdate;
  std::function<void(GameObject&)> onObjectCreate;
  std::function<void(objid, bool)> onObjectDelete;
  std::set<objid> entitiesToUpdate;
};

World createWorld(
  collisionPairPosFn onObjectEnter, 
  collisionPairFn onObjectLeave, 
  std::function<void(GameObject&)> onObjectUpdate, 
  std::function<void(GameObject&)> onObjectCreate,
  std::function<void(objid, bool)> onObjectDelete,
  btIDebugDraw* debugDrawer
);
Texture loadTextureWorld(World& world, std::string texturepath);

objid addObjectToScene(World& world, objid sceneId, std::string name, GameobjAttributes attributes, SysInterface interface);

objid addSceneToWorld(World& world, std::string sceneFile, SysInterface interface);
objid addSceneToWorldFromData(World& world, objid sceneId, std::string sceneData, SysInterface interface);
void removeSceneFromWorld(World& world, objid sceneId, SysInterface interface);
void removeAllScenesFromWorld(World& world, SysInterface interface);

objid addObjectToScene(World& world, objid sceneId, std::string serializedObj, objid id, bool useObjId, SysInterface interface);

void removeObjectFromScene(World& world, objid id, SysInterface interface);

Properties getProperties(World& world, objid id);
void setProperties(World& world, objid id, Properties& properties);

std::map<std::string, std::string> getAttributes(World& world, objid id);
void setAttributes(World& world, objid id, std::map<std::string, std::string> attr);

std::string serializeScene(World& world, objid sceneId, bool includeIds);
std::string serializeObject(World& world, objid id);

void physicsTranslate(World& world, objid index, float x, float y, float z, bool moveRelativeEnabled);
void physicsTranslateSet(World& world, objid index, glm::vec3 pos);

void physicsRotate(World& world, objid index, float x, float y, float z);
void physicsRotateSet(World& world, objid index, glm::quat rotation);  // this sets to rotation

void physicsScale(World& world, objid index, float x, float y, float z);
void physicsScaleSet(World& world, objid index, glm::vec3 scale);

void applyPhysicsTranslation(World& world, objid index, glm::vec3 position, float offsetX, float offsetY, Axis manipulatorAxis);
void applyPhysicsRotation(World& world, objid index, glm::quat currentOrientation, float offsetX, float offsetY, Axis manipulatorAxis);
void applyPhysicsScaling(World& world, objid index, glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, Axis manipulatorAxis);

void updatePhysicsBody(World& world, Scene& scene, objid id);

void onWorldFrame(World& world, float timestep, float timeElapsed, bool enablePhysics, bool dumpPhysics, SysInterface interface);

objid getIdForCollisionObject(World& world,  const btCollisionObject* body);
NameAndMesh getMeshesForGroupId(World& world, objid id);

std::optional<objid> getGameObjectByName(World& world, std::string name);
GameObject& getGameObject(World& world, std::string name);
GameObject& getGameObject(World& world, objid id);
Transformation fullTransformation(World& world, objid id);
objid getGroupId(World& world, objid id);
std::vector<objid> getIdsInGroup(World& world, objid index);

bool idInGroup(World& world, objid id, objid groupId);
bool idExists(World& world, objid id);


std::vector<HitObject> raycast(World& world, glm::vec3 posFrom, glm::quat direction, float maxDistance);

void traverseScene(World& world, Scene& scene, std::function<void(objid, glm::mat4, glm::mat4, bool, std::string)> onObject);
Transformation fullTransformation(World& world, Scene& scene, objid id);

struct LightInfo {
  glm::vec3 pos;
  glm::quat rotation;
  GameObjectLight light;
};

std::vector<LightInfo> getLightInfo(World& world);


struct PortalInfo {
  glm::vec3 cameraPos;
  glm::quat cameraRotation;
  glm::vec3 portalPos;
  glm::quat portalRotation;
  bool perspective;
  objid id;
};
PortalInfo getPortalInfo(World& world, objid id);
std::vector<PortalInfo> getPortalInfo(World& world);
bool isPortal(World& world, objid id);

#endif

