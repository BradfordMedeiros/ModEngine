#include "./rails.h"


RailSystem createRailSystem(){
  std::map<std::string, RailNode> defaultRails;
  RailSystem rails { 
    .rails = defaultRails,
  };
  return rails;
}

void addRail(RailSystem& rails, std::string railName, std::string nodeFrom, std::string nodeTo){
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

NextRail nextPosition(RailSystem& rails, std::function<void(std::string)> positionForRail, std::string railName, glm::vec3 position, glm::vec3 direction){
  
}
