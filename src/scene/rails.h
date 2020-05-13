#ifndef MOD_RAILS
#define MOD_RAILS

#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <functional>

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

// given a specified rail, and a direction vector, give me the next position.
// based on direction vector if distance > deltaX , take a speed + direction and expand for new distance
// else expand it's nodes and then based on direction vector determine new to and from nodes

struct NextRail {
  glm::vec3 position;
  std::string rail;
};

NextRail nextPosition(RailSystem& rails, std::function<void(std::string)> positionForRail, std::string railName, glm::vec3 position, glm::vec3 direction); 

#endif