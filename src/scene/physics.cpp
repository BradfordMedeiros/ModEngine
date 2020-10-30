#include "./physics.h"

physicsEnv initPhysics(collisionPairPosFn onObjectEnter,  collisionPairFn onObjectLeave, btIDebugDraw* debugDrawer){
  std::cout << "INFO: INIT: physics system" << std::endl;
  auto colConfig = new btDefaultCollisionConfiguration();  
  auto dispatcher = new btCollisionDispatcher(colConfig);  
  auto broadphase = new btDbvtBroadphase();
  auto constraintSolver = new btSequentialImpulseConstraintSolver();
  auto dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, constraintSolver, colConfig);

  dynamicsWorld -> setGravity(btVector3(0.f, -9.81f, 0.f));

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

btRigidBody* createRigidBody(glm::vec3 pos, btCollisionShape* shape, glm::quat rot, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  btTransform transform;
  transform.setIdentity();
  transform.setOrigin(glmToBt(pos));   
  transform.setRotation(glmToBt(rot));

  btScalar mass = isStatic ? btScalar(0.f) : btScalar(opts.mass);
  btDefaultMotionState* motionState = new btDefaultMotionState(transform);

  btVector3 inertia(0, 0, 0);
  if (!isStatic){
    shape -> calculateLocalInertia(mass, inertia);
  }
  shape -> setLocalScaling(glmToBt(scaling));

  auto constructionInfo = btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, inertia);
  constructionInfo.m_friction = opts.friction;
  constructionInfo.m_restitution = opts.restitution;

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
btRigidBody* createRigidBodyRect(glm::vec3 pos, float width, float height, float depth, glm::quat rot, bool isStatic, bool hasCollision, bool isCentered, glm::vec3 scaling, rigidBodyOpts opts){
  if (!isCentered){
    btCompoundShape* shape = new btCompoundShape();
    btCollisionShape* cshape1 = new btBoxShape(btVector3(btScalar(width / 2.f), btScalar(height / 2.f), btScalar(depth / 2.f)));
    cshape1 -> setLocalScaling(btVector3(1, 1, 1));
    btTransform position;
    position.setIdentity();
    position.setOrigin(btVector3(width / 2.f, height / 2.f, depth / 2.f));
    shape -> addChildShape(position, cshape1);
    shape -> setLocalScaling(btVector3(1, 1, 1));
    return createRigidBody(pos, shape, rot, isStatic, hasCollision, scaling, opts);
  }

  btCollisionShape* shape = new btBoxShape(btVector3(btScalar(width * 1.f / 2 ), btScalar(height * 1.f / 2), btScalar(depth * 1.f / 2)));
  return createRigidBody(pos, shape, rot, isStatic, hasCollision, scaling, opts);
}
btRigidBody* createRigidBodySphere(glm::vec3 pos, float radius, glm::quat rot, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  btCollisionShape* shape = new btSphereShape(radius); 
  return createRigidBody(pos, shape, rot, isStatic, hasCollision, scaling, opts);
}
btRigidBody* createRigidBodyCompound(glm::vec3 pos, glm::quat rotation, std::vector<VoxelBody> bodies, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  btCompoundShape* shape = new btCompoundShape();
  for (auto body: bodies){
    btCollisionShape* cshape1 = new btBoxShape(btVector3(btScalar(0.5f), btScalar(0.5f), btScalar(0.5f)));
    btTransform position;
    position.setIdentity();
    position.setOrigin(glmToBt(body.position + glm::vec3(0.5f, 0.5f, 0.5f)));
    shape -> addChildShape(position, cshape1);
  }
  return createRigidBody(pos, shape, rotation, isStatic, hasCollision, scaling, opts);
}

float MIN_HEIGHTMAP_HEIGHT = -2000.f;
float MAX_HEIGHTMAP_HEIGHT =  2000.f;
btRigidBody* createRigidBodyHeightmap(glm::vec3 pos, glm::quat rotation, bool isStatic, rigidBodyOpts opts, float* data, int width, int height, glm::vec3 scaling, float minHeight, float maxHeight){
  assert(minHeight >= MIN_HEIGHTMAP_HEIGHT);
  assert(maxHeight <= MAX_HEIGHTMAP_HEIGHT); 
  btHeightfieldTerrainShape * shape = new btHeightfieldTerrainShape(width, height, data, 1, MIN_HEIGHTMAP_HEIGHT, MAX_HEIGHTMAP_HEIGHT, 1, PHY_FLOAT, false);
  return createRigidBody(pos, shape, rotation, isStatic, true, scaling, opts);
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

// Due to bullet weirdness, this seems like it has to be called after adding rigid body to world (for the gravity part)
void setPhysicsOptions(btRigidBody* body, glm::vec3 linear, glm::vec3 angular,  glm::vec3 gravity){
  body->setLinearFactor(glmToBt(linear));
  body->setAngularFactor(glmToBt(angular));
  body -> setGravity(glmToBt(gravity));
}
btRigidBody* addRigidBodyRect(physicsEnv& env, glm::vec3 pos, float width, float height, float depth, glm::quat rotation, bool isStatic, bool hasCollision, bool isCentered, glm::vec3 scaling, rigidBodyOpts opts){  
  auto rigidBodyPtr = createRigidBodyRect(pos, width, height, depth, rotation, isStatic, hasCollision, isCentered, scaling, opts);
  env.dynamicsWorld -> addRigidBody(rigidBodyPtr);
  setPhysicsOptions(rigidBodyPtr, opts.linear, opts.angular, opts.gravity);
  return rigidBodyPtr;
}
btRigidBody* addRigidBodySphere(physicsEnv& env, glm::vec3 pos, float radius, glm::quat rotation, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  auto rigidBodyPtr = createRigidBodySphere(pos, radius, rotation, isStatic, hasCollision, scaling, opts);
  env.dynamicsWorld -> addRigidBody(rigidBodyPtr);
  setPhysicsOptions(rigidBodyPtr, opts.linear, opts.angular, opts.gravity);
  return rigidBodyPtr;
}
btRigidBody* addRigidBodyVoxel(physicsEnv& env, glm::vec3 pos, glm::quat rotation, std::vector<VoxelBody> bodies, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  auto rigidBodyPtr = createRigidBodyCompound(pos, rotation, bodies, isStatic, hasCollision, scaling, opts);
  env.dynamicsWorld -> addRigidBody(rigidBodyPtr);
  setPhysicsOptions(rigidBodyPtr, opts.linear, opts.angular, opts.gravity);
  return rigidBodyPtr;
}

btRigidBody* addRigidBodyHeightmap(physicsEnv& env, glm::vec3 pos, glm::quat rotation, bool isStatic, rigidBodyOpts opts, float* data, int width, int height, glm::vec3 scaling, float minHeight, float maxHeight){
  auto rigidBodyPtr = createRigidBodyHeightmap(pos, rotation, isStatic, opts, data, width, height, scaling, minHeight, maxHeight);
  env.dynamicsWorld -> addRigidBody(rigidBodyPtr);
  setPhysicsOptions(rigidBodyPtr, opts.linear, opts.angular, opts.gravity);
  return rigidBodyPtr;
}

void rmRigidBody(physicsEnv& env, btRigidBody* body){
  env.dynamicsWorld -> removeRigidBody(body);
  cleanupRigidBody(body);
}

glm::vec3 getPosition(btRigidBody* body){
  return btToGlm(body -> getWorldTransform().getOrigin());
}
void setPosition(btRigidBody* rigid, glm::vec3 pos){
  btTransform transform; 
  rigid -> getMotionState() -> getWorldTransform(transform);
  transform.setOrigin(glmToBt(pos));
  rigid -> getMotionState() -> setWorldTransform(transform);
  rigid -> setWorldTransform(transform);
}
glm::quat getRotation(btRigidBody* body){
  return btToGlm(body -> getWorldTransform().getRotation());
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
glm::vec3 getScale(btRigidBody* body){
  return btToGlm(body -> getCollisionShape() -> getLocalScaling());
}
    
void checkCollisions(physicsEnv& env){   
  auto dispatcher = env.dynamicsWorld -> getDispatcher();
  std::vector<CollisionInstance> collisionPairs;
  for (int i = 0; i < dispatcher -> getNumManifolds(); i++) {
    btPersistentManifold* contactManifold = dispatcher -> getManifoldByIndexInternal(i);

    if (contactManifold -> getNumContacts() > 0){
      /// @NOTE Internally there can be more than one contact point, but it's simpler to just report one of them (maybe expose more in future)
      btManifoldPoint& point = contactManifold -> getContactPoint(0);  
      btVector3 median = (point.getPositionWorldOnA() + point.getPositionWorldOnB()) / 2.f;

      collisionPairs.push_back(    CollisionInstance {
        .obj1 = contactManifold -> getBody0(),
        .obj2 = contactManifold -> getBody1(),
        .pos = btToGlm(median),
      });     
    }
  } 
  env.collisionCache.onObjectsCollide(collisionPairs);
}

void stepPhysicsSimulation(physicsEnv& env, float timestep){
  env.dynamicsWorld -> stepSimulation(timestep, 0);   // TODO revisit # of substeps
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

void clampMaxVelocity(btRigidBody* body, float maxspeed){
  if (maxspeed <= 0){
    return;
  }
  auto velocity = body -> getLinearVelocity();
  auto xVelocity = fmax(-maxspeed, fmin(maxspeed, velocity.getX()));
  auto yVelocity = fmax(-maxspeed, fmin(maxspeed, velocity.getY()));
  auto zVelocity = fmax(-maxspeed, fmin(maxspeed, velocity.getZ()));
  body -> setLinearVelocity(btVector3(xVelocity, yVelocity, zVelocity));
}

void deinitPhysics(physicsEnv env){   // @todo maybe clean up rigid bodies too but maybe not
  std::cout << "INFO: DEINIT: physics system" << std::endl;
  delete env.dynamicsWorld;
  delete env.constraintSolver;
  delete env.broadphase;
  delete env.dispatcher;
  delete env.colConfig;
}

std::vector<HitObject> raycast(physicsEnv& env, std::map<objid, btRigidBody*>& rigidbodys, glm::vec3 posFrom, glm::quat direction, float maxDistance){
  std::vector<HitObject> hitobjects;
  btCollisionWorld::AllHitsRayResultCallback result(glmToBt(posFrom),glmToBt(posFrom));
  auto posTo = moveRelative(posFrom, direction, glm::vec3(0.f, 0.f, -1 * maxDistance), false);
  auto btPosFrom = glmToBt(posFrom);
  auto btPosTo = glmToBt(posTo);
  env.dynamicsWorld -> rayTest(btPosFrom, btPosTo, result);

  for (int i = 0; i < result.m_hitFractions.size(); i++){
    const btCollisionObject* obj = result.m_collisionObjects[i];
    auto hitPoint = btPosFrom.lerp(btPosTo, result.m_hitFractions[i]);
    for (auto [objid, rigidbody] : rigidbodys){
      if (rigidbody == obj){
        hitobjects.push_back(HitObject{
          .id = objid,
          .point = btToGlm(hitPoint),  
        });
      }
    }
  } 
  assert(hitobjects.size() == result.m_hitFractions.size());
  return hitobjects;
}

bool pointInsideBody(btRigidBody* body, glm::vec3 point){
  assert(false);
  return false;
}