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

physicsEnv initPhysics(collisionPairFn onObjectEnter,  collisionPairFn onObjectLeave, btIDebugDraw* debugDrawer){
  std::cout << "INFO: INIT: physics system" << std::endl;
  auto colConfig = new btDefaultCollisionConfiguration();  
  auto dispatcher = new btCollisionDispatcher(colConfig);  
  auto broadphase = new btDbvtBroadphase();
  auto constraintSolver = new btSequentialImpulseConstraintSolver();
  auto dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, constraintSolver, colConfig);

  dynamicsWorld -> setGravity(btVector3(0.f, 9.81f, 0.f));

  CollisionCache collisionCache(onObjectEnter, onObjectLeave);

  physicsEnv env = {
    .colConfig = colConfig,
    .dispatcher = dispatcher,
    .broadphase = broadphase,
    .constraintSolver = constraintSolver,
    .dynamicsWorld = dynamicsWorld,
    .collisionCache = collisionCache,
    .hasDebugDrawer = debugDrawer != NULL
  };

  if (env.hasDebugDrawer){
    env.dynamicsWorld -> setDebugDrawer(debugDrawer);
  }
  return env;
}

btRigidBody* createRigidBody(glm::vec3 pos, btCollisionShape* shape, glm::quat rot, bool isStatic, bool hasCollision){
  btTransform transform;
  transform.setIdentity();
  transform.setOrigin(glmToBt(pos));   
  transform.setRotation(glmToBt(rot));

  btScalar mass = isStatic ? btScalar(0.f) : btScalar(1.0f);
  btDefaultMotionState* motionState = new btDefaultMotionState(transform);

  btVector3 inertia(0, 0, 0);
  if (!isStatic){
    shape -> calculateLocalInertia(mass, inertia);
  }

  auto constructionInfo = btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, inertia);
  constructionInfo.m_friction = 1.0f;

  auto body  = new btRigidBody(constructionInfo);
  if (!hasCollision){
    body -> setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
  }
  if (isStatic){
    body -> setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    body -> setActivationState(DISABLE_DEACTIVATION);
  }

  return body;
}
btRigidBody* createRigidBodyRect(glm::vec3 pos, float width, float height, float depth, glm::quat rot, bool isStatic, bool hasCollision){
  btCollisionShape* shape = new btBoxShape(btVector3(btScalar(1.f / 2 ), btScalar(1.f / 2), btScalar(1.f / 2)));
  shape -> setLocalScaling(btVector3(width, height, depth));
  return createRigidBody(pos, shape, rot, isStatic, hasCollision);
}
btRigidBody* createRigidBodySphere(glm::vec3 pos, float radius, glm::quat rot, bool isStatic, bool hasCollision){
  btCollisionShape* shape = new btSphereShape(1);
  shape -> setLocalScaling(btVector3(radius, radius, radius));
  return createRigidBody(pos, shape, rot, isStatic, hasCollision);
}
btRigidBody* createRigidBodyCompound(glm::vec3 pos, glm::quat rotation, std::vector<VoxelBody> bodies, bool isStatic, bool hasCollision){
  btCompoundShape* shape = new btCompoundShape();
  for (auto body: bodies){
    btCollisionShape* cshape1 = new btBoxShape(btVector3(btScalar(body.scale.x / 2 ), btScalar(body.scale.y / 2), btScalar(body.scale.z / 2)));
    btTransform position;
    position.setOrigin(glmToBt(body.position));
    shape -> addChildShape(position, cshape1);
  }
  shape -> setLocalScaling(btVector3(1, 1, 1));
  return createRigidBody(pos, shape, rotation, isStatic, hasCollision);
}

void cleanupRigidBody(btRigidBody* body){
  delete body -> getMotionState();
  btCollisionShape* shape = body -> getCollisionShape(); 

  btCompoundShape* cshape = dynamic_cast<btCompoundShape*>(shape);      // @TODO verify this
  if (cshape != NULL){
    int numChildren = cshape -> getNumChildShapes();
    for (int i = 0; i < numChildren; i++){
      btCollisionShape * childShape = cshape -> getChildShape(i);
      delete childShape;
    }
  }else{
    delete shape;
  }
  delete body;
}

btRigidBody* addRigidBody(physicsEnv& env, glm::vec3 pos, float width, float height, float depth, glm::quat rotation, bool isStatic, bool hasCollision){  
  auto rigidBodyPtr = createRigidBodyRect(pos, width, height, depth, rotation, isStatic, hasCollision);
  env.dynamicsWorld -> addRigidBody(rigidBodyPtr);
  return rigidBodyPtr;
}
btRigidBody* addRigidBody(physicsEnv& env, glm::vec3 pos, float radius, glm::quat rotation, bool isStatic, bool hasCollision){
  auto rigidBodyPtr = createRigidBodySphere(pos, radius, rotation, isStatic, hasCollision);
  env.dynamicsWorld -> addRigidBody(rigidBodyPtr);
  return rigidBodyPtr;
}
btRigidBody* addRigidBody(physicsEnv& env, glm::vec3 pos, glm::quat rotation, std::vector<VoxelBody> bodies, bool isStatic, bool hasCollision){
  auto rigidBodyPtr = createRigidBodyCompound(pos, rotation, bodies, isStatic, hasCollision);
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
    
void checkCollisions(physicsEnv& env){   
  auto dispatcher = env.dynamicsWorld -> getDispatcher();
  std::vector<std::pair<const btCollisionObject*, const btCollisionObject*>> collisionPairs;
  for (int i = 0; i < dispatcher -> getNumManifolds(); i++) {
    btPersistentManifold* contactManifold = dispatcher -> getManifoldByIndexInternal(i);
    collisionPairs.push_back(std::make_pair(contactManifold -> getBody0(), contactManifold -> getBody1()));
  } 
  env.collisionCache.onObjectsCollide(collisionPairs);
}

void stepPhysicsSimulation(physicsEnv& env, float timestep){
  env.dynamicsWorld -> stepSimulation(timestep, 10);
  checkCollisions(env);
  if (env.hasDebugDrawer){
    env.dynamicsWorld -> debugDrawWorld();
  }
}
void printRigidBodyInfo(btRigidBody* body){
  glm::vec3 origin = getPosition(body);
  std::cout << "position: (" << origin.x << " , " << origin.y << " , " << origin.z << " )" << std::endl;
}

void applyImpulse(btRigidBody* body, glm::vec3 force){
  body -> applyCentralImpulse(btVector3(force.x, force.y, force.z));
}
void clearImpulse(btRigidBody* body){
  body -> applyCentralImpulse(btVector3(0.f, 0.f, 0.f));
}

void deinitPhysics(physicsEnv env){   // @todo maybe clean up rigid bodies too but maybe not
  std::cout << "INFO: DEINIT: physics system" << std::endl;
  delete env.dynamicsWorld;
  delete env.constraintSolver;
  delete env.broadphase;
  delete env.dispatcher;
  delete env.colConfig;
}
