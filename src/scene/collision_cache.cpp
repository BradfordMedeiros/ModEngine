#include "./collision_cache.h"

bool collisionInList(std::vector<std::pair<const btCollisionObject*, const btCollisionObject*>> currentCollisions, std::pair<const btCollisionObject*, const btCollisionObject*> collisionPair){
  for (auto collisionPairCheck : currentCollisions){
    if (collisionPair.first == collisionPairCheck.first && collisionPair.second == collisionPairCheck.second || collisionPair.first == collisionPairCheck.second && collisionPair.second == collisionPairCheck.first){
      return true;
    }
  }
  return false;
}

void  onCollisionDoNothing(const btCollisionObject* obj1, const btCollisionObject* obj2){}

CollisionCache::CollisionCache(){
  this -> onObjectEnter = onCollisionDoNothing;
  this -> onObjectLeave = onCollisionDoNothing;
}

CollisionCache::CollisionCache(collisionPairFn onObjectEnter, collisionPairFn onObjectLeave){
  this -> onObjectEnter = onObjectEnter;
  this -> onObjectLeave = onObjectLeave;
}

void CollisionCache::onObjectsCollide(std::vector<std::pair<const btCollisionObject*, const btCollisionObject*>> collisionPairs){
  for (auto collisionPair : collisionPairs){
    if (!collisionInList(oldCollisions, collisionPair)){
      onObjectEnter(collisionPair.first, collisionPair.second);
    }
  }
  for (auto collisionPair : this -> oldCollisions){
    if (!collisionInList(collisionPairs, collisionPair)){
      onObjectLeave(collisionPair.first, collisionPair.second);
    }
  }
  this -> oldCollisions = collisionPairs;
}

