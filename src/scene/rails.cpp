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

  auto railDirection = toPos - fromPos;
  auto moveDirection = quatToVec(direction);
  float value = glm::dot(railDirection, moveDirection);
  bool positiveDirection = value > 0;

  std::cout << "from: " << print(fromPos) << std::endl;
  std::cout << "to: " << print(toPos) << std::endl;

  std::cout << "value: " << value << std::endl;
  std::cout << "positiveDirection: " << (positiveDirection ? "true" : "false") << std::endl;

  auto newPosition = glm::vec3(position.x + (positiveDirection ? 0.2f : -0.2f), position.y, position.z);
  NextRail rail {
    .position = newPosition,
    .rail = railName
  };

  return rail;
}
