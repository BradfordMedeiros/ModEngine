#ifndef MOD_PHYSICS_COMMON
#define MOD_PHYSICS_COMMON

#include <glm/gtc/quaternion.hpp>
#include <bullet/btBulletDynamicsCommon.h>

glm::vec3 btToGlm(btVector3 pos);
btVector3 glmToBt(glm::vec3 pos);
glm::quat btToGlm(btQuaternion rotation);
btQuaternion glmToBt(glm::quat rotation);

struct PhysicsValue {
  btRigidBody* body;
  glm::vec3 offset;
};

#endif
