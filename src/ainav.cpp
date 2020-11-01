#include "./ainav.h"

NavGraph createNavGraph(){
  std::map<std::string, NavConnection> connections;
  NavGraph graph {
    .connections = connections,
  };
  return graph;
}

glm::vec3 aiNavPosition(
  objid id, 
  glm::vec3 target,
  std::function<glm::vec3(objid, bool)> position,
  std::function<std::vector<HitObject>(glm::vec3 pos, glm::quat direction, float maxDistance)>raycast,
  std::function<bool(objid)> isNavmesh
){
  auto abitAbove = glm::vec3(target.x, target.y + 1, target.z);
  auto direction = orientationFromPos(abitAbove, target);
  auto hitObjects = raycast(abitAbove, direction, 1.1);

  auto objectPosition = position(id, true);
  if (hitObjects.size() == 0){
    return objectPosition;
  }
  auto targetObject = hitObjects.at(0);
  if (isNavmesh(targetObject.id)){
    auto directionTowardPoint = orientationFromPos(objectPosition, targetObject.point);
    return moveRelative(objectPosition, directionTowardPoint, glm::vec3(0.f, 0.f, -0.1f), false);
  }
  return objectPosition;
}

