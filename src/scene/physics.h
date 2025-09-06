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

struct SimpleMaskFilterCallback : public btOverlapFilterCallback {
  virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const {
    if (proxy0 -> m_collisionFilterMask == 0 || proxy1 -> m_collisionFilterMask == 0){
      return false;
    }
    return !(proxy0 -> m_collisionFilterMask & proxy1 -> m_collisionFilterMask) || (proxy0 -> m_collisionFilterMask == proxy1 -> m_collisionFilterMask);
  }
};

class btGhostLayerCollisionDispatcher : public btCollisionDispatcher {
public:
    btGhostLayerCollisionDispatcher(btCollisionConfiguration* config) : btCollisionDispatcher(config) {}

    bool needsCollision(const btCollisionObject* body0, const btCollisionObject* body1) override {
        // First respect the default rules (like groups/masks)
        if (!btCollisionDispatcher::needsCollision(body0, body1)){
          return false;
        }

        bool hasNoContactObj0 = (body0 -> getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0;
        bool hasNoContactObj1 = (body1 -> getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0;
        if (hasNoContactObj0 || hasNoContactObj1){
          return false;
          
        }
        if ((body0 -> getBroadphaseHandle() -> m_collisionFilterMask == 0) || (body1 -> getBroadphaseHandle() -> m_collisionFilterMask == 0)){
          return false;
        }

        return true;
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

void applyImpulse(btRigidBody* body, glm::vec3 force);
void clearImpulse(btRigidBody* body);
void applyForce(btRigidBody* body, glm::vec3 force);
void applyTorque(btRigidBody* body, glm::vec3 torque);

void clampMaxVelocity(btRigidBody* body, float maxspeed);
glm::vec3 getVelocity(btRigidBody* body);
glm::vec3 getAngularVelocity(btRigidBody* body);

ModAABB getModAABB(btRigidBody* body);

void printRigidBodyInfo(btRigidBody* body);

std::vector<HitObject> raycast(physicsEnv& env, std::unordered_map<objid, PhysicsValue>& rigidbodys, glm::vec3 posFrom, glm::quat direction, float maxDistance);
std::vector<HitObject> contactTest(physicsEnv& env, std::unordered_map<objid, PhysicsValue>& rigidbodys, btRigidBody* body);
std::vector<HitObject> contactTestShape(physicsEnv& env, std::unordered_map<objid, PhysicsValue>& rigidbodys, glm::vec3 pos, glm::quat orientation, glm::vec3 scale);

float calculateRadiusForScale(glm::vec3 scale);

#endif 
