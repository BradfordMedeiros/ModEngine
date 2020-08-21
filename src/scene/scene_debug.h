#ifndef MOD_SCENEDEBUG
#define MOD_SCENEDEBUG

#include "./scenegraph.h"
#include "./physics.h"
#include "./object_types.h"
#include "./common/util/types.h"
#include "./common/util/boundinfo.h"

std::string scenegraphAsDotFormat(Scene& scene, std::map<objid, GameObjectObj>& objectMapping);
void printPhysicsInfo(PhysicsInfo physicsInfo);
void dumpPhysicsInfo(std::map<objid, btRigidBody*>& rigidbodys);

#endif