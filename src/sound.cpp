#include <iostream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
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

// @todo support ogg file format.
// This call should support: .wav, .snd, .au , but have only tested .wav
ALuint loadSound(std::string filepath){
 std::cout << "EVENT: loading sound:" << filepath <<  std::endl; 
  
 ALuint soundBuffer = alutCreateBufferFromFile(filepath.c_str());

 ALenum error = alutGetError();
 if (error != ALUT_ERROR_NO_ERROR){
   std::cerr << "ERROR: " << alutGetErrorString(error) <<  ": " << std::strerror(errno) << std::endl;
   throw new std::runtime_error("error loading buffer");
 }  

 return soundBuffer;
}

