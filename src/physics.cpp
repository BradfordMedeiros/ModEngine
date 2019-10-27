#include "./physics.h"

physicsEnv initPhysics(){
  std::cout << "INFO: INIT: physics system" << std::endl;
  auto colConfig = new btDefaultCollisionConfiguration();  
  auto dispatcher = new btCollisionDispatcher(colConfig);
  auto broadphase = new btDbvtBroadphase();
  auto constraintSolver = new btSequentialImpulseConstraintSolver();
  auto dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, constraintSolver, colConfig);

  physicsEnv env = {
    .colConfig = colConfig,
    .dispatcher = dispatcher,
    .broadphase = broadphase,
    .constraintSolver = constraintSolver,
    .dynamicsWorld = dynamicsWorld,
  };
  return env;
}

void deinitPhysics(physicsEnv env){
  std::cout << "INFO: DEINIT: physics system" << std::endl;
  delete env.colConfig;
  delete env.dispatcher;
  delete env.broadphase;
  delete env.constraintSolver;
  delete env.dynamicsWorld;
}

