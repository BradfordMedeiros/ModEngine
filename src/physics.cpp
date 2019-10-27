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

void addRigidBody(physicsEnv& env, float x, float y, float z){  // @todo obviously this is managed improperly WIP
  btBoxShape box(btVector3(btScalar(10.0f), btScalar(10.0f), btScalar(10.0f)));
  btCollisionShape* shape = &box;
  
  btScalar mass(1.0f);

  btTransform transform;
  transform.setIdentity();
  transform.setOrigin(btVector3(x, y, z));   
  btDefaultMotionState motionState(transform);

  btVector3 inertia(0, 0, 0);
  box.calculateLocalInertia(mass, inertia);

  btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, &motionState, shape, inertia);
  btRigidBody body(rbInfo);
  env.dynamicsWorld -> addRigidBody(&body);
}
void rmRigidBody(physicsEnv& env){

}

void deinitPhysics(physicsEnv env){
  std::cout << "INFO: DEINIT: physics system" << std::endl;
  delete env.colConfig;
  delete env.dispatcher;
  delete env.broadphase;
  delete env.constraintSolver;
  delete env.dynamicsWorld;
}
