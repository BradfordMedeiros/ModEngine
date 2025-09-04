#ifndef MOD_PHYSICS_COMMON
#define MOD_PHYSICS_COMMON

#include <glm/gtc/quaternion.hpp>
#include <btBulletDynamicsCommon.h>
#include <optional>
#include <variant>

glm::vec3 btToGlm(btVector3 pos);
btVector3 glmToBt(glm::vec3 pos);
glm::quat btToGlm(btQuaternion rotation);
btQuaternion glmToBt(glm::quat rotation);

struct PhysicsValue {
  btRigidBody* body;
  std::optional<glm::vec3> offset;
  bool customManaged = false;
};

struct rigidBodyOpts {
  glm::vec3 linear;
  glm::vec3 angular;
  glm::vec3 gravity;
  float friction;
  float restitution;
  float mass;
  int layer;

  float linearDamping;
  bool isStatic;
  bool hasCollisions;
};

struct PhysicsCreateSphere {
  float radius;
};
struct PhysicsCreateRect {
  float width;
  float height;
  float depth;
};
typedef std::variant<PhysicsCreateSphere, PhysicsCreateRect> ShapeCreateType;


#endif
