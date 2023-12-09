#ifndef MOD_OBJ_NAVMESH
#define MOD_OBJ_NAVMESH

#include "../../common/util.h"
#include "./obj_util.h"
#include "../common/util/meshgen.h"

typedef std::function<std::vector<HitObject>(glm::vec3 pos, glm::quat direction, float maxDistance)> raycastFn;

std::optional<objid> targetNavmeshId(glm::vec3 target, raycastFn raycast, std::function<bool(objid)> isNavmesh);
glm::vec3 aiNavPosition(objid id, glm::vec3 target, std::function<glm::vec3(objid)> position, raycastFn raycast, std::function<bool(objid)> isNavmesh);

struct GameObjectNavmesh {
  Mesh mesh;
};

GameObjectNavmesh createNavmesh(GameobjAttributes& attr, ObjectTypeUtil& util);

#endif