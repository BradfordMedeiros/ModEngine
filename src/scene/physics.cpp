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
  SimpleMaskFilterCallback* filterCallback = new SimpleMaskFilterCallback();

  physicsEnv env = {
    .colConfig = colConfig,
    .dispatcher = dispatcher,
    .broadphase = broadphase,
    .constraintSolver = constraintSolver,
    .dynamicsWorld = dynamicsWorld,
    .collisionCache = collisionCache,
    .hasDebugDrawer = debugDrawer != NULL,
    .filterCallback = filterCallback,
  };

  env.dynamicsWorld -> getPairCache() -> setOverlapFilterCallback(filterCallback);
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
btRigidBody* createRigidBodyRect(glm::vec3 pos, float width, float height, float depth, glm::quat rot, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  btCollisionShape* shape = new btBoxShape(btVector3(btScalar(width * 1.f / 2 ), btScalar(height * 1.f / 2), btScalar(depth * 1.f / 2)));
  return createRigidBody(pos, shape, rot, isStatic, hasCollision, scaling, opts);
}
btRigidBody* createRigidBodySphere(glm::vec3 pos, float radius, glm::quat rot, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  btCollisionShape* shape = new btSphereShape(radius); 
  return createRigidBody(pos, shape, rot, isStatic, hasCollision, scaling, opts);
}
btRigidBody* createRigidBodyCapsule(physicsEnv& env, float radius, float height, glm::vec3 pos, glm::quat rot, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  btCollisionShape* shape = new btCapsuleShape(radius, height);
  return createRigidBody(pos, shape, rot, isStatic, hasCollision, scaling, opts);
};
btRigidBody* createRigidBodyCylinder(physicsEnv& env, float radius, float height, glm::vec3 pos, glm::quat rot, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  btCollisionShape* shape = new btCylinderShape(btVector3(radius, radius, height));
  return createRigidBody(pos, shape, rot, isStatic, hasCollision, scaling, opts);
}

btRigidBody* createRigidBodyHull(physicsEnv& env, std::vector<glm::vec3>& verts, glm::vec3 pos, glm::quat rot, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  std::cout << "rigid body hull not yet implemented" << std::endl;
  assert(false);
  return NULL;
  /*assert(verts.size() % 3 == 0);
  btTriangleMesh trimesh;
  for (int i = 0; i < verts.size(); i+=3){
    trimesh.addTriangle(glmToBt(verts.at(i)), glmToBt(verts.at(i + 1)), glmToBt(verts.at(i + 2)));
  }
  btConvexTriangleMeshShape shape(&trimesh);
  btShapeHull hullBuilder(&shape);
  hullBuilder.buildHull(0);

  btTriangleMesh*  hullmesh = new btTriangleMesh();
  const btVector3* hullVertexs = hullBuilder.getVertexPointer();
  for (int i = 0; i < hullBuilder.numVertices(); i+=3){
    hullmesh -> addTriangle(hullVertexs[i], hullVertexs[i + 1], hullVertexs[i + 2]);
  }
  btConvexTriangleMeshShape* shape2 = new btConvexTriangleMeshShape(hullmesh);
  shape2 -> setMargin(0);

  return createRigidBody(pos, shape2, rot, isStatic, hasCollision, scaling, opts);*/
}

btRigidBody* createRigidBodyExact(physicsEnv& env, std::vector<glm::vec3>& verts, glm::vec3 pos, glm::quat rot, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  assert(verts.size() % 3 == 0);
  btTriangleMesh*  trimesh = new btTriangleMesh();
  for (int i = 0; i < verts.size(); i+=3){
    trimesh -> addTriangle(glmToBt(verts.at(i)), glmToBt(verts.at(i + 1)), glmToBt(verts.at(i + 2)));
  }
  btTriangleMeshShape* shape = new btBvhTriangleMeshShape(trimesh, true);  
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
  btConvexTriangleMeshShape* tshape =  dynamic_cast<btConvexTriangleMeshShape*>(shape);

  if (cshape != NULL){
    int numChildren = cshape -> getNumChildShapes();
    for (int i = 0; i < numChildren; i++){
      btCollisionShape * childShape = cshape -> getChildShape(i);
      delete childShape;
    }
  }else if (tshape != NULL){
    auto trimesh = tshape -> getMeshInterface();
    delete trimesh;
  }
  delete shape;
  delete body;
}

// Due to bullet weirdness, this seems like it has to be called after adding rigid body to world (for the gravity part)
void setPhysicsOptions(btRigidBody* body, rigidBodyOpts& opts){
  body -> setLinearFactor(glmToBt(opts.linear));
  body -> setAngularFactor(glmToBt(opts.angular));
  body -> setGravity(glmToBt(opts.gravity));
  body -> setFriction(opts.friction);
  body -> setRestitution(opts.restitution);

  auto collisionFlags = body -> getCollisionFlags();
  auto isStatic = (collisionFlags | btCollisionObject::CF_KINEMATIC_OBJECT) ==  collisionFlags; 
  if (!isStatic){
    body -> setMassProps(opts.mass, body -> getLocalInertia());
  }
  if (opts.velocity.has_value()){
    body -> setLinearVelocity(glmToBt(opts.velocity.value()));
  }
  if (opts.angularVelocity.has_value()){
    body -> setAngularVelocity(glmToBt(opts.angularVelocity.value()));
  }
  body -> getBroadphaseHandle() -> m_collisionFilterMask = opts.layer;
}
btRigidBody* addBodyToWorld(physicsEnv& env, btRigidBody* rigidBodyPtr, rigidBodyOpts& opts){
  env.dynamicsWorld -> addRigidBody(rigidBodyPtr, 1, opts.layer);
  setPhysicsOptions(rigidBodyPtr, opts);
  return rigidBodyPtr;
}
btRigidBody* addRigidBodyRect(physicsEnv& env, glm::vec3 pos, float width, float height, float depth, glm::quat rotation, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){  
  auto rigidBodyPtr = createRigidBodyRect(pos, width, height, depth, rotation, isStatic, hasCollision, scaling, opts);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}
btRigidBody* addRigidBodySphere(physicsEnv& env, glm::vec3 pos, float radius, glm::quat rotation, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  auto rigidBodyPtr = createRigidBodySphere(pos, radius, rotation, isStatic, hasCollision, scaling, opts);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}
btRigidBody* addRigidBodyCapsule(physicsEnv& env, float radius, float height, glm::vec3 pos, glm::quat rot, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  auto rigidBodyPtr = createRigidBodyCapsule(env, radius, height, pos, rot, isStatic, hasCollision, scaling, opts);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}
btRigidBody* addRigidBodyCylinder(physicsEnv& env, float radius, float height, glm::vec3 pos, glm::quat rot, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  auto rigidBodyPtr = createRigidBodyCylinder(env, radius, height, pos, rot, isStatic, hasCollision, scaling, opts);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}
btRigidBody* addRigidBodyHull(physicsEnv& env, std::vector<glm::vec3>& verts, glm::vec3 pos, glm::quat rot, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  auto rigidBodyPtr = createRigidBodyHull(env, verts, pos, rot, isStatic, hasCollision, scaling, opts);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}
btRigidBody* addRigidBodyExact(physicsEnv& env, std::vector<glm::vec3>& verts, glm::vec3 pos, glm::quat rot, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  auto rigidBodyPtr = createRigidBodyExact(env, verts, pos, rot, isStatic, hasCollision, scaling, opts);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}
btRigidBody* addRigidBodyVoxel(physicsEnv& env, glm::vec3 pos, glm::quat rotation, std::vector<VoxelBody> bodies, bool isStatic, bool hasCollision, glm::vec3 scaling, rigidBodyOpts opts){
  auto rigidBodyPtr = createRigidBodyCompound(pos, rotation, bodies, isStatic, hasCollision, scaling, opts);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}

btRigidBody* addRigidBodyHeightmap(physicsEnv& env, glm::vec3 pos, glm::quat rotation, bool isStatic, rigidBodyOpts opts, float* data, int width, int height, glm::vec3 scaling, float minHeight, float maxHeight){
  auto rigidBodyPtr = createRigidBodyHeightmap(pos, rotation, isStatic, opts, data, width, height, scaling, minHeight, maxHeight);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}

void rmRigidBody(physicsEnv& env, btRigidBody* body){
  env.collisionCache.rmObject(body);
  env.dynamicsWorld -> removeRigidBody(body);
  cleanupRigidBody(body);
}

void updateRigidBodyOpts(physicsEnv& env, btRigidBody* body, rigidBodyOpts opts){
  setPhysicsOptions(body, opts);
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

void setTransform(btRigidBody* body, glm::vec3 pos, glm::vec3 scale, glm::quat rotation){
  setPosition(body, pos);
  setScale(body, scale.x, scale.y, scale.z);
  setRotation(body, rotation);
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
      
      collisionPairs.push_back(CollisionInstance {
        .obj1 = contactManifold -> getBody0(),
        .obj2 = contactManifold -> getBody1(),
        .pos = btToGlm(median),
        .normal = glm::normalize(btToGlm(point.m_normalWorldOnB)),
      });     
    }
  } 
  env.collisionCache.onObjectsCollide(collisionPairs);
}

void stepPhysicsSimulation(physicsEnv& env, float timestep, bool paused, bool enablePhysics){
  MODTODO("step physics simulation substeps should have consideration");
  if (enablePhysics && !paused){
    env.dynamicsWorld -> stepSimulation(timestep, 0);  
    checkCollisions(env);
  }
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
void applyForce(btRigidBody* body, glm::vec3 force){
  body -> applyCentralForce(btVector3(force.x, force.y, force.z));
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
glm::vec3 getVelocity(btRigidBody* body){
  auto velocity = body -> getLinearVelocity();
  return btToGlm(velocity);
}
glm::vec3 getAngularVelocity(btRigidBody* body){
  auto velocity = body -> getAngularVelocity();
  return btToGlm(velocity);
}


ModAABB getModAABB(btRigidBody* body){
  auto transform = body -> getWorldTransform();

  btVector3 vec1(0, 0, 0);
  btVector3 vec2(0, 0, 0);
  btCollisionShape* shape = body -> getCollisionShape(); 
  shape  -> getAabb(transform, vec1, vec2);
  return ModAABB {
    .position = btToGlm(transform.getOrigin()),
    .min = btToGlm(vec1),
    .max = btToGlm(vec2),
  };
}


void deinitPhysics(physicsEnv env){
  modlog("physics", "deinitializing physics system");
  MODTODO("maybe clean up rigid bodies too but maybe not");
  delete env.filterCallback;
  delete env.dynamicsWorld;
  delete env.constraintSolver;
  delete env.broadphase;
  delete env.dispatcher;
  delete env.colConfig;
}

class AllHitsRayResultCallbackCustomFilter : public btCollisionWorld::AllHitsRayResultCallback {
public:
  AllHitsRayResultCallbackCustomFilter(btVector3 x, btVector3 y) : btCollisionWorld::AllHitsRayResultCallback(x,y) {}
  bool needsCollision(btBroadphaseProxy* proxy0) const;
};

bool AllHitsRayResultCallbackCustomFilter::needsCollision(btBroadphaseProxy* proxy0) const {
  return true;
}

std::vector<HitObject> raycast(physicsEnv& env, std::map<objid, PhysicsValue>& rigidbodys, glm::vec3 posFrom, glm::quat direction, float maxDistance){
  std::vector<HitObject> hitobjects;
  AllHitsRayResultCallbackCustomFilter result(glmToBt(posFrom),glmToBt(posFrom));
 
  auto posTo = moveRelative(posFrom, direction, glm::vec3(0.f, 0.f, -1 * maxDistance), false);
  auto btPosFrom = glmToBt(posFrom);
  auto btPosTo = glmToBt(posTo);
  env.dynamicsWorld -> rayTest(btPosFrom, btPosTo, result);

  for (int i = 0; i < result.m_hitFractions.size(); i++){
    const btCollisionObject* obj = result.m_collisionObjects[i];
    auto hitPoint = btPosFrom.lerp(btPosTo, result.m_hitFractions[i]);
    auto hitNormal = result.m_hitNormalWorld[i];
    bool found = false;
    for (auto &[objid, physicsObj] : rigidbodys){
      if (physicsObj.body == obj){
        hitobjects.push_back(HitObject{
          .id = objid,
          .point = btToGlm(hitPoint),  
          .normal = quatFromDirection(btToGlm(hitNormal)),
        });
        found = true;
      }
    }
    assert(found);
  } 
  assert(hitobjects.size() == result.m_hitFractions.size());
  return hitobjects;
}