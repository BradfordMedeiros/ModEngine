#include "./obj_navmesh.h"


struct NavPlane {  // all points on navplane is reachable by straight line by another point in it (eg should be convex shape)
  objid id;
  std::vector<glm::vec3> vertices;
};

struct Edge {
  glm::vec3 from;
  glm::vec3 to;
};

struct NavPlaneConnection {
  objid from;
  objid to;
  glm::vec3 connectionPoint;
};

struct NavWorld {
  std::vector<NavPlane> navPlanes;
  std::vector<NavPlaneConnection> navplaneConnections;
};

std::vector<glm::vec3> values = {
  glm::vec3(0.f, 0.f, 0.f),
  glm::vec3(0.f, 1.f, 0.f),
  glm::vec3(0.f, 0.f, -1.f),
};


NavWorld navWorld {
  .navPlanes = {},
  .navplaneConnections = {},
};

std::optional<objid> targetNavmeshId(glm::vec3 target, raycastFn raycast, std::function<bool(objid)> isNavmesh){
  auto abitAbove = glm::vec3(target.x, target.y + 1, target.z);
  auto direction = orientationFromPos(abitAbove, target);
  auto hitObjects = raycast(abitAbove, direction, 5);
  for (auto targetObject : hitObjects){
    if (isNavmesh(targetObject.id)){
      return targetObject.id;
    }
  }
  return std::nullopt;
}

glm::vec3 aiNavPosition(objid id, glm::vec3 target, std::function<glm::vec3(objid)> position, raycastFn raycast, std::function<bool(objid)> isNavmesh){
  auto objectPosition = position(id);
  auto directionTowardPoint = orientationFromPos(objectPosition, target);
  return moveRelative(objectPosition, directionTowardPoint, glm::vec3(0.f, 0.f, -0.5f), false);
}

std::optional<NavPlane*> getNavPlane(objid navplaneId){
 for (int i = 0; i < navWorld.navPlanes.size(); i++){
    if (navWorld.navPlanes.at(i).id == navplaneId){
      return &navWorld.navPlanes.at(i);
    }
  }
  return std::nullopt;
}
std::optional<std::vector<glm::vec3>> valuesFromNavplane(objid navplaneId){
  auto navplane = getNavPlane(navplaneId);
  if (!navplane.has_value()){
    return std::nullopt;
  }
  return navplane.value() -> vertices;
}
void addTriangleToNavPlane(objid navplaneId, glm::vec3 point1, glm::vec3 point2, glm::vec3 point3){
  auto navplane = getNavPlane(navplaneId);
  modassert(navplane.has_value(), std::string("no navplane for id: ") + std::to_string(navplaneId));
  navplane.value() -> vertices.push_back(point1);
  navplane.value() -> vertices.push_back(point2);
  navplane.value() -> vertices.push_back(point3);
}

void addNavplane(objid navplaneId){
  modassert(!getNavPlane(navplaneId).has_value(), "naveplane already exists");
  navWorld.navPlanes.push_back(NavPlane {
    .id = navplaneId,
    .vertices = {},
  });
}

void removeConnectionForNavplane(objid navplaneId){
  std::vector<NavPlaneConnection> navplaneConnections;
  for (auto &connection : navWorld.navplaneConnections){
    if (connection.from == navplaneId || connection.to == navplaneId){
      continue;
    }
    navplaneConnections.push_back(connection);
  }
  navWorld.navplaneConnections = navplaneConnections;
}

void removeNavplane(objid navplaneId){
  removeConnectionForNavplane(navplaneId);
  std::vector<NavPlane> navPlanes;
  for (auto &navplane : navWorld.navPlanes){
    if (navplane.id == navplaneId){
      continue;
    }
    navPlanes.push_back(navplane);
  }
  navWorld.navPlanes = navPlanes;
}

std::vector<Edge> allEdgesForNavplane(NavPlane& navplane){
  std::vector<Edge> edges;
  for (int i = 0; i < navplane.vertices.size(); i+=3){
    edges.push_back(Edge {
      .from = navplane.vertices.at(i),
      .to = navplane.vertices.at(i + 1),
    });
    edges.push_back(Edge {
      .from = navplane.vertices.at(i),
      .to = navplane.vertices.at(i + 2),
    });
    edges.push_back(Edge {
      .from = navplane.vertices.at(i + 1),
      .to = navplane.vertices.at(i + 2),
    });
  }
  return edges;
}

bool edgesAreEqual(Edge& edge1, Edge& edge2){  // would be nice if vertexs don't have to be equal but just one edge contains the other
  return (
    aboutEqual(edge1.from, edge2.from) && aboutEqual(edge1.to, edge2.to) ||
    aboutEqual(edge1.from, edge2.to) && aboutEqual(edge1.to, edge2.from)
  );
}
bool hasCommonEdge(std::vector<Edge>& edge1, std::vector<Edge>& edge2){
  for (int i = 0; i < edge1.size(); i++){
    for (int j = 0; j < edge2.size(); j++){
      if (edgesAreEqual(edge1.at(i), edge2.at(j))){
        return true;
      }
    }
  }
  return false;
}

void calculateConnections(){
  navWorld.navplaneConnections = {};
  for (int i = 0; i < navWorld.navPlanes.size(); i++){
    for (int j = i + 1; j < navWorld.navPlanes.size(); j++){
      auto edges1 = allEdgesForNavplane(navWorld.navPlanes.at(i));
      auto edges2 = allEdgesForNavplane(navWorld.navPlanes.at(j));
      auto connects = hasCommonEdge(edges1, edges2);
      if (connects){
        navWorld.navplaneConnections.push_back(NavPlaneConnection {
          .from = navWorld.navPlanes.at(i).id,
          .to = navWorld.navPlanes.at(j).id,
          .connectionPoint = glm::vec3(0.f, 0.f, 0.f), // obviously incorrect
        });
        continue;
      }
    }

  }
  // guess go through all the vertices in a navplane, and 

  // check if the edge is shared
  // or the edge is a subset of the other edge i guess? 
      // if they're parralel than it is, then just do discrete logic to determine the partition
}


Mesh createNavmeshFromPointList(std::vector<glm::vec3> points, ObjectTypeUtil& util){
  std::vector<Vertex> vertices;
  for (int i = 0; i < points.size(); i++){
    int triangleIndex = i % 3;
    if (triangleIndex == 0){
      vertices.push_back(createVertex(points.at(i), glm::vec2(0.f, 0.f)));  // maybe the tex coords should just be calculated as a ratio to a fix texture
      continue;
    }else if (triangleIndex == 1){
      vertices.push_back(createVertex(points.at(i), glm::vec2(1.f, 0.f)));
      continue;
    }else if (triangleIndex == 2){
      vertices.push_back(createVertex(points.at(i), glm::vec2(0.f, 1.f)));
      continue;
    }
    modassert(false, "invalid triangleIndex");
  }
  std::vector<unsigned int> indices;
  for (int i = 0; i < vertices.size(); i++){ // should be able to optimize this better...
    indices.push_back(i);
  }
  MeshData meshData {
    .vertices = vertices,
    .indices = indices,
    .diffuseTexturePath = "./res/textures/wood.jpg",
    .hasDiffuseTexture = true,
    .emissionTexturePath = "",
    .hasEmissionTexture = false,
    .opacityTexturePath = "",
    .hasOpacityTexture = false,
    .boundInfo = getBounds(vertices),
    .bones = {},
  };
  return util.loadMesh(meshData);
}

std::string print(NavWorld& navWorld){
  std::string str = "navworld = ";
  str += std::string("size = ") + std::to_string(navWorld.navPlanes.size()) + "\n";

  str += std::string("navplanes = [") + "\n";
  for (auto &navplane : navWorld.navPlanes){
    str += std::string("   ") + std::to_string(navplane.id) + "\n";
    str += std::string("   ") + "vertices =  [\n";
    for (auto &vertex : navplane.vertices){
      str += "      " + print(vertex) + "\n";
    }
    str += "   ]\n";

    auto edges = allEdgesForNavplane(navplane);
    str += std::string("   ") + "edges =  [\n";
    for (auto &edge : edges){
      str += "      " + print(edge.from) + " | " + print(edge.to) + "\n";
    }

    str += "   ]\n\n";
  }
  str += "\n]";

  str += "\nconnections = [";
  for (auto &connection : navWorld.navplaneConnections){
    str += std::string("(") + std::to_string(connection.from) + " - " + std::to_string(connection.to) + ")";
  }
  str += "\n]";

  return str;
}

void printNavmeshDebug(){
  std::cout << std::endl << print(navWorld) << std::endl << std::endl;
}


std::vector<glm::vec3> mesh1Points = {
  glm::vec3(0.f, 0.f, 0.f),
  glm::vec3(2.f, 0.f, 0.f),
  glm::vec3(0.f, 0.f, -2.f),

  glm::vec3(0.f, 0.f, -2.f),
  glm::vec3(2.f, 0.f, 0.f),
  glm::vec3(2.f, 0.f, -2.f),
};

std::vector<glm::vec3> mesh2Points = {
  glm::vec3(2.f, 0.f, 0.f),
  glm::vec3(4.f, 0.f, 0.f),
  glm::vec3(2.f, 0.f, -2.f),
};


std::vector<AutoSerialize> navmeshAutoserializer {
};

int navmeshId = 0;
GameObjectNavmesh createNavmesh(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto points = navmeshId == 0 ? mesh1Points : mesh2Points;
  if (navmeshId > 1){
    assert(false);
  }
  navmeshId++;

  addNavplane(util.id);
  for (int i = 0; i < points.size(); i+= 3){
    addTriangleToNavPlane(util.id, points.at(i), points.at(i + 1), points.at(i + 2));
  }
  calculateConnections();
  
  GameObjectNavmesh obj {
    .mesh = createNavmeshFromPointList(points, util),
  };
  return obj;
}