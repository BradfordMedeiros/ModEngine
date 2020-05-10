#ifndef MOD_PHYSICS
#define MOD_PHYSICS

#include <iostream>
#include <vector>
#include <glm/gtc/quaternion.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include "./collision_cache.h"
#include "./common/util/types.h"
#include "./bulletdebug.h"
#include "./physics_common.h"

struct physicsEnv {
  btDefaultCollisionConfiguration* colConfig;
  btCollisionDispatcher* dispatcher;
  btDbvtBroadphase* broadphase;
  btSequentialImpulseConstraintSolver* constraintSolver;
  btDiscreteDynamicsWorld* dynamicsWorld;
  CollisionCache collisionCache;
  bool hasDebugDrawer;
};

physicsEnv initPhysics(collisionPairFn onObjectEnter,  collisionPairFn onObjectLeave, btIDebugDraw* debugDrawer);
void deinitPhysics(physicsEnv env);
void stepPhysicsSimulation(physicsEnv& env, float timestep);

btRigidBody* addRigidBody(physicsEnv& env, glm::vec3 pos, float width, float height, float depth, glm::quat rotation, bool isStatic, bool hasCollision, bool isCentered, glm::vec3 scaling, glm::vec3 linear, glm::vec3 angular, glm::vec3 gravity);
btRigidBody* addRigidBody(physicsEnv& env, glm::vec3 pos, float radius, glm::quat rotation, bool isStatic, bool hasCollision, glm::vec3 scaling, glm::vec3 linear, glm::vec3 angular, glm::vec3 gravity);
btRigidBody* addRigidBody(physicsEnv& env, glm::vec3 pos, glm::quat rotation, std::vector<VoxelBody> bodies, bool isStatic, bool hasCollision, glm::vec3 scaling, glm::vec3 linear, glm::vec3 angular, glm::vec3 gravity);
void rmRigidBody(physicsEnv& env, btRigidBody* body);

void setPosition(btRigidBody* body, glm::vec3);
glm::vec3 getPosition(btRigidBody* rigidbody);
void setRotation(btRigidBody* body, glm::quat rotation);
glm::quat getRotation(btRigidBody* body);
void setScale(btRigidBody* body, float width, float height, float depth);
glm::vec3 getScale(btRigidBody* body);

void applyImpulse(btRigidBody* body, glm::vec3 force);
void clearImpulse(btRigidBody* body);

void printRigidBodyInfo(btRigidBody* body);

#endif 
