#ifndef MOD_SCENE
#define MOD_SCENE

#include <iostream>
#include <map>
#include "./scenegraph.h"
#include "./physics.h"
#include "./object_types.h"
#include "./common/mesh.h"
#include "../translations.h"

struct FullScene {
  Scene scene;
  std::map<std::string, Mesh> meshes;
  std::map<short, GameObjectObj> objectMapping;
};

struct PhysicsInfo {
  BoundInfo boundInfo;
  GameObject gameobject;
  glm::vec3 collisionInfo;
};

FullScene deserializeFullScene(std::string content);
std::string serializeFullScene(Scene& scene, std::map<short, GameObjectObj> objectMapping);
void addObjectToFullScene(FullScene& scene, std::string name, std::string meshName, glm::vec3 pos);
PhysicsInfo getPhysicsInfoForGameObject(FullScene& fullscene, short index);
void physicsTranslate(FullScene& fullscene, btRigidBody* body, float x, float y, float z, bool moveRelativeEnabled, short index);
void physicsRotate(FullScene& fullscene, btRigidBody* body, float x, float y, float z, short index);
void physicsScale(physicsEnv& physicsEnv, FullScene& fullscene, btRigidBody* body, short index, float x, float y, float z);

#endif

