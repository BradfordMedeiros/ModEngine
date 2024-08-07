#ifndef MOD_OBJ_SOUND
#define MOD_OBJ_SOUND

#include "../../../common/util.h"
#include "./sound.h"
#include "../obj_util.h"

struct GameObjectSound{
  std::string clip;  
  ALuint source;
  bool loop;
  bool center;
  float volume;
};

GameObjectSound createSound(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeSound(GameObjectSound& obj, ObjectSerializeUtil& util);
void removeSound(GameObjectSound& soundObj, ObjectRemoveUtil& util);
std::optional<AttributeValuePtr> getSoundAttribute(GameObjectSound& obj, const char* field);
bool setSoundAttribute(GameObjectSound& obj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&);

#endif