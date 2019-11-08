#ifndef MOD_PHYSICS
#define MOD_PHYSICS

#include <iostream>
#include <vector>
#include <bullet/btBulletDynamicsCommon.h>

struct physicsEnv {
  btDefaultCollisionConfiguration* colConfig;
  btCollisionDispatcher* dispatcher;
  btDbvtBroadphase* broadphase;
  btSequentialImpulseConstraintSolver* constraintSolver;
  btDiscreteDynamicsWorld* dynamicsWorld;
};

physicsEnv initPhysics();
void deinitPhysics(physicsEnv env);
void stepPhysicsSimulation(physicsEnv& env, float timestep);
btRigidBody* addRigidBody(physicsEnv& env, float x, float y, float z, float width, float height, float depth, bool isStatic);
void rmRigidBody(physicsEnv& env, btRigidBody* body);
void setPosition(btRigidBody* body, float x, float y, float z);
btVector3 getPosition(btRigidBody* rigidbody);

void addColCol();
void rmColVol();

void printRigidBodyInfo(btRigidBody* body);

#endif 
