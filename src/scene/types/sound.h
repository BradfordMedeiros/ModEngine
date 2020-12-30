#ifndef MOD_SOUNDMANAGER
#define MOD_SOUNDMANAGER

#include <map>
#include <vector>
#include <assert.h>
#include <iostream>
#include <AL/alut.h>
#include <iostream>
#include <stdexcept>
#include <cerrno>
#include <cstring>

// https://www.openal.org/documentation/OpenAL_Programmers_Guide.pdf

void startSoundSystem();
void stopSoundSystem();
ALuint loadSoundState(std::string filepath);
void unloadSoundState(ALuint source, std::string filepath);
void playSource(ALuint source);

std::vector<std::string> listSounds();
void setSoundPosition(ALuint source, float x, float y, float z);
void setListenerPosition(float x, float y, float z);

ALuint createBufferedAudio();
void freeBufferedAudio(ALuint buffer);
void playBufferedAudio(ALuint buffer);

#endif