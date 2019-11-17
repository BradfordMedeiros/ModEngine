#ifndef MOD_PHYSICS
#define MOD_PHYSICS

#include <iostream>
#include <vector>
#include <glm/gtc/quaternion.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>

struct physicsEnv {
  btDefaultCollisionConfiguration* colConfig;
  btCollisionDispatcher* dispatcher;
  btDbvtBroadphase* broadphase;
  btSequentialImpulseConstraintSolver* constraintSolver;
  btOverlappingPairCallback* btOverlappingPairs;
  btDiscreteDynamicsWorld* dynamicsWorld;
};

physicsEnv initPhysics();
void deinitPhysics(physicsEnv env);
void stepPhysicsSimulation(physicsEnv& env, float timestep);
btRigidBody* addRigidBody(physicsEnv& env, glm::vec3 pos, float width, float height, float depth, glm::quat rotation, bool isStatic);
void rmRigidBody(physicsEnv& env, btRigidBody* body);
void setPosition(btRigidBody* body, glm::vec3);
glm::vec3 getPosition(btRigidBody* rigidbody);
void setRotation(btRigidBody* body, glm::quat rotation);
glm::quat getRotation(btRigidBody* body);
void setScale(btRigidBody* body, float width, float height, float depth);

btGhostObject* addColVol(physicsEnv& env, glm::vec3 pos, float width, float height, float depth);
void rmColVol(physicsEnv& env, btGhostObject* obj);

void printRigidBodyInfo(btRigidBody* body);

#endif 
