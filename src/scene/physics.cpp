#include "./physics.h"

physicsEnv initPhysics(){
  std::cout << "INFO: INIT: physics system" << std::endl;
  auto colConfig = new btDefaultCollisionConfiguration();  
  auto dispatcher = new btCollisionDispatcher(colConfig);
  auto broadphase = new btDbvtBroadphase();
  auto constraintSolver = new btSequentialImpulseConstraintSolver();
  auto dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, constraintSolver, colConfig);

  dynamicsWorld -> setGravity(btVector3(0.f, 9.f, 0.f));

  physicsEnv env = {
    .colConfig = colConfig,
    .dispatcher = dispatcher,
    .broadphase = broadphase,
    .constraintSolver = constraintSolver,
    .dynamicsWorld = dynamicsWorld,
  };
  return env;
}

btRigidBody* createRigidBody(float x, float y, float z, float width, float height, float depth, glm::quat rot, bool isStatic){
  btTransform transform;
  transform.setIdentity();
  transform.setOrigin(btVector3(x, y, z));   
  transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

  btScalar mass = isStatic ? btScalar(0.f) : btScalar(1.0f);

  btCollisionShape* shape = new btBoxShape(btVector3(btScalar(width / 2 ), btScalar(height / 2), btScalar(depth / 2)));
  btDefaultMotionState* motionState = new btDefaultMotionState(transform);

  btVector3 inertia(0, 0, 0);
  if (!isStatic){
    shape -> calculateLocalInertia(mass, inertia);
  }

  auto constructionInfo = btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, inertia);
  constructionInfo.m_friction = 1.0f;

  auto body  = new btRigidBody(constructionInfo);
  if (isStatic){
    body -> setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    body -> setActivationState(DISABLE_DEACTIVATION);
  }

  return body;
}
void cleanupRigidBody(btRigidBody* body){
  delete body -> getMotionState();
  delete body -> getCollisionShape();
  delete body;
}

btRigidBody* addRigidBody(physicsEnv& env, float x, float y, float z, float width, float height, float depth, glm::quat rotation, bool isStatic){  
  auto rigidBodyPtr = createRigidBody(x, y, z, width, height, depth, rotation, isStatic);
  env.dynamicsWorld -> addRigidBody(rigidBodyPtr);
  return rigidBodyPtr;
}
void rmRigidBody(physicsEnv& env, btRigidBody* body){
  env.dynamicsWorld -> removeRigidBody(body);
  cleanupRigidBody(body);
}
glm::vec3 getPosition(btRigidBody* body){
  btTransform transform; 
  body -> getMotionState() -> getWorldTransform(transform);
  auto pos = transform.getOrigin();
  return glm::vec3(pos.getX(), pos.getY(), pos.getZ());
}
void setPosition(btRigidBody* rigid, float x, float y, float z){
  btTransform transform; 
  rigid -> getMotionState() -> getWorldTransform(transform);
  transform.setOrigin(btVector3(x, y, z));
  rigid -> getMotionState() -> setWorldTransform(transform);
  rigid -> setWorldTransform(transform);
}
glm::quat getRotation(btRigidBody* body){
  btTransform transform;
  body -> getMotionState() -> getWorldTransform(transform);
  auto rotation = transform.getRotation();
  return glm::quat(rotation.getX(), rotation.getY(), rotation.getZ(), rotation.getW());
}
void setRotation(btRigidBody* body, glm::quat rotation){

}

// https://stackoverflow.com/questions/11175694/bullet-physics-simplest-collision-example
// https://stackoverflow.com/questions/12251199/re-positioning-a-rigid-body-in-bullet-physics
// https://gamedev.stackexchange.com/questions/22319/how-to-disable-y-axis-movement-in-the-bullet-physics-engine
void stepPhysicsSimulation(physicsEnv& env, float timestep){
  env.dynamicsWorld -> stepSimulation(timestep);

  // todo detect collision here
}
void printRigidBodyInfo(btRigidBody* body){
  glm::vec3 origin = getPosition(body);
  std::cout << "position: (" << origin.x << " , " << origin.y << " , " << origin.z << " )" << std::endl;
}

void deinitPhysics(physicsEnv env){   // @todo maybe clean up rigid bodies too but maybe not
  std::cout << "INFO: DEINIT: physics system" << std::endl;
  delete env.dynamicsWorld;
  delete env.constraintSolver;
  delete env.broadphase;
  delete env.dispatcher;
  delete env.colConfig;
}
