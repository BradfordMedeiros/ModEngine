#ifndef MOD_RAILS
#define MOD_RAILS

#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <functional>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

#include <iostream>
#include "../common/util.h"

struct RailNode {
  std::string from;
  std::string to;
};

struct RailSystem {
  std::map<std::string, RailNode> rails;
};

RailSystem createRailSystem();
void addRail(RailSystem& rails, std::string railName, std::string nodeFrom, std::string nodeTo);
void removeRail(RailSystem& rails, std::string railName);
std::vector<std::string> railnames(RailSystem& rails);

struct NextRail {
  glm::vec3 position;
  std::string rail;
};

NextRail nextPosition(RailSystem& rails, std::function<glm::vec3(std::string)> positionForRail, std::string railName, glm::vec3 position, glm::quat direction); 

#endif