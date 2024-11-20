#ifndef MOD_SCENEOBJECT
#define MOD_SCENEOBJECT

#include "./scene.h"

struct LightInfo {
  Transformation transform;
  GameObjectLight light;
  objid id;
};
std::vector<LightInfo> getLightInfo(World& world);

struct PortalInfo {
  Transformation cameraTransform;
  glm::vec3 portalPos;
  glm::quat portalRotation;
  bool perspective;
  objid id;
};
std::optional<PortalInfo> getPortalInfo(World& world, objid id);
std::vector<PortalInfo> getPortalInfo(World& world);
bool isPortal(World& world, objid id);
glm::mat4 renderPortalView(PortalInfo info, Transformation transform);
void maybeTeleportObjects(World& world, objid obj1Id, objid obj2Id);

bool isOctree(World& world, objid id);

std::optional<Texture> textureForId(World& world, objid id);

GameObjectCamera& getCamera(World& world, objid id);

std::optional<glm::vec3> aiNavigate(World& world, objid id, glm::vec3 target, std::function<void(glm::vec3, glm::vec3)> drawLine);

std::vector<HitObject> raycast(World& world, glm::vec3 posFrom, glm::quat direction, float maxDistance);
std::vector<HitObject> contactTest(World& world, objid id);
std::optional<Texture> textureForId(World& world, objid id);
void setObjectDimensions(World& world, std::vector<objid>& ids, float width, float height, float depth);

std::optional<objid> getIdForCollisionObject(World& world, const btCollisionObject* body);
bool idInGroup(World& world, objid id, std::vector<objid> groupIds);

void emit(World& world, objid id, NewParticleOptions particleOpts);

void createGeneratedMesh(World& world, std::vector<glm::vec3>& face, std::vector<glm::vec3>& points, std::string destMesh);

struct TextureAndName {
  Texture texture;
  std::string textureName;
};
std::vector<TextureAndName> worldTextures(World& world);
void setTexture(World& world, objid index, std::string textureName, std::function<void(unsigned int)> setNavmeshTextureId);
std::optional<std::string> lookupNormalTexture(World& world, std::string textureName);

bool isPrefab(World& world, objid id);
std::optional<objid> prefabId(World& world, objid id);

#endif