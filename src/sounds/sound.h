#ifndef MOD_SOUND
#define MOD_SOUND

#include <iostream>
#include <AL/alut.h>
#include <iostream>
#include <stdexcept>
#include <cerrno>
#include <cstring>

struct SoundInfo {
  ALuint source;
  ALuint buffer;
};
void startSoundSystem();
void stopSoundSystem();
SoundInfo loadSound(std::string filepath);
void unloadSound(SoundInfo sound);
void playSound(SoundInfo sound);

#endif 
