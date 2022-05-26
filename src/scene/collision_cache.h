#ifndef MOD_PHYSICS_CACHE
#define MOD_PHYSICS_CACHE

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <btBulletDynamicsCommon.h>

typedef void(*collisionPairPosFn)(const btCollisionObject* obj1, const btCollisionObject* obj2, glm::vec3 contactPos, glm::vec3 normal);
typedef void(*collisionPairFn)(const btCollisionObject* obj1, const btCollisionObject* obj2);

struct CollisionInstance {
  const btCollisionObject* obj1;
  const btCollisionObject* obj2;
  glm::vec3 pos;
  glm::vec3 normal;
};

bool collisionInList(
  std::vector<CollisionInstance> currentCollisions,
  CollisionInstance collisionPair
);

class CollisionCache {
  private:
    collisionPairPosFn onObjectEnter;
    collisionPairFn onObjectLeave;
    std::vector<CollisionInstance> oldCollisions;   
  public:
    CollisionCache();
    CollisionCache(collisionPairPosFn onObjectEnter, collisionPairFn onObjectLeave);
    void onObjectsCollide(std::vector<CollisionInstance>& collisionPairs);
};

#endif 


