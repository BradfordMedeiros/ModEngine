#ifndef MOD_OBJ_EMITTER
#define MOD_OBJ_EMITTER

#include "../../common/util.h"
#include "./obj_util.h"
#include "../types/emitter.h"

struct GameObjectEmitter{
	float rate;
	float duration;
	int limit;
	bool state;
  EmitterDeleteBehavior deleteBehavior;
  std::string templateName;
};

GameObjectEmitter createEmitter(GameobjAttributes& attributes, ObjectTypeUtil& util);
void removeEmitterObj(GameObjectEmitter& heightmapObj, ObjectRemoveUtil& util);
bool setEmitterAttributes(GameObjectEmitter& emitterObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);
bool setEmitterAttribute(GameObjectEmitter& obj, const char* field, AttributeValue value, ObjectSetAttribUtil& util);
std::optional<AttributeValuePtr> getEmitterAttribute(GameObjectEmitter& obj, const char* field);
void emitterObjAttr(GameObjectEmitter& emitterObj, GameobjAttributes& _attributes);
std::vector<std::pair<std::string, std::string>> serializeEmitter(GameObjectEmitter& emitterObj, ObjectSerializeUtil& util);

#endif