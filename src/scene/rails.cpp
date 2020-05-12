#include "./rails.h"

RailSystem createRailSystem(){
  RailSystem rails { };
  return rails;
}

void addRail(RailSystem& rails, std::string nodeFrom, std::string nodeTo){

}

void removeRail(RailSystem& rails, std::string nodeFrom, std::string nodeTo){
  
}

NextRail nextPosition(RailSystem& rails, std::function<void(std::string)> positionForRail, std::string railName, glm::vec3 position, glm::vec3 direction){

}
