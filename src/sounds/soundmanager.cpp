#include "./soundmanager.h"

static std::map<std::string, SoundInfo> managedSounds;  
static std::map<std::string, int> soundUsages;

int getUsages(std::string filepath){
  if (soundUsages.find(filepath) == soundUsages.end()){
    return 0;
  }
  return soundUsages.at(filepath);
}

void loadSoundState(std::string filepath){
  int usages = getUsages(filepath);
  if (usages == 0){
    SoundInfo sound = loadSound(filepath);
    managedSounds[filepath] = sound;
  }
  if (soundUsages.find(filepath) == soundUsages.end()){
    soundUsages[filepath] = 0;
  }
  soundUsages[filepath] = soundUsages[filepath] + 1;
}
void unloadSoundState(std::string filepath){
  int usages = getUsages(filepath);
  assert(usages > 0);
  if (usages == 1){
    soundUsages.erase(filepath);
    unloadSound(managedSounds.at(filepath));
    managedSounds.erase(filepath);
  }else{
    soundUsages[filepath] = soundUsages[filepath] - 1;
  }
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