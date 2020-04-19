#ifndef MOD_SOUNDMANAGER
#define MOD_SOUNDMANAGER

#include <map>
#include <vector>
#include <assert.h>
#include "./sound.h"

void loadSoundState(std::string filepath);
void unloadSoundState(std::string filepath);
void playSoundState(std::string filepath);
std::vector<std::string> listSounds();

#endif