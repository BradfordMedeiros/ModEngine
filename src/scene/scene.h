#ifndef MOD_SCENE
#define MOD_SCENE

#include <iostream>
#include <map>
#include "./scenegraph.h"
#include "./physics.h"
#include "./object_types.h"
#include "./rails.h"
#include "./scene_debug.h"
#include "./common/mesh.h"
#include "../translations.h"
#include "../common/util.h"

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

objid addSceneToWorld(World& world, std::string sceneFile, std::function<void(std::string)> loadClip, std::function<void(std::string, objid)> loadScript);
objid addSceneToWorldFromData(World& world, objid sceneId, std::string sceneData, std::function<void(std::string)> loadClip, std::function<void(std::string, objid)> loadScript);
void removeSceneFromWorld(World& world, objid sceneId, std::function<void(std::string)> unloadClip, std::function<void(std::string, objid)> unloadScript);
void removeAllScenesFromWorld(World& world, std::function<void(std::string)> unloadClip, std::function<void(std::string, objid)> unloadScript);

objid addObjectToScene(World& world, objid sceneId, std::string name, std::string meshName, glm::vec3 pos, objid id, bool useObjId, std::function<void(std::string)> loadClip, std::function<void(std::string, objid)> loadScript);
objid addObjectToScene(World& world, objid sceneId, std::string serializedObj, objid id, bool useObjId, std::function<void(std::string)> loadClip, std::function<void(std::string, objid)> loadScript);
objid addObjectToScene(World& world, objid sceneId, std::string name, std::map<std::string, std::string> stringAttributes, std::map<std::string, double> numAttributes, std::map<std::string, glm::vec3> vecAttributes, std::function<void(std::string)> loadClip, std::function<void(std::string, objid)> loadScript);

void removeObjectFromScene(World& world, objid id, std::function<void(std::string)> unloadClip, std::function<void(std::string, objid)> unloadScript);

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

void onWorldFrame(World& world, float timestep, bool enablePhysics, bool dumpPhysics);
objid getIdForCollisionObject(World& world,  const btCollisionObject* body);
NameAndMesh getMeshesForGroupId(World& world, objid id);

objid getGameObjectByName(World& world, std::string name);
GameObject& getGameObject(World& world, std::string name);
GameObject& getGameObject(World& world, objid id);
Transformation fullTransformation(World& world, objid id);
objid getGroupId(World& world, objid id);
std::vector<objid> getIdsInGroup(World& world, objid index);

bool idInGroup(World& world, objid id, objid groupId);
bool idExists(World& world, objid id);


std::vector<HitObject> raycast(World& world, glm::vec3 posFrom, glm::quat direction, float maxDistance);

struct LightInfo {
  glm::vec3 pos;
  glm::quat rotation;
  glm::vec3 color;
};

std::vector<LightInfo> getLightInfo(World& world);

void traverseScene(Scene& scene, std::function<void(objid, glm::mat4, glm::mat4, bool)> onObject);
Transformation fullTransformation(Scene& scene, objid id);

#endif

