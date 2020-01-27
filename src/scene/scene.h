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

struct FullScene {    // @todo probably get rid of fullscene.  This use to have more stuff moved to world, so now this is obsolete (for now). 
  Scene scene;
};

struct World {
  physicsEnv physicsEnvironment;
  std::map<short, btRigidBody*> rigidbodys;
  std::map<short, GameObjectObj> objectMapping;
  std::map<std::string, Mesh> meshes;
  std::map<short, FullScene> scenes;
  std::map<short, short> idToScene;
};

struct PhysicsInfo {
  BoundInfo boundInfo;
  GameObject gameobject;
  glm::vec3 collisionInfo;
};

World createWorld(collisionPairFn onObjectEnter, collisionPairFn onObjectLeave, btIDebugDraw* debugDrawer);
short addSceneToWorld(World& world, std::string sceneFile);
void removeSceneFromWorld(World& world, short sceneId);

FullScene   deserializeFullScene(World& world, std::string sceneFile);
std::string serializeFullScene(Scene& scene, std::map<short, GameObjectObj> objectMapping);

void  addObjectToFullScene(World& world, short sceneId, std::string name, std::string meshName, glm::vec3 pos);
void  physicsTranslate(FullScene& fullscene, btRigidBody* body, float x, float y, float z, bool moveRelativeEnabled, short index);
void  physicsTranslateSet(FullScene& fullScene, btRigidBody* body, glm::vec3 pos, short index);
void  physicsRotate(FullScene& fullscene, btRigidBody* body, float x, float y, float z, short index);
void  physicsRotateSet(FullScene& fullscene, btRigidBody* body, glm::quat rotation, short index);  // this sets to rotation
void  physicsScale(World& world, FullScene& fullscene, btRigidBody* body, short index, float x, float y, float z);
void  applyPhysicsTranslation(FullScene& scene, btRigidBody* body, short index, glm::vec3 position, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis);
void  applyPhysicsRotation(FullScene& scene, btRigidBody* body, short index, glm::quat currentOrientation, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis);
void  applyPhysicsScaling(World& world, FullScene& scene, btRigidBody* body, short index, glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, ManipulatorAxis manipulatorAxis);
void  onPhysicsFrame(World& world, float timestep, bool dumpPhysics);
short getIdForCollisionObject(World& world,  const btCollisionObject* body);

#endif

