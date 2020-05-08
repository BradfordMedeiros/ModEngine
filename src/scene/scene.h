#ifndef MOD_SCENE
#define MOD_SCENE

#include <iostream>
#include <map>
#include "./scenegraph.h"
#include "./physics.h"
#include "./object_types.h"
#include "./common/mesh.h"
#include "../translations.h"
#include "../common/util.h"

struct World {
  physicsEnv physicsEnvironment;
  std::map<short, btRigidBody*> rigidbodys;
  std::map<short, GameObjectObj> objectMapping;
  std::map<std::string, Mesh> meshes;
  std::map<short, std::vector<Animation>> animations;
  std::map<short, Scene> scenes;
  std::map<short, short> idToScene;
  std::map<std::string, std::map<std::string, std::string>> meshnameToBoneToParent;
};

struct PhysicsInfo {
  BoundInfo boundInfo;
  GameObject gameobject;
  glm::vec3 collisionInfo;
};

World createWorld(collisionPairFn onObjectEnter, collisionPairFn onObjectLeave, btIDebugDraw* debugDrawer);
short addSceneToWorld(World& world, std::string sceneFile, std::function<void(std::string)> loadClip, std::function<void(std::string, short)> loadScript);
void removeSceneFromWorld(World& world, short sceneId, std::function<void(std::string)> unloadClip);
void addObject(World& world, short sceneId, std::string name, std::string meshName, glm::vec3 pos, std::function<void(std::string)> loadClip, std::function<void(std::string, short)> loadScript);
void removeObject(World& world, short id, std::function<void(std::string)> unloadClip);

std::string serializeFullScene(Scene& scene, std::map<short, GameObjectObj> objectMapping);

void physicsTranslate(World& world, short index, float x, float y, float z, bool moveRelativeEnabled);
void  physicsTranslateSet(World& world, short index, glm::vec3 pos);
void  physicsRotate(World& world, short index, float x, float y, float z);
void  physicsRotateSet(World& world, short index, glm::quat rotation);  // this sets to rotation
void  physicsScale(World& world, short index, float x, float y, float z);
void  applyPhysicsTranslation(Scene& scene, btRigidBody* body, short index, glm::vec3 position, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis);
void  applyPhysicsRotation(Scene& scene, btRigidBody* body, short index, glm::quat currentOrientation, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis);
void  applyPhysicsScaling(World& world, Scene& scene, btRigidBody* body, short index, glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis);
void  onWorldFrame(World& world, float timestep, bool enablePhysics, bool dumpPhysics);
short getIdForCollisionObject(World& world,  const btCollisionObject* body);
NameAndMesh getMeshesForGroupId(World& world, short id);

std::map<std::string, std::string> getAttributes(World& world, short id);
void setAttributes(World& world, short id, std::map<std::string, std::string> attr);

std::string scenegraphAsDotFormat(Scene& scene, std::map<short, GameObjectObj>& objectMapping);


#endif

