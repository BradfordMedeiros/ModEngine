#ifndef MOD_AINAV
#define MOD_AINAV

#include <map>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include "./common/util.h"
#include "./translations.h"

struct NavPointConnection {
  glm::vec3 fromPoint;
  glm::vec3 toPoint;
};

struct NavConnection {
  std::string destination;
  std::vector<NavPointConnection> points;
};

struct NavGraph {
  std::map<std::string, std::vector<NavConnection>> connections;
};

struct aiSearchResult {
  bool found;
  std::vector<std::string> path;
};

NavGraph createNavGraph();
aiSearchResult aiNavSearchPath(NavGraph& navgraph, std::string from, std::string to);
glm::vec3 aiNavPosition(
  objid id, 
  glm::vec3 target,
  std::function<glm::vec3(objid, bool)> position,
  std::function<std::vector<HitObject>(glm::vec3 pos, glm::quat direction, float maxDistance)> raycast,
  std::function<bool(objid)> isNavmesh
);


#endif