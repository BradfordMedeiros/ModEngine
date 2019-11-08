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

btRigidBody* createRigidBody(float x, float y, float z, float width, float height, float depth, bool isStatic){
  btTransform transform;
  transform.setIdentity();
  transform.setOrigin(btVector3(x, y, z));   

  btScalar mass = isStatic ? btScalar(0.f) : btScalar(1.0f);

  btCollisionShape* shape = new btBoxShape(btVector3(btScalar(width / 2 ), btScalar(height / 2), btScalar(depth / 2)));
  btDefaultMotionState* motionState = new btDefaultMotionState(transform);

  btVector3 inertia(0, 0, 0);
  if (!isStatic){
    shape -> calculateLocalInertia(mass, inertia);
  }

  auto rigidBodyPtr = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, inertia));
  return rigidBodyPtr;
}
void cleanupRigidBody(btRigidBody* body){
  delete body -> getMotionState();
  delete body -> getCollisionShape();
  delete body;
}

btRigidBody* addRigidBody(physicsEnv& env, float x, float y, float z, float width, float height, float depth, bool isStatic){  
  auto rigidBodyPtr = createRigidBody(x, y, z, width, height, depth, isStatic);
  env.dynamicsWorld -> addRigidBody(rigidBodyPtr);
  rigidBodyPtr -> setGravity(btVector3(0, -1, 0));

  return rigidBodyPtr;
}
void rmRigidBody(physicsEnv& env, btRigidBody* body){
  env.dynamicsWorld -> removeRigidBody(body);
  cleanupRigidBody(body);
}
btVector3 getPosition(btRigidBody* body){
  btTransform transform; 
  body -> getMotionState() -> getWorldTransform(transform);
  auto origin = transform.getOrigin();
  return origin;
}
void setPosition(btRigidBody* rigid, float x, float y, float z){
  std::cout << "SETTING PHYSICS position: " << x << " , " << y << " , " << z << std::endl;
  btTransform transform; 
  rigid -> getMotionState() -> getWorldTransform(transform);
  transform.setOrigin(btVector3(x, y, z));
  rigid -> getMotionState() -> setWorldTransform(transform);

}

// https://stackoverflow.com/questions/11175694/bullet-physics-simplest-collision-example
// https://stackoverflow.com/questions/12251199/re-positioning-a-rigid-body-in-bullet-physics
// https://gamedev.stackexchange.com/questions/22319/how-to-disable-y-axis-movement-in-the-bullet-physics-engine
void stepPhysicsSimulation(physicsEnv& env, float timestep){
  env.dynamicsWorld -> stepSimulation(timestep);

  // todo detect collision here
}
void printRigidBodyInfo(btRigidBody* body){
  btVector3 origin = getPosition(body);
  std::cout << "position: (" << origin.getX() << " , " << origin.getY() << " , " << origin.getZ() << " )" << std::endl;
}

void deinitPhysics(physicsEnv env){   // @todo maybe clean up rigid bodies too but maybe not
  std::cout << "INFO: DEINIT: physics system" << std::endl;
  delete env.dynamicsWorld;
  delete env.constraintSolver;
  delete env.broadphase;
  delete env.dispatcher;
  delete env.colConfig;
}
