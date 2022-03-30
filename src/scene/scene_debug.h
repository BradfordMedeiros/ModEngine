#ifndef MOD_SCENEDEBUG
#define MOD_SCENEDEBUG

#include "./scene_sandbox.h"
#include "./physics.h"
#include "./object_types.h"
#include "./common/util/types.h"
#include "./common/util/boundinfo.h"

std::string scenegraphAsDotFormat(SceneSandbox& sandbox, std::map<objid, GameObjectObj>& objectMapping);
std::string debugAllGameObjects(SceneSandbox& sandbox);
std::string debugAllGameObjectsH(SceneSandbox& sandbox);
std::string debugTransformCache(SceneSandbox& sandbox);
std::string debugLoadedTextures(std::map<std::string, TextureRef>& textures);
std::string debugLoadedMeshes(std::map<std::string, MeshRef>& meshes);
std::string debugAnimations(std::map<objid, std::vector<Animation>>& animations);

void printPhysicsInfo(PhysicsInfo physicsInfo);
void dumpPhysicsInfo(std::map<objid, PhysicsValue>& rigidbodys);

#endif