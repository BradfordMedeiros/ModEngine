#ifndef MOD_SCENEOBJECT
#define MOD_SCENEOBJECT

#include "./scene.h"

struct LightInfo {
  Transformation transform;
  GameObjectLight light;
};
std::vector<LightInfo> getLightInfo(World& world);

struct PortalInfo {
  Transformation cameraTransform;
  glm::vec3 portalPos;
  glm::quat portalRotation;
  bool perspective;
  objid id;
};
PortalInfo getPortalInfo(World& world, objid id);
std::vector<PortalInfo> getPortalInfo(World& world);
bool isPortal(World& world, objid id);
glm::mat4 renderPortalView(PortalInfo info, Transformation transform);
void maybeTeleportObjects(World& world, objid obj1Id, objid obj2Id);

std::optional<GameObjectVoxel*> getVoxel(World& world, objid id);
bool isVoxel(World& world, objid id);
void handleVoxelRaycast(World& world, objid id, glm::vec3 fromPos, glm::vec3 toPosDirection, int textureId);

std::optional<Texture> textureForId(World& world, objid id);

void applyHeightmapMasking(World& world, objid id, float amount, float uvx, float uvy, bool shouldAverage);
GameObjectHeightmap& getHeightmap(World& world, objid id);
void saveHeightmap(World& world, objid id, std::string filepath);
bool isHeightmap(World& world, objid id);

GameObjectCamera& getCamera(World& world, objid id);

glm::vec3 aiNavigate(World& world, objid id, glm::vec3 target);

std::vector<HitObject> raycast(World& world, glm::vec3 posFrom, glm::quat direction, float maxDistance);
std::optional<Texture> textureForId(World& world, objid id);
void setObjectDimensions(World& world, std::vector<objid>& ids, float width, float height, float depth);

objid getIdForCollisionObject(World& world, const btCollisionObject* body);
bool idInGroup(World& world, objid id, std::vector<objid> groupIds);

void emit(World& world, objid id, NewParticleOptions particleOpts);

void enforceAllLayouts(World& world);

void createGeneratedMesh(World& world, std::vector<glm::vec3>& face, std::vector<glm::vec3>& points, std::string destMesh);

struct TextureAndName {
  Texture texture;
  std::string textureName;
};
std::vector<TextureAndName> worldTextures(World& world);
void setTexture(World& world, objid index, std::string textureName);

#endif