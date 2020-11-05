#include "./ainav.h"


// TODO this is hardcoded for the example, needs to be based off serializable data
void addDefaultConnections(std::map<std::string, std::vector<NavConnection>>& connections){  
  connections[";navmesh1"] = {
    NavConnection {
      .destination = ";navmesh2",
      .points = {
        NavPointConnection { 
          .fromPoint = glm::vec3(100.f, 0.f, 0.f),
          .toPoint = glm::vec3(100.f, 0.f, 0.f),
        },
      },
    },
  };
  connections[";navmesh2"] = {
    NavConnection {
      .destination = ";navmesh1",
      .points = {
        NavPointConnection { 
          .fromPoint = glm::vec3(0.f, 0.f, 0.f),
          .toPoint = glm::vec3(0.f, 0.f, 0.f),
        },
      },
    },
    NavConnection {
      .destination = ";navmesh3",
      .points = {
        NavPointConnection { 
          .fromPoint = glm::vec3(200.f, 0.f, 0.f),
          .toPoint = glm::vec3(200.f, 0.f, 0.f),
        },
      },
    },
  };
  connections[";navmesh3"] = {
    NavConnection {
      .destination = ";navmesh2",
      .points = {
        NavPointConnection { 
          .fromPoint = glm::vec3(100.f, 0.f, 0.f),
          .toPoint = glm::vec3(100.f, 0.f, 0.f),
        },
      },
    },
  };
}

NavGraph parseNavGraph(){
  std::map<std::string, std::vector<NavConnection>> connections;
  NavGraph graph {
    .connections = connections,
  };
  return graph;
}

NavGraph createNavGraph(){
  std::map<std::string, std::vector<NavConnection>> connections;
  addDefaultConnections(connections);
  NavGraph graph {
    .connections = connections,
  };
  return graph;
}

aiSearchResult searchPath(std::map<std::string, std::vector<NavConnection>>& connections, std::string from, std::string to, std::vector<std::string>& visited, std::vector<std::string> path){
  if (std::find(visited.begin(), visited.end(), from) != visited.end()){
    return aiSearchResult{ .found = false, .path = {} };
  }
  if (connections.find(from) == connections.end()){
    std::cout << "WARNING: node: " << from  << " not in connection graph" << std::endl;
    return aiSearchResult{ .found = false, .path = {} };
  }
  visited.push_back(from);
  path.push_back(from);

  if (from == to){
    return aiSearchResult { .found = true, .path = path };
  } 
  auto meshConnections = connections.at(from);
  for (auto connection : meshConnections){
    auto result = searchPath(connections, connection.destination, to, visited, path);
    if (result.found){
      return result;
    }
  }
  return aiSearchResult { .found = false, .path = {} };
}

aiSearchResult aiNavSearchPath(NavGraph& navgraph, std::string from, std::string to){
  std::vector<std::string> path;
  std::vector<std::string> visitedNodes;
  return searchPath(navgraph.connections, from, to, visitedNodes, path);
}

std::string targetNavmesh(glm::vec3 target, raycastFn raycast, std::function<bool(objid)> isNavmesh, std::function<std::string(objid)> getName){
  auto abitAbove = glm::vec3(target.x, target.y + 1, target.z);
  auto direction = orientationFromPos(abitAbove, target);
  auto hitObjects = raycast(abitAbove, direction, 5);
  if (hitObjects.size() == 0){
    assert(false);
    return "";
  }

  for (auto targetObject : hitObjects){
    if (isNavmesh(targetObject.id)){
      return getName(targetObject.id);
    }
  }
  assert(false);
  return "";
}

glm::vec3 aiTargetLink(NavGraph& navgraph, std::string from, std::string to){
  std::cout << "target link: " << from << " - " << to << std::endl;
  for (auto connection : navgraph.connections.at(from)){
    if (connection.destination == to){
      return connection.points.at(0).fromPoint;
    }
  }
  assert(false);
  return glm::vec3(0.f, 0.f, 0.f);
}

glm::vec3 aiNavPosition(objid id, glm::vec3 target, std::function<glm::vec3(objid, bool)> position, raycastFn raycast, std::function<bool(objid)> isNavmesh){
  auto objectPosition = position(id, true);
  auto directionTowardPoint = orientationFromPos(objectPosition, target);
  return moveRelative(objectPosition, directionTowardPoint, glm::vec3(0.f, 0.f, -0.5f), false);
}

