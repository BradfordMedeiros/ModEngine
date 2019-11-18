#include "./physics.h"

glm::vec3 btToGlm(btVector3 pos){
  return glm::vec3(pos.getX(), pos.getY(), pos.getZ());
}
btVector3 glmToBt(glm::vec3 pos){
  return btVector3(pos.x, pos.y, pos.z);
}
glm::quat btToGlm(btQuaternion rotation){
  return glm::quat(rotation.getW(), rotation.getX(), rotation.getY(), rotation.getZ());
}
btQuaternion glmToBt(glm::quat rotation){
  return btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w);
}

physicsEnv initPhysics(){
  std::cout << "INFO: INIT: physics system" << std::endl;
  auto colConfig = new btDefaultCollisionConfiguration();  
  auto dispatcher = new btCollisionDispatcher(colConfig);
  
  auto broadphase = new btDbvtBroadphase();
  auto btOverlappingPairCallback = new btGhostPairCallback();
  broadphase -> getOverlappingPairCache() -> setInternalGhostPairCallback(btOverlappingPairCallback);

  auto constraintSolver = new btSequentialImpulseConstraintSolver();
  auto dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, constraintSolver, colConfig);

  std::cout << "1: set internal ghost pair callback" << std::endl;
  std::cout << "2: set internal ghost pair callback" << std::endl;

  dynamicsWorld -> setGravity(btVector3(0.f, 9.f, 0.f));

  physicsEnv env = {
    .colConfig = colConfig,
    .dispatcher = dispatcher,
    .broadphase = broadphase,
    .btOverlappingPairs = btOverlappingPairCallback,
    .constraintSolver = constraintSolver,
    .dynamicsWorld = dynamicsWorld,
  };
  return env;
}

//  btGhostObject a;

btRigidBody* createRigidBody(glm::vec3 pos, float width, float height, float depth, glm::quat rot, bool isStatic){
  btTransform transform;
  transform.setIdentity();
  transform.setOrigin(glmToBt(pos));   
  transform.setRotation(glmToBt(rot));

  btScalar mass = isStatic ? btScalar(0.f) : btScalar(1.0f);

  btCollisionShape* shape = new btBoxShape(btVector3(btScalar(1.f / 2 ), btScalar(1.f / 2), btScalar(1.f / 2)));
  shape -> setLocalScaling(btVector3(width, height, depth));

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

void setScale(btRigidBody* body, float width, float height, float depth){
  body -> getCollisionShape() -> setLocalScaling(btVector3(width, height, depth));
}

btGhostObject* createColVol(glm::vec3 pos, float width, float height, float depth){
  btTransform transform;
  transform.setOrigin(glmToBt(pos));
  btGhostObject* obj = new btGhostObject();
  obj -> setCollisionShape(new btBoxShape(btVector3(width, height, depth)));
  obj -> setWorldTransform(transform);
  obj -> setCollisionFlags(obj->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
  return obj;
}
void cleanupColVol(btGhostObject* obj){
  delete obj -> getCollisionShape();
  delete obj;
}
btGhostObject* addCollisionVolume(physicsEnv& env, glm::vec3 pos, float width, float height, float depth){
  std::cout << "adding collision vol: (" << pos.x << " , " << pos.y << " , " << pos.z << ")" << " : " << width << "|" << height << "|" << depth << std::endl;
  auto colVolPtr = createColVol(pos, width, height, depth);
  env.dynamicsWorld -> addCollisionObject(colVolPtr);
  return colVolPtr;
}
void rmColVol(physicsEnv& env, btGhostObject* obj){
  env.dynamicsWorld -> removeCollisionObject(obj);
  cleanupColVol(obj);
}
void checkCollisions(physicsEnv& env, btGhostObject* obj){
  auto numOverlapping = obj -> getNumOverlappingObjects();
  std::cout << "overlapping pairs: " << obj->getOverlappingPairs().size() << std::endl;
  std::cout << "num overlapping is: "  << numOverlapping << std::endl;
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
  delete env.btOverlappingPairs;
  delete env.constraintSolver;
  delete env.broadphase;
  delete env.dispatcher;
  delete env.colConfig;
}
