#ifndef MOD_PHYSICS_CACHE
#define MOD_PHYSICS_CACHE

#include <vector>
#include <bullet/btBulletDynamicsCommon.h>

typedef void(*collisionPairFn)(const btCollisionObject* obj1, const btCollisionObject* obj2);

bool collisionInList(
  std::vector<std::pair<const btCollisionObject*, const btCollisionObject*>> currentCollisions,
  std::pair<const btCollisionObject*, const btCollisionObject*> collisionPair
);

class CollisionCache {
  private:
    collisionPairFn onObjectEnter;
    collisionPairFn onObjectLeave;
    std::vector<std::pair<const btCollisionObject*, const btCollisionObject*>> oldCollisions;   
  public:
    CollisionCache();
    CollisionCache(collisionPairFn onObjectEnter, collisionPairFn onObjectLeave);
    void onObjectsCollide(std::vector<std::pair<const btCollisionObject*, const btCollisionObject*>> collisionPairs);
};

#endif 


