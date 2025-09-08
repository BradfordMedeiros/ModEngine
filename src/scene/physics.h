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

btRigidBody* addRigidBodyRect(physicsEnv& env, glm::vec3 pos, float width, float height, float depth, glm::quat rotation, glm::vec3 scaling, rigidBodyOpts opts);
btRigidBody* addRigidBodySphere(physicsEnv& env, glm::vec3 pos, float radius, glm::quat rotation, glm::vec3 scaling, rigidBodyOpts opts);
btRigidBody* addRigidBodyCapsule(physicsEnv& env, float radius, float height,  glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts);
btRigidBody* addRigidBodyCylinder(physicsEnv& env, float radius, float height,  glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts);
btRigidBody* addRigidBodyHull(physicsEnv& env, std::vector<glm::vec3>& verts, glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts);
btRigidBody* addRigidBodyExact(physicsEnv& env, std::vector<glm::vec3>& verts, glm::vec3 pos, glm::quat rot, glm::vec3 scaling, rigidBodyOpts opts);
btRigidBody* addRigidBodyVoxel(physicsEnv& env, glm::vec3 pos, glm::quat rotation, std::vector<VoxelBody> bodies, glm::vec3 scaling, rigidBodyOpts opts);
btRigidBody* addRigidBodyOctree(physicsEnv& env, glm::vec3 pos, glm::quat rotation, glm::vec3 scaling, rigidBodyOpts opts, std::vector<PositionAndScale>& blocks, std::vector<PositionAndScaleVerts>& shapes);

void rmRigidBody(physicsEnv& env, btRigidBody* body);

void updateRigidBodyOpts(physicsEnv& env, btRigidBody* body, rigidBodyOpts opts);

void setPosition(btRigidBody* body, glm::vec3);
glm::vec3 getPosition(btRigidBody* rigidbody);
void setRotation(btRigidBody* body, glm::quat rotation);
glm::quat getRotation(btRigidBody* body);
void setScale(physicsEnv& env, btRigidBody* body, float width, float height, float depth);
glm::vec3 getScale(btRigidBody* body);
void setTransform(physicsEnv& env, btRigidBody* body, glm::vec3 pos, glm::vec3 scale, glm::quat rotation);

void setVelocity(btRigidBody* body, glm::vec3 velocity);
void setAngularVelocity(btRigidBody* body, glm::vec3 angularVelocity);

void applyImpulse(btCollisionObject* colObject, glm::vec3 force);
void clearImpulse(btCollisionObject* colObject);
void applyForce(btCollisionObject* colObject, glm::vec3 force);
void applyTorque(btCollisionObject* colObject, glm::vec3 torque);

void clampMaxVelocity(btCollisionObject* colObject, float maxspeed);
glm::vec3 getVelocity(btRigidBody* body);
glm::vec3 getAngularVelocity(btRigidBody* body);

ModAABB getModAABB(btCollisionObject* body);

void printRigidBodyInfo(btRigidBody* body);

std::vector<HitObject> raycast(physicsEnv& env, std::unordered_map<objid, PhysicsValue>& rigidbodys, glm::vec3 posFrom, glm::quat direction, float maxDistance);
std::vector<HitObject> contactTest(physicsEnv& env, std::unordered_map<objid, PhysicsValue>& rigidbodys, btCollisionObject* body);
std::vector<HitObject> contactTestShape(physicsEnv& env, std::unordered_map<objid, PhysicsValue>& rigidbodys, glm::vec3 pos, glm::quat orientation, glm::vec3 scale);

float calculateRadiusForScale(glm::vec3 scale);

#endif 
