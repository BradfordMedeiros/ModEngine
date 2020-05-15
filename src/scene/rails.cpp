#include "./rails.h"


RailSystem createRailSystem(){
  std::map<std::string, RailNode> defaultRails;
  RailSystem rails { 
    .rails = defaultRails,
  };
  return rails;
}

void addRail(RailSystem& rails, std::string railName, std::string nodeFrom, std::string nodeTo){
  assert (rails.rails.find(railName) == rails.rails.end());
  rails.rails[railName] = RailNode {
    .from = nodeFrom,
    .to = nodeTo
  };
}

void removeRail(RailSystem& rails, std::string railName){
  assert(rails.rails.find(railName) != rails.rails.end());
  rails.rails.erase(railName);
}

std::vector<std::string> railnames(RailSystem& rails){
  std::vector<std::string> railNames;
  for (auto [name, _] : rails.rails){
    railNames.push_back(name);
  }
  return railNames;
}

// return first rail found that has a rail connecting it
std::string railForNode(RailSystem& rails, std::string node, std::string exclude){
  return "";
}

NextRail nextPosition(
  RailSystem& rails, 
  std::function<glm::vec3(std::string)> positionForRail, 
  std::string railName, 
  glm::vec3 position, 
  glm::quat direction
){
  auto currentRail = rails.rails.at(railName);
  auto toRail = positionForRail(currentRail.to);
  auto fromRail = positionForRail(currentRail.from);
  
  bool isNullRail = (toRail.x == fromRail.x) && (toRail.y == fromRail.y) && (toRail.z == fromRail.z);
  if (isNullRail){
    return NextRail{
      .position = position,
      .rail = railName
    };
  }  

  auto railDirection = glm::normalize(toRail - fromRail);
  bool inDirectionOfRail = glm::dot(railDirection, quatToVec(direction)) >= 0;
  auto newPosition = position + (inDirectionOfRail ? 0.1f : -0.1f) * railDirection;   // might be interesting to allow the dot value to actually be used to determine speed as well 
 
  auto atEndpoint = glm::distance(toRail, position) < 0.01;   // If it's negative this can be the fromRail

  NextRail rail {
    .position = newPosition,
    .rail = atEndpoint ? railForNode(rails, currentRail.to, railName) : railName
  };

  return rail;
}
