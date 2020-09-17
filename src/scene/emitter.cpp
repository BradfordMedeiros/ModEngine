#include "./emitter.h"

void addEmitter(EmitterSystem& system, std::string name, float currentTime, unsigned int targetParticles, float spawnrate, float lifetime){
  std::cout << "INFO: emitter: adding emitter -  " << name << std::endl;
  Emitter emitter {
    .name = name,
    .initTime = currentTime,
    .lastSpawnTime = currentTime,
    .targetParticles = targetParticles,
    .currentParticles = 0,
    .spawnrate = spawnrate,
    .lifetime = lifetime,
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
  return (currentTime - emitter.lastSpawnTime) > emitter.lifetime;                  
}
bool shouldSpawnParticle(Emitter& emitter, float currentTime){
  return (emitter.currentParticles < emitter.targetParticles) && ((currentTime - emitter.lastSpawnTime) > emitter.spawnrate) ;
}

void updateEmitters(EmitterSystem& system, float currentTime, std::function<objid(std::string emitterName)> addParticle, std::function<void(objid)> rmParticle){   
  std::vector<std::string> emitterToRemove;
  for (auto &emitter : system.emitters){
    if (emitterTimeExpired(emitter, currentTime)){
      emitter.currentParticles-= 1;
      emitter.initTime = currentTime;

      if (emitter.particles.size() > 0){
        auto particleId = emitter.particles.front();
        emitter.particles.pop();
        rmParticle(particleId);
        std::cout << "removing particle" << std::endl;
      }

    }
  }

  for (auto &emitter : system.emitters){
    if (shouldSpawnParticle(emitter, currentTime)){
      emitter.currentParticles+= 1; 
      auto particleId = addParticle(emitter.name);
      emitter.particles.push(particleId);
      emitter.lastSpawnTime = emitter.lastSpawnTime + emitter.spawnrate;
      std::cout << "adding particle" << std::endl;
    }
  }
}