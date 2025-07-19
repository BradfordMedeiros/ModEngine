#ifndef MOD_SCENEOBJECT
#define MOD_SCENEOBJECT

#include "./scene.h"

struct LightInfo {
  Transformation transform;
  glm::mat4 transformMatrix;
  GameObjectLight light;
  objid id;
};
std::vector<LightInfo> getLightInfo(World& world);
int getLightsArrayIndex(std::vector<LightInfo>& lights, objid lightId);

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
void createGeneratedMeshRaw(World& world, std::vector<glm::vec3>& verts, std::vector<glm::vec2>& uvCoords, std::vector<unsigned int>& indexs, std::string destMesh);

struct TextureAndName {
  Texture texture;
  std::string textureName;
};
std::vector<TextureAndName> worldTextures(World& world);
void setTexture(World& world, objid index, std::string textureName, std::function<void(unsigned int)> setNavmeshTextureId);
std::optional<std::string> lookupNormalTexture(World& world, std::string textureName);

bool isPrefab(World& world, objid id);
std::optional<objid> prefabId(World& world, objid id);


LayerInfo& layerByName(World& world, std::string layername);
RenderStagesDofInfo getDofInfo(World& world, bool* _shouldRender, GameObjectCamera* activeCameraData, glm::mat4 view);


// octree ////////////
void doOctreeRaycast(World& world, objid id, glm::vec3 fromPos, glm::vec3 toPos, bool alt);
void setPrevOctreeTexture();
void setNextOctreeTexture();
void loadOctree(World& world, objid selectedIndex);
void saveOctree(World& world, objid selectedIndex);
void writeOctreeTexture(World& world, objid selectedIndex, bool unitTexture);
std::vector<TagInfo> getTag(World& world, int tag, glm::vec3 position);
std::vector<TagInfo> getAllTags(World& world, int tag);
GameObjectOctree* getMainOctree(World& world, objid* id);
std::optional<OctreeMaterial> getMaterial(World& world, glm::vec3 position);

#endif