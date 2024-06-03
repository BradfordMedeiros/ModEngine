#include "./collision_cache.h"

bool collisionInList(std::vector<CollisionInstance> currentCollisions, CollisionInstance collisionPair){
  for (auto collisionPairCheck : currentCollisions){
    if (collisionPair.obj1 == collisionPairCheck.obj1  && collisionPair.obj2 == collisionPairCheck.obj2 || 
        collisionPair.obj1 == collisionPairCheck.obj2 && collisionPair.obj2 == collisionPairCheck.obj1){
      return true;
    }
  }
  return false;
}

void onCollisionPosDoNothing(const btCollisionObject* obj1, const btCollisionObject* obj2, glm::vec3 contactPos, glm::vec3 normal, float force){}
void onCollisionDoNothing(const btCollisionObject* obj1, const btCollisionObject* obj2){}

CollisionCache::CollisionCache(){
  this -> onObjectEnter = onCollisionPosDoNothing;
  this -> onObjectLeave = onCollisionDoNothing;
}

CollisionCache::CollisionCache(collisionPairPosFn onObjectEnter, collisionPairFn onObjectLeave){
  this -> onObjectEnter = onObjectEnter;
  this -> onObjectLeave = onObjectLeave;
}

void CollisionCache::onObjectsCollide(std::vector<CollisionInstance>& collisionPairs){
  for (auto collisionPair : collisionPairs){
    if (!collisionInList(this -> oldCollisions, collisionPair)){
      onObjectEnter(collisionPair.obj1, collisionPair.obj2, collisionPair.pos, collisionPair.normal, collisionPair.force);
    }
  }
  for (auto collisionPair : this -> oldCollisions){
    if (!collisionInList(collisionPairs, collisionPair)){
      onObjectLeave(collisionPair.obj1, collisionPair.obj2);
    }
  }
  this -> oldCollisions = collisionPairs;
}


void CollisionCache::rmObject(const btCollisionObject* obj){
  std::vector<CollisionInstance> oldCollisions;   
  for (auto collisionObj : this -> oldCollisions){
    if (collisionObj.obj1 != obj && collisionObj.obj2 != obj){
      oldCollisions.push_back(collisionObj);
    }
  }
  this -> oldCollisions = oldCollisions;
}