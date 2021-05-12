#ifndef MOD_SCENEOBJECT
#define MOD_SCENEOBJECT

#include "./scene.h"

struct LightInfo {
  glm::vec3 pos;
  glm::quat rotation;
  GameObjectLight light;
};

std::vector<LightInfo> getLightInfo(World& world);

struct PortalInfo {
  glm::vec3 cameraPos;
  glm::quat cameraRotation;
  glm::vec3 portalPos;
  glm::quat portalRotation;
  bool perspective;
  objid id;
};
PortalInfo getPortalInfo(World& world, objid id);
std::vector<PortalInfo> getPortalInfo(World& world);
bool isPortal(World& world, objid id);
std::optional<GameObjectVoxel*> getVoxel(World& world, objid id);

std::optional<Texture> textureForId(World& world, objid id);

void applyHeightmapMasking(World& world, objid id, float amount, float uvx, float uvy, bool shouldAverage);
void saveHeightmap(World& world, objid id);

glm::vec3 aiNavigate(World& world, objid id, glm::vec3 target);

std::vector<HitObject> raycast(World& world, glm::vec3 posFrom, glm::quat direction, float maxDistance);
std::optional<Texture> textureForId(World& world, objid id);

objid getIdForCollisionObject(World& world, const btCollisionObject* body);

bool idInGroup(World& world, objid id, std::vector<objid> groupIds);

void emit(World& world, objid id);

void enforceAllLayouts(World& world);

#endif