#include "./physics_common.h"

glm::vec3 btToGlm(btVector3 pos){
  return glm::vec3(pos.getX(), pos.getY(), pos.getZ());
}
btVector3 glmToBt(glm::vec3 pos){
  return btVector3(pos.x, pos.y, pos.z);
}
glm::quat btToGlm(btQuaternion rotation){
  return glm::quat(rotation.getW(), rotation.getX(), rotation.getY(), rotation.getZ());
}
btQuaternion glmToBt(glm::quat rotation){
  return btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w);
}