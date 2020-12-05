#include "./emitter.h"

void addEmitter(
  EmitterSystem& system, 
  std::string name, 
  objid emitterNodeId, 
  float currentTime, 
  unsigned int targetParticles,
  float spawnrate, 
  float lifetime, 
  GameobjAttributes particleAttributes, 
  std::vector<EmitterDelta> deltas
){
  std::cout << "INFO: emitter: adding emitter -  " << name << ", " << currentTime << std::endl;

  Emitter emitter {
    .name = name,
    .emitterNodeId = emitterNodeId,
    .initTime = currentTime,
    .lastSpawnTime = currentTime,
    .targetParticles = targetParticles,
    .currentParticles = 0,
    .spawnrate = spawnrate,
    .lifetime = lifetime,
    .particleAttributes = particleAttributes,
    .deltas =  deltas,
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
  auto countUnderTarget = emitter.currentParticles < emitter.targetParticles;
  auto enoughTimePassed = (currentTime - emitter.lastSpawnTime) > emitter.spawnrate;
  // std::cout << "INFO: particles: count(" << (countUnderTarget ? "true" : "false") << " -- " << emitter.currentParticles << " / " << emitter.targetParticles << ")"  <<  " time(" << (enoughTimePassed ? "true" : "false") << ")" << std::endl;
  return countUnderTarget && enoughTimePassed;
}

void updateEmitters(
  EmitterSystem& system, 
  float currentTime, 
  std::function<objid(std::string emitterName, GameobjAttributes attributes, objid emitterNodeId)> addParticle, 
  std::function<void(objid)> rmParticle,
  std::function<void(objid, std::string, AttributeValue)> updateParticle
){   
  for (auto &emitter : system.emitters){
    for (auto particleId : emitter.particles){
      //std::cout << "INFO: PARTICLES: " << particleId << " , attribute: " << emitter.delta.attributeName << std::endl;
      for (auto delta : emitter.deltas){
        auto value = std::get_if<glm::vec3>(&delta.value);
        auto variance = std::get_if<glm::vec3>(&delta.variance);
        if (value != NULL && variance != NULL){
          auto randomFloatX = ((float)rand() * variance -> x) / RAND_MAX;
          auto randomFloatY = ((float)rand() * variance -> y) / RAND_MAX;
          auto randomFloatZ = ((float)rand() * variance -> z) / RAND_MAX;
          updateParticle(particleId, delta.attributeName, *value + glm::vec3(randomFloatX, randomFloatY, randomFloatZ));
        }else{
          updateParticle(particleId, delta.attributeName, delta.value);
        }
      }
    }
  }


  for (auto &emitter : system.emitters){
    if (emitter.currentParticles > 0 && emitterTimeExpired(emitter, currentTime)){
      emitter.currentParticles-= 1;
      emitter.initTime = currentTime;

      if (emitter.particles.size() > 0){
        auto particleId = emitter.particles.front();
        emitter.particles.pop_front();
        rmParticle(particleId);
        //std::cout << "INFO: particles: removing particle" << std::endl;
      }
    }
  }

  for (auto &emitter : system.emitters){
    if (shouldSpawnParticle(emitter, currentTime)){
      emitter.currentParticles+= 1; 
      auto particleId = addParticle(emitter.name, emitter.particleAttributes, emitter.emitterNodeId);
      emitter.particles.push_back(particleId);
      emitter.lastSpawnTime = emitter.lastSpawnTime + emitter.spawnrate;
      std::cout << "INFO: particles: adding particle" << std::endl;
    }
  }
}