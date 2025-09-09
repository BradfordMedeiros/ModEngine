#ifndef MOD_PHYSICS
#define MOD_PHYSICS

#include <iostream>
#include <vector>
#include <map>
#include <optional>
#include <glm/gtc/quaternion.hpp>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>


#include "./collision_cache.h"
#include "./common/util/types.h"
#include "../common/util.h"
#include "../translations.h"
#include "./bulletdebug.h"
#include "./physics_common.h"

// Semi modified contract:   We just expose a single int as a layer mask 
// And then that gets used.  Nice and simple to reason about
struct SimpleMaskFilterCallback : public btOverlapFilterCallback{
  virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const {
    if (proxy0 -> m_collisionFilterMask == 0){
      return false;
    }
    if (proxy1 -> m_collisionFilterMask == 0){
      return false;
    }
    return !(proxy0 -> m_collisionFilterMask & proxy1 -> m_collisionFilterMask) || (proxy0 -> m_collisionFilterMask == proxy1 -> m_collisionFilterMask);
  }
};

struct physicsEnv {
  btDefaultCollisionConfiguration* colConfig;
  btCollisionDispatcher* dispatcher;
  btDbvtBroadphase* broadphase;
  btSequentialImpulseConstraintSolver* constraintSolver;
  btDiscreteDynamicsWorld* dynamicsWorld;
  CollisionCache collisionCache;
  bool hasDebugDrawer;
  SimpleMaskFilterCallback* filterCallback;
};

physicsEnv initPhysics(collisionPairPosFn onObjectEnter,  collisionPairFn onObjectLeave, btIDebugDraw* debugDrawer);
void deinitPhysics(physicsEnv env);
void stepPhysicsSimulation(physicsEnv& env, float timestep, bool paused, bool enablePhysics, bool drawOnly);

btCollisionObject* addRigidBodyRect(physicsEnv& env, glm::vec3 pos, float width, float height, float depth, glm::quat rotation, glm::vec3 scaling, rigidBodyOpts opts, bool ghost);
btCollisionObject* addRigidBodySphere(physicsEnv& env, glm::vec3 pos, float radius, glm::quat rotation, glm::vec3 scaling, rigidBodyOpts opts, bool ghost);
btCollisionObject* addRigidBodyCapsule(physicsEnv& env, float radius, float height,  glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts);
btCollisionObject* addRigidBodyCylinder(physicsEnv& env, float radius, float height,  glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts);
btCollisionObject* addRigidBodyHull(physicsEnv& env, std::vector<glm::vec3>& verts, glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts);
btCollisionObject* addRigidBodyExact(physicsEnv& env, std::vector<glm::vec3>& verts, glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts);
btCollisionObject* addRigidBodyOctree(physicsEnv& env, glm::vec3 pos, glm::quat rotation, glm::vec3 scaling, rigidBodyOpts opts, std::vector<PositionAndScale>& blocks, std::vector<PositionAndScaleVerts>& shapes);

void rmRigidBody(physicsEnv& env, btCollisionObject* colObject);

void updateRigidBodyOpts(physicsEnv& env, btCollisionObject* colObject, rigidBodyOpts opts);

void setPosition(btCollisionObject* body, glm::vec3);
glm::vec3 getPosition(btCollisionObject* rigidbody);
void setRotation(btCollisionObject* body, glm::quat rotation);
glm::quat getRotation(btCollisionObject* body);
void setScale(physicsEnv& env, btCollisionObject* body, float width, float height, float depth);
glm::vec3 getScale(btCollisionObject* body);
void setTransform(physicsEnv& env, btCollisionObject* body, glm::vec3 pos, glm::vec3 scale, glm::quat rotation);

void setVelocity(btCollisionObject* obj, glm::vec3 velocity);
void setAngularVelocity(btCollisionObject* obj, glm::vec3 angularVelocity);

void applyImpulse(btCollisionObject* colObject, glm::vec3 force);
void clearImpulse(btCollisionObject* colObject);
void applyForce(btCollisionObject* colObject, glm::vec3 force);
void applyTorque(btCollisionObject* colObject, glm::vec3 torque);

void clampMaxVelocity(btCollisionObject* colObject, float maxspeed);
glm::vec3 getVelocity(btCollisionObject* colObject);
glm::vec3 getAngularVelocity(btCollisionObject* colObject);

ModAABB getModAABB(btCollisionObject* body);

void printRigidBodyInfo(btRigidBody* body);

std::vector<HitObject> raycast(physicsEnv& env, std::unordered_map<objid, PhysicsValue>& rigidbodys, glm::vec3 posFrom, glm::quat direction, float maxDistance);
std::vector<HitObject> contactTest(physicsEnv& env, std::unordered_map<objid, PhysicsValue>& rigidbodys, btCollisionObject* body);
std::vector<HitObject> contactTestShape(physicsEnv& env, std::unordered_map<objid, PhysicsValue>& rigidbodys, glm::vec3 pos, glm::quat orientation, glm::vec3 scale);

float calculateRadiusForScale(glm::vec3 scale);


bool staticallyUpdated(btCollisionObject* collisionObj);


#endif 
