#ifndef MOD_SOUND
#define MOD_SOUND

#include <iostream>
#include <AL/alut.h>
#include <iostream>
#include <stdexcept>
#include <cerrno>
#include <cstring>

// https://www.openal.org/documentation/OpenAL_Programmers_Guide.pdf

struct SoundInfo {
  ALuint source;
  ALuint buffer;
};
void startSoundSystem();
void stopSoundSystem();
SoundInfo loadSound(std::string filepath);
void unloadSound(SoundInfo sound);
void playSound(SoundInfo sound);
void setSoundPosition(ALuint source, float x, float y, float z);

#endif 
