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

NextRail nextPosition(RailSystem& rails, std::function<glm::vec3(std::string)> positionForRail, std::string railName, glm::vec3 position, glm::quat direction){
  auto currentRail = rails.rails.at(railName);
  auto fromPos = positionForRail(currentRail.from);
  auto toPos = positionForRail(currentRail.to);

  auto newPosition = glm::vec3(position.x + 0.2f, position.y, position.z);
  NextRail rail {
    .position = newPosition,
    .rail = railName
  };

  return rail;
}
