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
  for (auto [railName, connection] : rails.rails){
    if (railName != exclude && (connection.from == node || connection.to == node)){
      return railName;
    }
  }
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
  //bool inDirectionOfRail = glm::dot(railDirection, quatToVec(direction)) >= 0;
  bool inDirectionOfRail = true;
  auto newPosition = position + (inDirectionOfRail ? 1.0f : -1.0f) * railDirection;   // might be interesting to allow the dot value to actually be used to determine speed as well 
  auto atEndpoint = glm::distance(toRail, position) < 0.1;                            // If it's negative this can be the fromRail, also can be weird if rail moves while this is happening. 

  NextRail rail {
    .position = newPosition,
    .rail = atEndpoint ? railForNode(rails, currentRail.to, railName) : railName
  };

  return rail;
}

bool entityExists(RailSystem& rails, short id){
  for (auto entity : rails.activeRails){
    if (entity.id == id){
      return true;
    }
  }
  return false;
}
bool railExists(RailSystem& rails, std::string railName){
  for (auto [rail, _] : rails.rails){
    if (rail == railName){
      return true;
    }
  }
  return false;
}
void addEntity(RailSystem& rails, short id, std::string railName){
  if(entityExists(rails, id)){
    removeEntity(rails, id);
  }
  assert(railExists(rails, railName));
  rails.activeRails.push_back(ActiveRail{
    .id = id,
    .rail = railName,
  });
}
void removeEntity(RailSystem& rails, short id){
  assert(entityExists(rails, id));
  std::vector<ActiveRail> newRails;
  for (auto activeRail : rails.activeRails){
    if (activeRail.id != id){
      newRails.push_back(activeRail);
    }
  }
  rails.activeRails = newRails;
}
