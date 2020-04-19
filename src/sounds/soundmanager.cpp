#include "./soundmanager.h"

static std::map<std::string, SoundInfo> managedSounds;  

void loadSoundState(std::string filepath){
  SoundInfo sound = loadSound(filepath);
  managedSounds[filepath] = sound;
}
void unloadSoundState(std::string filepath){
 // unloadSound(managedSounds.at(filepath));
 // managedSounds.erase(filepath);
 // @TODO need to do reference counting basically, so going to hold off here for now
}
void playSoundState(std::string filepath){
  playSound(managedSounds.at(filepath));
}

std::vector<std::string> listSounds(){
  std::vector<std::string> sounds;
  for (auto [soundname, _ ] : managedSounds){
    sounds.push_back(soundname);
  }
  return sounds;
}