#ifndef MOD_OBJ_EMITTER
#define MOD_OBJ_EMITTER

#include "../../../common/util.h"
#include "../obj_util.h"
#include "./emitter.h"

struct GameObjectEmitter{
	float rate;
	float duration;
	int numParticlesPerFrame;
	int limit;
	bool state;
  EmitterDeleteBehavior deleteBehavior;
};

GameObjectEmitter createEmitter(GameobjAttributes& attributes, ObjectTypeUtil& util);
void removeEmitterObj(GameObjectEmitter& heightmapObj, ObjectRemoveUtil& util);
bool setEmitterAttribute(GameObjectEmitter& obj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&);
std::optional<AttributeValuePtr> getEmitterAttribute(GameObjectEmitter& obj, const char* field);
std::vector<std::pair<std::string, std::string>> serializeEmitter(GameObjectEmitter& emitterObj, ObjectSerializeUtil& util);

EmitterSystem& getEmitterSystem();

#endif