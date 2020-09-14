#include "./emitter.h"

void addEmitter(EmitterSystem& system, std::string name, float currentTime){
  Emitter emitter {
    .name = name,
    .initTime = currentTime,
  };
  system.emitters.push_back(emitter);
}

struct EmitterFilterResult {
  std::vector<Emitter> removedEmitters;
  std::vector<Emitter> remainingEmitters;
};

EmitterFilterResult filterEmitters(EmitterSystem& system, std::vector<std::string> toRemove){
  std::vector<Emitter> removedEmitters;
  std::vector<Emitter> remainingEmitters;
  for (auto emitter : system.emitters){
    bool shouldRemove = false;
    for (auto emitterToRemove : toRemove){
      if (emitter.name == emitterToRemove){
        shouldRemove = true;
        break;
      }
    }
    if (shouldRemove){
      removedEmitters.push_back(emitter);
    }else{
      remainingEmitters.push_back(emitter);
    }
  }
  
  EmitterFilterResult result {
    .removedEmitters = removedEmitters,
    .remainingEmitters = remainingEmitters,
  };
  return result;
}

void removeEmitter(EmitterSystem& system, std::string name){
  auto remainingEmitters = filterEmitters(system, { name }).remainingEmitters;
  assert(remainingEmitters.size() == system.emitters.size() - 1);  // Better to just no op?)
  system.emitters = remainingEmitters;
}

bool emitterTimeExpired(Emitter& emitter, float currentTime){
  return (currentTime - emitter.initTime) > 1;                  
}

void updateEmitters(EmitterSystem& system, float currentTime){   
  std::vector<std::string> emitterToRemove;
  for (auto emitter : system.emitters){
    if (emitterTimeExpired(emitter, currentTime)){
      emitterToRemove.push_back(emitter.name);
    }
  }
  system.emitters = filterEmitters(system, emitterToRemove).remainingEmitters;
}