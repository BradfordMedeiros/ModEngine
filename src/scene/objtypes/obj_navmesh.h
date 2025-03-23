#ifndef MOD_OBJ_NAVMESH
#define MOD_OBJ_NAVMESH

#include "../../common/util.h"
#include "./obj_util.h"
#include "../common/util/meshgen.h"

typedef std::function<std::vector<HitObject>(glm::vec3 pos, glm::quat direction, float maxDistance)> raycastFn;

std::optional<objid> targetNavmeshId(glm::vec3 target, raycastFn raycast, std::function<bool(objid)> isNavmesh);
glm::vec3 aiNavPosition(objid id, glm::vec3 target, std::function<glm::vec3(objid)> position, raycastFn raycast, std::function<bool(objid)> isNavmesh);

struct NavPathElement {
  objid navplaneId;
  glm::vec3 point;
};
std::optional<std::vector<NavPathElement>> findNavplanePath(objid navplaneFrom, objid navplaneTo);

void drawNavplanePath(std::vector<NavPathElement>& navElements, std::function<void(glm::vec3, glm::vec3)> drawLine);

struct GameObjectNavmesh {
  std::vector<Mesh> meshes;
};

GameObjectNavmesh createNavmesh(GameobjAttributes& attr, ObjectTypeUtil& util);
void removeNavmesh(GameObjectNavmesh& navmeshObj, ObjectRemoveUtil& util);
void drawControlPoints(objid navmeshId, std::function<void(glm::vec3)> drawPoint);
std::optional<AttributeValuePtr> getNavmeshAttribute(GameObjectNavmesh& obj, const char* field);

void printNavmeshDebug();

#endif