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
void printPhysicsInfo(PhysicsInfo physicsInfo);
void dumpPhysicsInfo(std::map<objid, btRigidBody*>& rigidbodys);

#endif