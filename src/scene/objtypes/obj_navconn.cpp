#include "./obj_navconn.h"

GameObjectNavConns createNavConns(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectNavConns obj {
    .navgraph = createNavGraph(attr.stringAttributes),
  };
  return obj;
}
