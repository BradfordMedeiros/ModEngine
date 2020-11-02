#include "./ainav.h"


// TODO this is hardcoded for the example, needs to be based off serializable data
void addDefaultConnections(std::map<std::string, NavConnection>& connections){  
  connections[";navmesh1"] = NavConnection {
    .destination = ";navmesh2",
    .points = {
      NavPointConnection { 
        .fromPoint = glm::vec3(0.f, 0.f, 0.f),
        .toPoint = glm::vec3(0.f, 0.f, 0.f),
      },
      NavPointConnection { 
        .fromPoint = glm::vec3(1.f, 0.f, 0.f),
        .toPoint = glm::vec3(1.f, 0.f, 0.f),
      },
    },
  };
  connections[";navmesh2"] = NavConnection {
    .destination = ";navmesh1",
    .points = {
      NavPointConnection { 
        .fromPoint = glm::vec3(0.f, 0.f, 0.f),
        .toPoint = glm::vec3(0.f, 0.f, 0.f),
      },
      NavPointConnection { 
        .fromPoint = glm::vec3(1.f, 0.f, 0.f),
        .toPoint = glm::vec3(1.f, 0.f, 0.f),
      },
    },
  };
}

NavGraph createNavGraph(){
  std::map<std::string, NavConnection> connections;
  addDefaultConnections(connections);
  NavGraph graph {
    .connections = connections,
  };
  return graph;
}

std::vector<std::string> aiNavSearchPath(NavGraph& connections, std::string from, std::string to){
  std::vector<std::string> path;
  path.push_back("a");
  path.push_back("b");
  path.push_back("c");
  return path;
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

