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

btRigidBody* createRigidBody(float x, float y, float z){
  btTransform transform;
  transform.setIdentity();
  transform.setOrigin(btVector3(x, y, z));   

  btScalar mass(1.0f);

  btCollisionShape* shape = new btBoxShape(btVector3(btScalar(10.0f), btScalar(10.0f), btScalar(10.0f)));
  btDefaultMotionState* motionState = new btDefaultMotionState(transform);

  btVector3 inertia(0, 0, 0);
  shape -> calculateLocalInertia(mass, inertia);

  auto rigidBodyPtr = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, inertia));
  return rigidBodyPtr;
}

btRigidBody* addRigidBody(physicsEnv& env, float x, float y, float z){  // @todo obviously this is managed improperly WIP
  auto rigidBodyPtr = createRigidBody(x, y, z);
  env.dynamicsWorld -> addRigidBody(rigidBodyPtr);
  return rigidBodyPtr;
}

void rmRigidBody(physicsEnv& env, btRigidBody* body){
  env.dynamicsWorld -> removeRigidBody(body);
  delete body -> getMotionState();
  delete body -> getCollisionShape();
  delete body;
}

void deinitPhysics(physicsEnv env){   // @todo maybe clean up rigid bodies too but maybe not
  std::cout << "INFO: DEINIT: physics system" << std::endl;
  delete env.dynamicsWorld;
  delete env.constraintSolver;
  delete env.broadphase;
  delete env.dispatcher;
  delete env.colConfig;
}
