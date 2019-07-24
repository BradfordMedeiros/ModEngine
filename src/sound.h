#ifndef MOD_SOUND
#define MOD_SOUND

#include <iostream>
#include <AL/alut.h>

void startSoundSystem();
void stopSoundSystem();
ALuint loadSound(std::string filepath);
void playSound(ALuint soundBuffer);

#endif 
