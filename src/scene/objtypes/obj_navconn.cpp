#include "./obj_navconn.h"

std::vector<AutoSerialize> navconnAutoserializer {
};

GameObjectNavConns createNavConns(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectNavConns obj {
    .navgraph = createNavGraph(attr.stringAttributes),
  };
  return obj;
}
