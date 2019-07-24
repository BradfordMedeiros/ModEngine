#include <iostream>
#include "./sound.h"

void startSoundSystem(){
  alutInit(NULL, NULL);
}

void stopSoundSystem(){
  alutExit();
}


void playSound(ALuint soundBuffer){
  ALuint soundSource;
  alGenSources(1, &soundSource);
  alSourcei(soundSource, AL_BUFFER, soundBuffer);
  alSourcePlay(soundSource);
}

ALuint loadSound(std::string filepath){
  std::cout << "load sound placeholder" << std::endl; 
  ALuint soundBuffer = alutCreateBufferHelloWorld();
  return soundBuffer;
}

