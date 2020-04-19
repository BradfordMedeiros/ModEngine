#include "./sound.h"

void startSoundSystem(){
  alutInit(NULL, NULL);
}
void stopSoundSystem(){
  alutExit();
}
void playSound(SoundInfo sound){
  alSourcePlay(sound.source);
}

// @todo support ogg file format.
// This call should support: .wav, .snd, .au , but have only tested .wav
SoundInfo loadSound(std::string filepath){
 std::cout << "EVENT: loading sound:" << filepath <<  std::endl; 
  
 ALuint soundBuffer = alutCreateBufferFromFile(filepath.c_str());

 ALenum error = alutGetError();
 if (error != ALUT_ERROR_NO_ERROR){
   std::cerr << "ERROR: " << alutGetErrorString(error) <<  ": " << std::strerror(errno) << std::endl;
   throw std::runtime_error("error loading buffer");
 }  

 ALuint soundSource;
 alGenSources(1, &soundSource);
 alSourcei(soundSource, AL_BUFFER, soundBuffer);

 SoundInfo handles {
  .source = soundSource,
  .buffer = soundBuffer,
 };
 return handles;
}

void unloadSound(SoundInfo sound){
  alDeleteSources(1, &sound.source);
  alDeleteBuffers(1, &sound.buffer);
}