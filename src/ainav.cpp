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

struct NavTarget {
  bool valid;
  std::string from;
  std::string to;
};

NavTarget fieldNameToNavTarget(std::string field){
  auto values = filterWhitespace(split(field, '>'));
  assert(values.size() == 2);
  NavTarget target {
    .valid = true,
    .from = values.at(0),
    .to = values.at(1),
  };
  return target;
}

NavPointConnection parsePointExpression(std::string pointExpression){
  auto points = filterWhitespace(split(pointExpression, ','));
  assert(points.size() == 2);
  NavPointConnection point {
    .fromPoint = parseVec(points.at(0)),
    .toPoint = parseVec(points.at(1)),
  };
  return point;
}

std::vector<NavPointConnection> valueNameToNavPoints(std::string value){
  std::vector<NavPointConnection> points;
  auto pointExpressions = filterWhitespace(split(value, ';'));
  for (auto pointExpression : pointExpressions){
    points.push_back(parsePointExpression(pointExpression));
  }
  return points;
}

std::map<std::string, std::vector<NavConnection>> fieldsToConnections(std::map<std::string, std::string> fields){
  std::map<std::string, std::vector<NavConnection>> connections;
  for (auto [field, value] : fields){
    auto navTarget = fieldNameToNavTarget(field);
    auto navPoints = valueNameToNavPoints(value);
    if (connections.find(navTarget.from) == connections.end()){
      connections[navTarget.from] = {};
    }
    connections[navTarget.from].push_back(NavConnection{
      .destination = navTarget.to,
      .points = valueNameToNavPoints(value),
    });
    assert(navTarget.valid);
  }

  return connections;
}


NavGraph parseNavGraph(){
  std::map<std::string, std::string> fields;
  fields[";navmesh1 > ;navmesh2"] = "100 0 0, 100 0 0; 100 5 0, 100 5 0";
  fields[";navmesh2 > ;navmesh1"] = "100 0 0, 0 0 0";
  fields[";navmesh2 > ;navmesh3"] = "200 0 0, 200 0 0";
  fields[";navmesh3 >;navmesh2"] = "100 0 0, 100 0 0;";
  NavGraph graph {
    .connections = fieldsToConnections(fields),
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
      return connection.points.at(0).toPoint;
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

