#include "./obj_navmesh.h"

std::vector<AutoSerialize> navmeshAutoserializer {
};

GameObjectNavmesh createNavmesh(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectNavmesh obj {
    .mesh = util.meshes.at("./res/models/ui/node.obj").mesh,
  };
  return obj;
}