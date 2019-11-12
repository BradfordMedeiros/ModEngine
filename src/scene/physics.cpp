#include "./physics.h"

glm::vec3 btToGlm(btVector3 pos){
  return glm::vec3(pos.getX(), pos.getY(), pos.getZ());
}
btVector3 glmToBt(glm::vec3 pos){
  return btVector3(pos.x, pos.y, pos.z);
}
glm::quat btToGlm(btQuaternion rotation){
  return glm::quat(rotation.getX(), rotation.getY(), rotation.getZ(), rotation.getW());
}
btQuaternion glmToBt(glm::quat rotation){
  return btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w);
}

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

btRigidBody* createRigidBody(glm::vec3 pos, float width, float height, float depth, glm::quat rot, bool isStatic){
  btTransform transform;
  transform.setIdentity();
  transform.setOrigin(glmToBt(pos));   
  transform.setRotation(glmToBt(rot));

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

btRigidBody* addRigidBody(physicsEnv& env, glm::vec3 pos, float width, float height, float depth, glm::quat rotation, bool isStatic){  
  auto rigidBodyPtr = createRigidBody(pos, width, height, depth, rotation, isStatic);
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
  return btToGlm(pos);
}
void setPosition(btRigidBody* rigid, glm::vec3 pos){
  btTransform transform; 
  rigid -> getMotionState() -> getWorldTransform(transform);
  transform.setOrigin(glmToBt(pos));
  rigid -> getMotionState() -> setWorldTransform(transform);
  rigid -> setWorldTransform(transform);
}
glm::quat getRotation(btRigidBody* body){
  btTransform transform;
  body -> getMotionState() -> getWorldTransform(transform);
  auto rotation = transform.getRotation();
  return btToGlm(rotation);
}
void setRotation(btRigidBody* body, glm::quat rotation){
  btTransform transform; 
  body -> getMotionState() -> getWorldTransform(transform);
  transform.setRotation(glmToBt(rotation));
  body -> getMotionState() -> setWorldTransform(transform);
  body -> setWorldTransform(transform);
}

// https://stackoverflow.com/questions/11175694/bullet-physics-simplest-collision-example
// https://stackoverflow.com/questions/12251199/re-positioning-a-rigid-body-in-bullet-physics
// https://gamedev.stackexchange.com/questions/22319/how-to-disable-y-axis-movement-in-the-bullet-physics-engine
void stepPhysicsSimulation(physicsEnv& env, float timestep){
  env.dynamicsWorld -> stepSimulation(timestep);
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
