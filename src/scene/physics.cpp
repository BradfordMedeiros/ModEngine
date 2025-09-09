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

btCollisionObject* createRigidBody(glm::vec3 pos, btCollisionShape* shape, glm::quat rot, glm::vec3 scaling, bool ghost){
  shape -> setLocalScaling(glmToBt(scaling));

  btTransform transform;
  transform.setIdentity();
  transform.setOrigin(glmToBt(pos));   
  transform.setRotation(glmToBt(rot));
  if (ghost){
    btGhostObject* ghost = new btGhostObject();
    ghost -> setCollisionShape(shape);
    return ghost;
  }

  btDefaultMotionState* motionState = new btDefaultMotionState(transform);
  btScalar mass(0.f);
  btVector3 inertia(0, 0, 0);
  auto constructionInfo = btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, inertia);
  auto body  = new btRigidBody(constructionInfo);
  return body;
}

void setPhysicsOptions(btCollisionObject* colObject, rigidBodyOpts& opts, bool skipCollisionMask = false){
  btRigidBody* body = btRigidBody::upcast(colObject);
  if (body){
    body -> setLinearFactor(glmToBt(opts.linear));
    body -> setAngularFactor(glmToBt(opts.angular));
    body -> setGravity(glmToBt(opts.gravity));
    body -> setFriction(opts.friction);
    body -> setRestitution(opts.restitution);
    body -> setDamping(0.1f, 0.6f);
  }
  if (!opts.hasCollisions){
    colObject -> setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
  }else{
    colObject -> setCollisionFlags(body->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);
    colObject -> forceActivationState(ACTIVE_TAG);
    colObject -> activate(true);   
  }

  if (opts.isStatic){
    colObject -> setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    colObject -> setActivationState(DISABLE_DEACTIVATION);
  }else{
    colObject -> setCollisionFlags(body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
    colObject -> activate(true);
  }

  if (body){
    btScalar mass = opts.isStatic ? btScalar(0.f) : btScalar(opts.mass);
    btVector3 inertia(0, 0, 0);

    btCollisionShape* shape = colObject -> getCollisionShape(); 
    shape -> calculateLocalInertia(mass, inertia);
    body -> setMassProps(mass, inertia);
    body -> updateInertiaTensor();
  }

  if (!skipCollisionMask){
    modassert(colObject -> getBroadphaseHandle() != NULL, "null broadphase handle");
    colObject -> getBroadphaseHandle() -> m_collisionFilterMask = opts.layer;
  }    

}

btCollisionObject* createRigidBodyRect(glm::vec3 pos, float width, float height, float depth, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts, bool ghost){
  btCollisionShape* shape = new btBoxShape(btVector3(btScalar(width * 1.f / 2 ), btScalar(height * 1.f / 2), btScalar(depth * 1.f / 2)));
  return createRigidBody(pos, shape, rot, scaling, ghost);
}
btCollisionObject* createRigidBodySphere(glm::vec3 pos, float radius, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts, bool ghost){
  btCollisionShape* shape = new btSphereShape(radius); 
  shape -> setMargin(0.1); // TODO check if this margin makes sense

  return createRigidBody(pos, shape, rot, scaling, ghost);
}
btCollisionObject* createRigidBodyCapsule(physicsEnv& env, float radius, float height, glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts){
  btCollisionShape* shape = new btCapsuleShape(radius, height);
  return createRigidBody(pos, shape, rot, scaling, false);
};
btCollisionObject* createRigidBodyCylinder(physicsEnv& env, float radius, float height, glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts){
  btCollisionShape* shape = new btCylinderShape(btVector3(radius, radius, height));
  return createRigidBody(pos, shape, rot, scaling, false);
}

btCollisionObject* createRigidBodyHull(physicsEnv& env, std::vector<glm::vec3>& verts, glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts){
  modassert(verts.size() % 3 == 0, "verts must be % 3 = 0");
  btTriangleMesh* trimesh = new btTriangleMesh();
  for (int i = 0; i < verts.size(); i+=3){
    trimesh -> addTriangle(glmToBt(verts.at(i)), glmToBt(verts.at(i + 1)), glmToBt(verts.at(i + 2)));
  }
  btConvexTriangleMeshShape* shape = new btConvexTriangleMeshShape(trimesh);
  shape -> setMargin(0.1); // TODO check if this margin makes sense
  btShapeHull hullBuilder(shape);
  hullBuilder.buildHull(0);

  std::cout << "physics createRigidBodyHull, created, allocated: 2 elements" << std::endl;
  return createRigidBody(pos, shape, rot, scaling, false);
}

btCollisionObject* createRigidBodyExact(physicsEnv& env, std::vector<glm::vec3>& verts, glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts){
  modassert(opts.isStatic, "physics - bullet limitation - cannot create non-static exact");
  modassert(verts.size() % 3 == 0, std::string("create rigid body exact, verts not multiple of 3, got  ") + std::to_string(verts.size()));
  modassert(verts.size() >= 3, std::string("create rigid body exact, not enough verts, got  ") + std::to_string(verts.size()));
  modlog("ridig body exact, added verts number", std::to_string((int)verts.size()));

  btTriangleMesh*  trimesh = new btTriangleMesh();

  for (int i = 0; i < verts.size(); i+=3){
    modlog("physics rigid body exact-  adding vert", print(verts.at(i)) + " ");
    trimesh -> addTriangle(glmToBt(verts.at(i)), glmToBt(verts.at(i + 1)), glmToBt(verts.at(i + 2)));
  }
  btTriangleMeshShape* shape = new btBvhTriangleMeshShape(trimesh, true);  
  shape -> setMargin(0.4); // TODO check if this margin makes sense

  return createRigidBody(pos, shape, rot, scaling, false);
}

void cleanupRigidBody(btCollisionObject* colObject){
  btRigidBody* body = btRigidBody::upcast(colObject);
  if (body){
    delete body -> getMotionState();
  }
  btCollisionShape* shape = colObject -> getCollisionShape(); 
  btCompoundShape* cshape = dynamic_cast<btCompoundShape*>(shape);      // @TODO verify this
  btConvexTriangleMeshShape* tshape =  dynamic_cast<btConvexTriangleMeshShape*>(shape);

  if (cshape != NULL){
    int numChildren = cshape -> getNumChildShapes();
    for (int i = 0; i < numChildren; i++){
      btCollisionShape* childShape = cshape -> getChildShape(i);
      btBvhTriangleMeshShape* childShapeMesh = dynamic_cast<btBvhTriangleMeshShape*>(childShape);
      //std::cout << "addr: child shape: " << childShape << std::endl;
      //std::cout << "addr: childShapeMesh: " << childShapeMesh << std::endl;
      if (childShapeMesh != NULL){
        delete childShapeMesh -> getMeshInterface();
      }
      delete childShape;
    }
  }else if (tshape != NULL){
    std::cout << "physics deleting convex triangle mesh" << std::endl;
    auto trimesh = tshape -> getMeshInterface();
    delete trimesh;
  }
  delete shape;
  delete colObject;
}

btRigidBody* toRigidBody(btCollisionObject* collisionObject){
  btRigidBody* body = btRigidBody::upcast(collisionObject);
  modassert(body != NULL, "not a rigid body");
  return body;
}

btCollisionObject* addBodyToWorld(physicsEnv& env, btCollisionObject* collisionObj, rigidBodyOpts& opts){
  setPhysicsOptions(collisionObj, opts, true); /* this can be true because opts.layer in next line*/
  btRigidBody* body = btRigidBody::upcast(collisionObj);
  if (body){
    env.dynamicsWorld -> addRigidBody(body, 1, opts.layer);
  }else{
    env.dynamicsWorld -> addCollisionObject(collisionObj, 1, opts.layer);
  }
  collisionObj -> getBroadphaseHandle() -> m_collisionFilterMask = opts.layer;
  if (body){
    body -> setGravity(glmToBt(opts.gravity)); // kind of lame, has to be done after added rigid body
  }
  return collisionObj;
}
btCollisionObject* addRigidBodyRect(physicsEnv& env, glm::vec3 pos, float width, float height, float depth, glm::quat rotation, glm::vec3 scaling, rigidBodyOpts opts, bool ghost){  
  auto rigidBodyPtr = createRigidBodyRect(pos, width, height, depth, rotation, scaling, opts, ghost);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}
btCollisionObject* addRigidBodySphere(physicsEnv& env, glm::vec3 pos, float radius, glm::quat rotation, glm::vec3 scaling, rigidBodyOpts opts, bool ghost){
  auto rigidBodyPtr = createRigidBodySphere(pos, radius, rotation, scaling, opts, ghost);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}
btCollisionObject* addRigidBodyCapsule(physicsEnv& env, float radius, float height, glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts){
  auto rigidBodyPtr = createRigidBodyCapsule(env, radius, height, pos, rot, scaling, opts);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}
btCollisionObject* addRigidBodyCylinder(physicsEnv& env, float radius, float height, glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts){
  auto rigidBodyPtr = createRigidBodyCylinder(env, radius, height, pos, rot, scaling, opts);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}
btCollisionObject* addRigidBodyHull(physicsEnv& env, std::vector<glm::vec3>& verts, glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts){
  auto rigidBodyPtr = createRigidBodyHull(env, verts, pos, rot, scaling, opts);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}
btCollisionObject* addRigidBodyExact(physicsEnv& env, std::vector<glm::vec3>& verts, glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts){
  auto rigidBodyPtr = createRigidBodyExact(env, verts, pos, rot, scaling, opts);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}


btCollisionObject* createRigidBodyOctree(physicsEnv& env, glm::vec3 pos, glm::quat rotation, glm::vec3 scaling, rigidBodyOpts opts, std::vector<PositionAndScale>& blocks, std::vector<PositionAndScaleVerts>& shapes){
  btCompoundShape* shape = new btCompoundShape();
  for (auto &block : blocks){
    modassert(block.size.x >= 0 && block.size.y >= 0 && block.size.z >= 0, "negative block size createRigidBodyOctree");
    glm::vec3 halfSize = 0.5f * block.size;
    glm::vec3 positionVec = block.position + glm::vec3(halfSize.x, halfSize.y, -1 * halfSize.z);
    btCollisionShape* cshape1 = new btBoxShape(glmToBt(halfSize));
    btTransform position;
    position.setIdentity();
    position.setOrigin(glmToBt(positionVec));
    shape -> addChildShape(position, cshape1);
  }

  for (auto &shapeType : shapes){
    for (auto &block : shapeType.specialBlocks){
      btTriangleMesh*  trimesh = new btTriangleMesh();
      modassert(shapeType.verts.size() % 3 == 0, "verts shapetype must be multiple of 3");
      for (int i = 0; i < shapeType.verts.size(); i+=3){
        trimesh -> addTriangle(glmToBt(shapeType.verts.at(i)), glmToBt(shapeType.verts.at(i + 1)), glmToBt(shapeType.verts.at(i + 2)));
      }
      btBvhTriangleMeshShape* triangleShape = new btBvhTriangleMeshShape(trimesh, true);
      //std::cout << "addr: btBvhTriangleMeshShape: " << triangleShape << std::endl;
      btTransform transform;
      transform.setIdentity();

      // first term brings it into the center relative to the new rotation, second term puts it back to the offset
      //auto extraOffset = (block.rotation * shapeType.centeringOffset) - shapeType.centeringOffset + block.position;
      auto scale = block.scale * 2.f;

      auto rotatedScaledCentering = block.rotation * (shapeType.centeringOffset * scale);
      if (rotatedScaledCentering.x < 0){
        rotatedScaledCentering.x *= -1;
      }
      if (rotatedScaledCentering.y < 0){
        rotatedScaledCentering.y *= -1;
      }
      if (rotatedScaledCentering.z < 0){
        rotatedScaledCentering.z *= -1;
      }
      rotatedScaledCentering.z *= -1;

      auto extraOffset = (block.rotation * (scale * shapeType.centeringOffset))  + block.position + rotatedScaledCentering;


      transform.setRotation(glmToBt(block.rotation));
      transform.setOrigin(glmToBt(extraOffset));
      triangleShape -> setLocalScaling(glmToBt(scale));
      shape -> addChildShape(transform, triangleShape);
    }

  }

  return createRigidBody(pos, shape, rotation, scaling, false);
}


btCollisionObject* addRigidBodyOctree(physicsEnv& env, glm::vec3 pos, glm::quat rotation, glm::vec3 scaling, rigidBodyOpts opts, std::vector<PositionAndScale>& blocks, std::vector<PositionAndScaleVerts>& shapes){
  auto rigidBodyPtr = createRigidBodyOctree(env, pos, rotation, scaling, opts, blocks, shapes);
  return addBodyToWorld(env, rigidBodyPtr, opts);
}

void rmRigidBody(physicsEnv& env, btCollisionObject* colObject){
  env.collisionCache.rmObject(colObject);
  env.dynamicsWorld -> removeCollisionObject(colObject);
  cleanupRigidBody(colObject);
}

void updateRigidBodyOpts(physicsEnv& env, btCollisionObject* colObject, rigidBodyOpts opts){
  bool removedBody = false;
  if (!opts.isStatic){
    removedBody = true;
    env.dynamicsWorld -> removeCollisionObject(colObject);
  }
  setPhysicsOptions(colObject, opts, true);
  if (removedBody){
    btRigidBody* body = btRigidBody::upcast(colObject);
    if (body){
      env.dynamicsWorld -> addRigidBody(body, 1, opts.layer);
    }else{
      env.dynamicsWorld -> addCollisionObject(body, 1, opts.layer);
    }
  }

  btRigidBody* body = btRigidBody::upcast(colObject);
  if (body){
    body -> setGravity(glmToBt(opts.gravity)); // kind of lame, has to be done after added rigid body
  }

}

glm::vec3 getPosition(btCollisionObject* body){
  return btToGlm(body -> getWorldTransform().getOrigin());
}
void setPosition(btCollisionObject* collisionObj, glm::vec3 pos){
  btRigidBody* body = btRigidBody::upcast(collisionObj);

  btTransform transform; 
  if (body){
    body -> getMotionState() -> getWorldTransform(transform);
  }else{
    transform = collisionObj -> getWorldTransform();
  }
  transform.setOrigin(glmToBt(pos));    
 
  if (body){
    body -> getMotionState() -> setWorldTransform(transform);
  }
  collisionObj -> setWorldTransform(transform);
  collisionObj -> activate(true); 
}


glm::quat getRotation(btCollisionObject* body){
  return btToGlm(body -> getWorldTransform().getRotation());
}
void setRotation(btCollisionObject* collisionObj, glm::quat rotation){
  btRigidBody* body = btRigidBody::upcast(collisionObj);

  btTransform transform; 
  if (body){
    body -> getMotionState() -> getWorldTransform(transform);
  }else{
    transform = collisionObj -> getWorldTransform();
  }
  transform.setRotation(glmToBt(rotation));    
 
  if (body){
    body -> getMotionState() -> setWorldTransform(transform);
  }
  collisionObj -> setWorldTransform(transform);
  collisionObj -> activate(true); 

}
void setScale(physicsEnv& env, btCollisionObject* body, float width, float height, float depth){
  body -> getCollisionShape() -> setLocalScaling(btVector3(width, height, depth));
  env.dynamicsWorld -> updateSingleAabb(body);

  // this use to be instead of update singleaabb see ff6d5292f275e725e37cf527e89a28e09c4ef241
  //env.dynamicsWorld -> removeRigidBody(body);   // if we don't add or remove it just gets stuck in the air...
  //env.dynamicsWorld -> addRigidBody(body, 1, mask);  // todo preserve physics mask
}
glm::vec3 getScale(btCollisionObject* body){
  return btToGlm(body -> getCollisionShape() -> getLocalScaling());
}

void setTransform(physicsEnv& env, btCollisionObject* body, glm::vec3 pos, glm::vec3 scale, glm::quat rotation){
  setPosition(body, pos);
  setScale(env, body, scale.x, scale.y, scale.z);
  setRotation(body, rotation);
}

void setVelocity(btCollisionObject* obj, glm::vec3 velocity){
  btRigidBody* body = btRigidBody::upcast(obj);
  if (body){
    body -> setLinearVelocity(glmToBt(velocity));
    body -> activate(true);    
  }
}
void setAngularVelocity(btCollisionObject* obj, glm::vec3 angularVelocity){
  btRigidBody* body = btRigidBody::upcast(obj);
  if (body){
    body -> setAngularVelocity(glmToBt(angularVelocity));
    body -> activate(true);    
  }
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
        .force = point.getAppliedImpulse(),
      });     
    }
  } 
  env.collisionCache.onObjectsCollide(collisionPairs);
}

void stepPhysicsSimulation(physicsEnv& env, float timestep, bool paused, bool enablePhysics, bool drawOnly){
  MODTODO("step physics simulation substeps should have consideration");
  if (!drawOnly && (enablePhysics && !paused)){
    env.dynamicsWorld -> stepSimulation(timestep, 0);  
    checkCollisions(env);
  }
  if (drawOnly && env.hasDebugDrawer){
    env.dynamicsWorld -> debugDrawWorld();
  }
}
void printRigidBodyInfo(btRigidBody* body){
  glm::vec3 origin = getPosition(body);
  std::cout << "position: (" << origin.x << " , " << origin.y << " , " << origin.z << " )" << std::endl;
}

void applyImpulse(btCollisionObject* colObject, glm::vec3 force){
  auto body = btRigidBody::upcast(colObject);
  if (body){
    body -> applyCentralImpulse(btVector3(force.x, force.y, force.z));
  }
}
void clearImpulse(btCollisionObject* colObject){
  auto body = btRigidBody::upcast(colObject);
  if (body){
    body -> applyCentralImpulse(btVector3(0.f, 0.f, 0.f));
  }
}
void applyForce(btCollisionObject* colObject, glm::vec3 force){
  auto body = btRigidBody::upcast(colObject);
  if (body){
    body -> applyCentralForce(btVector3(force.x, force.y, force.z));
  }
}
void applyTorque(btCollisionObject* colObject, glm::vec3 torque){
  auto body = btRigidBody::upcast(colObject);
  if (body){
    body -> applyTorque(btVector3(torque.x, torque.y, torque.z));
  }
}

void clampMaxVelocity(btCollisionObject* colObject, float maxspeed){
  if (maxspeed <= 0){
    return;
  }
  auto body = btRigidBody::upcast(colObject);
  if (body){
    auto velocity = body -> getLinearVelocity();
    auto xVelocity = fmax(-maxspeed, fmin(maxspeed, velocity.getX()));
    auto yVelocity = fmax(-maxspeed, fmin(maxspeed, velocity.getY()));
    auto zVelocity = fmax(-maxspeed, fmin(maxspeed, velocity.getZ()));
    body -> setLinearVelocity(btVector3(xVelocity, yVelocity, zVelocity));    
  }
}

glm::vec3 getVelocity(btCollisionObject* colObject){
  auto body = btRigidBody::upcast(colObject);
  if (body){
    auto velocity = body -> getLinearVelocity();
    return btToGlm(velocity);    
  }
  modassert(false, "not a rigidbody");
  return glm::vec3(0.f, 0.f, 0.f);
}
glm::vec3 getAngularVelocity(btCollisionObject* colObject){
  auto body = btRigidBody::upcast(colObject);
  if (body){
    auto velocity = body -> getAngularVelocity();
    return btToGlm(velocity);    
  }
  modassert(false, "not a rigidbody");
  return glm::vec3(0.f, 0.f, 0.f); 
}


ModAABB getModAABB(btCollisionObject* body){
  auto transform = body -> getWorldTransform();

  btVector3 vec1(0, 0, 0);
  btVector3 vec2(0, 0, 0);
  btCollisionShape* shape = body -> getCollisionShape(); 
  shape  -> getAabb(transform, vec1, vec2);
  return ModAABB {
    .position = btToGlm(transform.getOrigin()),
    .rotation = btToGlm(transform.getRotation()),
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
  bool needsCollision(btBroadphaseProxy* proxy0) const override;
};

bool AllHitsRayResultCallbackCustomFilter::needsCollision(btBroadphaseProxy* proxy0) const {
  return true;
}


std::optional<objid> getIdForCollisionObject(std::unordered_map<objid, PhysicsValue>& rigidbodys, const btCollisionObject* obj){
  for (auto &[id, physicsObj] : rigidbodys){
    if (physicsObj.collisionObj == obj){
      return id;
    }
  }
  return std::nullopt;
}

std::vector<HitObject> raycast(physicsEnv& env, std::unordered_map<objid, PhysicsValue>& rigidbodys, glm::vec3 posFrom, glm::quat direction, float maxDistance){
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
    auto id = getIdForCollisionObject(rigidbodys, obj).value();
    hitobjects.push_back(
      HitObject {
        .id = id,
        .mask = obj -> getBroadphaseHandle() -> m_collisionFilterMask,
        .point = btToGlm(hitPoint),  
        .normal = quatFromDirection(btToGlm(hitNormal)),
      }
    );    
  } 
  assert(hitobjects.size() == result.m_hitFractions.size());
  return hitobjects;
}

class ContactResultCallback : public btCollisionWorld::ContactResultCallback
{
public:
    std::function<void(const btCollisionObject* obj, glm::vec3, glm::quat)> callback;
    ContactResultCallback(btCollisionObject* testCollisionObj) : btCollisionWorld::ContactResultCallback() {
      this -> testCollisionObj = testCollisionObj;
    };

    virtual btScalar addSingleResult(
      btManifoldPoint& point,
      const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
      const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1
    ) override{
      const btCollisionObject* obj0 = colObj0Wrap -> m_collisionObject;
      const btCollisionObject* obj = colObj1Wrap -> m_collisionObject;
      modassert(obj0 != obj, "ContactResultCallback obj made contact with itself");
      // this is b/c you don't know which collision obj is the obj being tested against
      // normal is the only thing where this matters currently
      // normal(A,B) => -1 * normal(B, A)
      bool flipNormal = false;
      if (obj0 == this -> testCollisionObj){   
        flipNormal = true;
      }
      auto median = btToGlm((point.getPositionWorldOnA() + point.getPositionWorldOnB()) / 2.f);
      glm::vec3 direction = btToGlm(point.m_normalWorldOnB);
      if (!flipNormal){
        direction.x *= -1;
        direction.y *= -1;
        direction.z *= -1;
      }
      //modlog("contact", print(direction));
      auto normal  = quatFromDirection(direction);
      this -> callback(!flipNormal ? obj0 : obj, median, normal);
      return 0;
    }
    virtual bool needsCollision(btBroadphaseProxy* proxy) const override {
      return true;
    } ;
    btCollisionObject* testCollisionObj;
};

std::vector<HitObject> contactTest(physicsEnv& env, std::unordered_map<objid, PhysicsValue>& rigidbodys, btCollisionObject* body){
  auto contactCallback = ContactResultCallback(body);
  std::vector<HitObject> hitobjects = {};
  auto originalId = getIdForCollisionObject(rigidbodys, body);

  contactCallback.callback = [&rigidbodys, &hitobjects, body, originalId](const btCollisionObject* obj, glm::vec3 pos, glm::quat normal) -> void {
    auto id = getIdForCollisionObject(rigidbodys, obj).value();
    //modlog("contact test id: ", std::to_string(id) + std::string(", original id: ") + std::to_string(originalId.value()));
    modassert(id != originalId, "id and original id are the same");
    hitobjects.push_back(HitObject {
      .id = id,
      .mask = obj -> getBroadphaseHandle() -> m_collisionFilterMask,
      .point = pos,
      .normal = normal,
    });
  };

  for (auto &hitobject : hitobjects){
    modassert(hitobject.id != originalId, "contacted with itself, uh lol");
  }

  env.dynamicsWorld -> contactTest(body, contactCallback);
  return hitobjects;
}


// Create way to do visualization for this, add more shape types currently only sphere
std::vector<HitObject> contactTestShape(physicsEnv& env, std::unordered_map<objid, PhysicsValue>& rigidbodys, glm::vec3 pos, glm::quat orientation, glm::vec3 scale){
  static rigidBodyOpts opts {
    .linear = glm::vec3(1.f, 1.f, 1.f),
    .angular = glm::vec3(1.f, 1.f, 1.f),
    .gravity = glm::vec3(0.f, -9.81f, 0.f),
    .friction = 1.f,
    .restitution = 1.f,
    .mass = 1.f,
    .layer = 1,
    .linearDamping = 0.f,
    .isStatic = true,
    .hasCollisions = true,
  };

  auto body = addRigidBodySphere(env, pos, 1.f, orientation, scale, opts, false);
  auto originalId = getIdForCollisionObject(rigidbodys, body);

  setPosition(body, pos);
  setRotation(body, orientation);

  std::vector<HitObject> hitobjects = {};

  auto contactCallback = ContactResultCallback(body);
  contactCallback.callback = [&rigidbodys, &hitobjects, body](const btCollisionObject* obj, glm::vec3 pos, glm::quat normal) -> void {
    modassert(obj != body, "contactTest - made contact with itself");
    auto id = getIdForCollisionObject(rigidbodys, obj);
    hitobjects.push_back(HitObject {
      .id = id.value(),
      .mask = obj -> getBroadphaseHandle() -> m_collisionFilterMask,
      .point = pos,
      .normal = normal,
    });
  };

  for (auto &hitobject : hitobjects){
    modassert(hitobject.id != originalId, "contacted with itself, uh lol");
  }
  env.dynamicsWorld -> contactTest(body, contactCallback);
  rmRigidBody(env, body);
  return hitobjects;
}

float calculateRadiusForScale(glm::vec3 scale){
  return (maxvalue(scale.y, scale.y, scale.y) / 2.f);
}