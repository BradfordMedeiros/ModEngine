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
  std::map<std::string, std::vector<Animation>> animations;
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
short addSceneToWorld(World& world, std::string sceneFile);
void removeSceneFromWorld(World& world, short sceneId);

std::string serializeFullScene(Scene& scene, std::map<short, GameObjectObj> objectMapping);

void  addObjectToFullScene(World& world, short sceneId, std::string name, std::string meshName, glm::vec3 pos);
void  physicsTranslate(Scene& scene, btRigidBody* body, float x, float y, float z, bool moveRelativeEnabled, short index);
void  physicsTranslateSet(Scene& scene, btRigidBody* body, glm::vec3 pos, short index);
void  physicsRotate(Scene& scene, btRigidBody* body, float x, float y, float z, short index);
void  physicsRotateSet(Scene& scene, btRigidBody* body, glm::quat rotation, short index);  // this sets to rotation
void  physicsScale(World& world, Scene& scene, btRigidBody* body, short index, float x, float y, float z);
void  applyPhysicsTranslation(Scene& scene, btRigidBody* body, short index, glm::vec3 position, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis);
void  applyPhysicsRotation(Scene& scene, btRigidBody* body, short index, glm::quat currentOrientation, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis);
void  applyPhysicsScaling(World& world, Scene& scene, btRigidBody* body, short index, glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis);
void  onPhysicsFrame(World& world, float timestep, bool dumpPhysics);
short getIdForCollisionObject(World& world,  const btCollisionObject* body);

std::string scenegraphAsDotFormat(Scene& scene, std::map<short, GameObjectObj>& objectMapping);


#endif

