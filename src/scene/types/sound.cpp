#include "./sound.h"

static std::map<std::string, ALuint> soundBuffers;  
static std::map<std::string, int> soundUsages;

void startSoundSystem(){
  alutInit(NULL, NULL);
}
void stopSoundSystem(){
  alutExit();
}

void playSource(ALuint source){
  alSourcePlay(source);
}
std::vector<std::string> listSounds(){
  std::vector<std::string> sounds;
  for (auto [soundname, _ ] : soundBuffers){
    sounds.push_back(soundname);
  }
  return sounds;
}
void setSoundPosition(ALuint source, float x, float y, float z){
  alSource3f(source, AL_POSITION, x, y, z);
}

int getUsages(std::string filepath){
  if (soundUsages.find(filepath) == soundUsages.end()){
    return 0;
  }
  return soundUsages.at(filepath);
}

ALuint findOrLoadBuffer(std::string filepath){
  if (soundBuffers.find(filepath) != soundBuffers.end()){
    return soundBuffers.at(filepath);
  }
  ALuint soundBuffer = alutCreateBufferFromFile(filepath.c_str());
  ALenum error = alutGetError();
  if (error != ALUT_ERROR_NO_ERROR){
    std::cerr << "ERROR: " << alutGetErrorString(error) <<  ": " << std::strerror(errno) << std::endl;
    throw std::runtime_error("error loading buffer");
  }
  soundBuffers[filepath] = soundBuffer;
  return soundBuffer;  
}

ALuint createSource(ALuint soundBuffer){
  ALuint soundSource;
  alGenSources(1, &soundSource);
  alSourcei(soundSource, AL_BUFFER, soundBuffer);  
  return soundSource;
}

// @todo support ogg file format.
// This call should support: .wav, .snd, .au , but have only tested .wav
ALuint loadSoundState(std::string filepath){
  std::cout << "EVENT: loading sound:" << filepath <<  std::endl; 

  ALuint soundBuffer = findOrLoadBuffer(filepath);
  ALuint soundSource = createSource(soundBuffer);
  
  if (soundUsages.find(filepath) == soundUsages.end()){
    soundUsages[filepath] = 0;
  }
  soundUsages[filepath] = soundUsages[filepath] + 1;
  return soundSource;
}

void unloadSoundState(ALuint source,  std::string filepath){
  int usages = getUsages(filepath);
  assert(usages > 0);
  alDeleteSources(1, &source);
  if (usages == 1){
    soundUsages.erase(filepath);
    ALuint buffer = soundBuffers.at(filepath);
    alDeleteBuffers(1, &buffer);
    soundBuffers.erase(filepath);
  }else{
    soundUsages[filepath] = soundUsages[filepath] - 1;
  }
}
