#include "./ainav.h"

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


NavGraph createNavGraph(std::map<std::string, std::string> fields){
  NavGraph graph {
    .connections = fieldsToConnections(fields),
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

glm::vec3 aiNavPosition(objid id, glm::vec3 target, std::function<glm::vec3(objid)> position, raycastFn raycast, std::function<bool(objid)> isNavmesh){
  auto objectPosition = position(id);
  auto directionTowardPoint = orientationFromPos(objectPosition, target);
  return moveRelative(objectPosition, directionTowardPoint, glm::vec3(0.f, 0.f, -0.5f), false);
}

std::vector<NavPointConnection> aiAllPoints(NavGraph& navgraph){
  std::vector<NavPointConnection> points;
  for (auto [name, navConnections] : navgraph.connections){
    for (auto conn : navConnections){
      for (auto point : conn.points){
        points.push_back(NavPointConnection{
          .fromPoint = point.fromPoint,
          .toPoint = point.toPoint,
        });
      }
    }
  }
  return points;
}