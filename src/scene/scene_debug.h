#ifndef MOD_SCENEDEBUG
#define MOD_SCENEDEBUG

#include "./scene_sandbox.h"
#include "./physics.h"
#include "./object_types.h"
#include "./common/util/types.h"
#include "./common/util/boundinfo.h"

struct DotInfo {
  std::string name;
  objid id;
  objid sceneId;
  objid groupId;
  glm::vec3 position;
  glm::vec3 scale;
  glm::quat rotation;
  std::vector<std::string> meshes;
};
struct DotInfos {
  DotInfo child;
  std::optional<DotInfo> parent;
};
std::vector<DotInfos> getDotRelations(SceneSandbox& sandbox, std::map<objid, GameObjectObj>& objectMapping);
std::string scenegraphAsDotFormat(SceneSandbox& sandbox, std::map<objid, GameObjectObj>& objectMapping);

std::string debugAllGameObjects(SceneSandbox& sandbox);
std::string debugAllGameObjectsH(SceneSandbox& sandbox);
std::string debugTransformCache(SceneSandbox& sandbox);
std::string debugLoadedTextures(std::map<std::string, TextureRef>& textures);
std::string debugLoadedMeshes(std::map<std::string, MeshRef>& meshes);
std::string debugAnimations(std::map<objid, std::vector<Animation>>& animations);
std::string debugPhysicsInfo(std::map<objid, PhysicsValue>& rigidbodys);

void printPhysicsInfo(PhysicsInfo physicsInfo);

#endif