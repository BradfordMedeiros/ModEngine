#ifndef MOD_PHYSICS
#define MOD_PHYSICS

#include <iostream>
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


#endif 
