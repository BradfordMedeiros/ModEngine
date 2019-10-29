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
void stepSimulation(physicsEnv& env);
btRigidBody* addRigidBody(physicsEnv& env, float x, float y, float z);
void rmRigidBody(physicsEnv& env, btRigidBody* body);
void addColCol();
void rmColVol();

#endif 
