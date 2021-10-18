#ifndef MOD_OBJ_SOUND
#define MOD_OBJ_SOUND

#include "../../common/util.h"
#include "../types/sound.h"

struct GameObjectSound{
  std::string clip;  
  ALuint source;
  bool loop;
};

GameObjectSound createSound(GameobjAttributes& attr);

#endif