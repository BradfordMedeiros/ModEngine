#include "./emitter.h"

void addEmitter(
  EmitterSystem& system, 
  std::string name,
  std::string templateName,
  objid emitterNodeId, 
  float currentTime, 
  unsigned int targetParticles,
  float spawnrate, 
  float lifetime, 
  GameobjAttributes particleAttributes, 
  std::vector<EmitterDelta> deltas,
  bool enabled,
  EmitterDeleteBehavior deleteBehavior
){
  std::cout << "INFO: emitter: adding emitter -  " << name << ", " << currentTime << std::endl;
  std::cout << "emitter particle attrs: " << print(particleAttributes) << std::endl;

  Emitter emitter {
    .name = name,
    .templateName = templateName,
    .emitterNodeId = emitterNodeId,
    .lastSpawnTime = currentTime,
    .targetParticles = targetParticles,
    .currentParticles = 0,
    .spawnrate = spawnrate,
    .lifetime = lifetime,
    .particleAttributes = particleAttributes,
    .submodelAttributes = {
    },
    .deltas =  deltas,
    .deleteBehavior = deleteBehavior,
    .enabled = enabled,
    .active = true,
    .forceParticles = {},
  };
  system.emitters.push_back(emitter);
}

struct EmitterFilterResult {
  std::vector<Emitter> removedEmitters;
  std::vector<Emitter> remainingEmitters;
};

EmitterFilterResult filterEmitters(EmitterSystem& system, std::vector<std::string> toRemove, EmitterDeleteBehavior preserveType){
  std::vector<Emitter> removedEmitters;
  std::vector<Emitter> remainingEmitters;
  for (auto emitter : system.emitters){
    bool shouldRemove = false;
    for (auto emitterToRemove : toRemove){
      if (emitter.name == emitterToRemove && emitter.deleteBehavior != preserveType){
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
  auto emitters = filterEmitters(system, { name }, EMITTER_FINISH);
  auto remainingEmitters = emitters.remainingEmitters;
  auto removedEmitters = emitters.removedEmitters;
  for (auto emitter : removedEmitters){
    if (emitter.deleteBehavior == EMITTER_DELETE){
      for (auto particle : emitter.particles){
        system.additionalParticlesToRemove.push_back(particle.id);
      }
    }else if (emitter.deleteBehavior == EMITTER_ORPHAN){
      // do nothing, orphan this.  particles removed when scene gets unloaded
    }
  }

  for (auto &emitter : remainingEmitters){
    if (emitter.name == name && emitter.deleteBehavior == EMITTER_FINISH){
      emitter.active = false;
    }
  }

  system.emitters = remainingEmitters;
}

bool emitterTimeExpired(Emitter& emitter, float currentTime){
  return (currentTime - emitter.particles.front().spawntime) > emitter.lifetime;
}
bool shouldSpawnParticle(Emitter& emitter, float currentTime){
  auto countUnderTarget = emitter.currentParticles < emitter.targetParticles;
  auto enoughTimePassed = (currentTime - emitter.lastSpawnTime) > emitter.spawnrate;
  // std::cout << "INFO: particles: count(" << (countUnderTarget ? "true" : "false") << " -- " << emitter.currentParticles << " / " << emitter.targetParticles << ")"  <<  " time(" << (enoughTimePassed ? "true" : "false") << ")" << std::endl;
  return countUnderTarget && enoughTimePassed;
}

float calcLifetimeEffect(float timeElapsed, float totalDuration, std::vector<float>& lifetimeEffect){
  if (lifetimeEffect.size() == 0){
    return 1.f;
  }
  auto index = 1 + (int)((timeElapsed / totalDuration) * lifetimeEffect.size());  // round up, down, or interpolate?  or different for first run?
  index = index >= lifetimeEffect.size() ? (lifetimeEffect.size() - 1) : index;   // Do I actually need this?  Fix despawing and this probably goes away
  return lifetimeEffect.at(timeElapsed == 0 ? 0 : index);
}

void updateEmitters(
  EmitterSystem& system, 
  float currentTime, 
  std::function<objid(std::string emitterName, std::string templateName, GameobjAttributes attributes, std::map<std::string, GameobjAttributes> submodelAttributes, objid emitterNodeId, NewParticleOptions newParticleOpts)> addParticle, 
  std::function<void(objid)> rmParticle,
  std::function<void(objid, std::string, AttributeValue)> updateParticle
){   
  std::vector<std::string> emittersToRemove;
  for (auto &emitter : system.emitters){
    if (!emitter.active && emitter.particles.size() == 0){
      emittersToRemove.push_back(emitter.name);
    }
  }
  if (emittersToRemove.size() > 0){
    system.emitters  = filterEmitters(system, emittersToRemove, EMITTER_NOTYPE).remainingEmitters;
  }

  for (auto &emitter : system.emitters){
    for (auto particle : emitter.particles){
      //std::cout << "INFO: PARTICLES: " << particleId << " , attribute: " << emitter.delta.attributeName << std::endl;
      for (auto delta : emitter.deltas){
        auto value = std::get_if<glm::vec3>(&delta.value);
        auto variance = std::get_if<glm::vec3>(&delta.variance);
        auto valueVec4 = std::get_if<glm::vec4>(&delta.value);
        auto varianceVec4 = std::get_if<glm::vec4>(&delta.variance);
        auto lifetimeEffect = calcLifetimeEffect(currentTime - particle.spawntime, emitter.lifetime, delta.lifetimeEffect);
        if (value != NULL && variance != NULL){
          auto randomFloatX = (((float)rand() * variance -> x) / RAND_MAX) * 2 - variance -> x;
          auto randomFloatY = (((float)rand() * variance -> y) / RAND_MAX) * 2 - variance -> y;
          auto randomFloatZ = (((float)rand() * variance -> z) / RAND_MAX) * 2 - variance -> z;
          updateParticle(particle.id, delta.attributeName, *value + (lifetimeEffect * glm::vec3(randomFloatX, randomFloatY, randomFloatZ)));
        }else if (valueVec4 != NULL && varianceVec4 != NULL){
          //  dup code for vec4 type from above, should consolidate
          auto randomFloatX = (((float)rand() * varianceVec4 -> x) / RAND_MAX) * 2 - varianceVec4 -> x;
          auto randomFloatY = (((float)rand() * varianceVec4 -> y) / RAND_MAX) * 2 - varianceVec4 -> y;
          auto randomFloatZ = (((float)rand() * varianceVec4 -> z) / RAND_MAX) * 2 - varianceVec4 -> z;
          auto randomFloatW = (((float)rand() * varianceVec4 -> w) / RAND_MAX) * 2 - varianceVec4 -> w;
          updateParticle(particle.id, delta.attributeName, *valueVec4 + (lifetimeEffect * glm::vec4(randomFloatX, randomFloatY, randomFloatZ, randomFloatW)));
        }else{
          updateParticle(particle.id, delta.attributeName, delta.value);
        }
      }
    }
  }

  for (auto &emitter : system.emitters){
    if (emitter.currentParticles > 0 && (emitterTimeExpired(emitter, currentTime) || (emitter.currentParticles > emitter.targetParticles))){
      emitter.currentParticles-= 1;

      if (emitter.particles.size() > 0){
        auto particleId = emitter.particles.front().id;
        emitter.particles.pop_front();
        rmParticle(particleId);
        //std::cout << "INFO: particles: removing particle" << std::endl;
      }
    }
  }

  for (auto particleId : system.additionalParticlesToRemove){
    rmParticle(particleId);
  }
  system.additionalParticlesToRemove = {};

  for (auto &emitter : system.emitters){
    if (!emitter.active || (!emitter.enabled && emitter.forceParticles.size() == 0)){
      continue;
    }
    bool forceSpawn = emitter.forceParticles.size() > 0;
    if (shouldSpawnParticle(emitter, currentTime) || forceSpawn){
      NewParticleOptions newParticleOpts {
        .position = std::nullopt,
        .orientation = std::nullopt,
        .velocity = std::nullopt,
        .angularVelocity = std::nullopt,
      };
      if (forceSpawn){
        newParticleOpts = emitter.forceParticles.front();
        emitter.forceParticles.pop_front();
      }
      emitter.currentParticles+= 1; 

      auto particleId = addParticle(emitter.name, emitter.templateName, emitter.particleAttributes, emitter.submodelAttributes, emitter.emitterNodeId, newParticleOpts);

      emitter.particles.push_back(ActiveParticle {
        .id = particleId,
        .spawntime = currentTime,
      });
      emitter.lastSpawnTime = currentTime;
      std::cout << "INFO: particles: adding particle" << std::endl;
    }
  }
}

void emitNewParticle(EmitterSystem& system, objid emitterNodeId, NewParticleOptions options){
  std::cout << "Emit new particle placehodler: " << emitterNodeId << std::endl;
  for (auto &emitter : system.emitters){
    if (emitter.emitterNodeId == emitterNodeId){
      emitter.forceParticles.push_back(options);
      return;
    }
  }
  assert(false);
}

void setEmitterEnabled(EmitterSystem& system, objid emitterNodeId, bool enabled){
  for (auto &emitter : system.emitters){
    if (emitter.emitterNodeId == emitterNodeId){
      emitter.enabled = enabled;
      return;
    }
  }
  assert(false);
}