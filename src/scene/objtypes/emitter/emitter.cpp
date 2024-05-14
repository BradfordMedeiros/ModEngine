#include "./emitter.h"

void addEmitter(
  EmitterSystem& system, 
  objid emitterNodeId, 
  float currentTime, 
  unsigned int targetParticles,
  float spawnrate, 
  float lifetime,
  int numParticlesPerFrame,
  ParticleConfig particleConfig,
  bool enabled,
  EmitterDeleteBehavior deleteBehavior
){
  //std::cout << "INFO: emitter: adding emitter -  " << emitterNodeId << ", " << currentTime << std::endl;
  Emitter emitter {
    .emitterNodeId = emitterNodeId,
    .lastSpawnTime = currentTime,
    .numParticlesPerFrame = numParticlesPerFrame,
    .targetParticles = targetParticles,
    .currentParticles = 0,
    .spawnrate = spawnrate,
    .lifetime = lifetime,
    .particleConfig = particleConfig,
    .deleteBehavior = deleteBehavior,
    .enabled = enabled,
    .active = true,
    .forceParticles = {},
    .particleFrameIndex = 0,
  };
  system.emitters.push_back(emitter);
}

struct EmitterFilterResult {
  std::vector<Emitter> removedEmitters;
  std::vector<Emitter> remainingEmitters;
};

EmitterFilterResult filterEmitters(EmitterSystem& system, std::vector<objid> toRemove, EmitterDeleteBehavior preserveType){
  std::vector<Emitter> removedEmitters;
  std::vector<Emitter> remainingEmitters;
  for (auto emitter : system.emitters){
    bool shouldRemove = false;
    for (auto emitterToRemove : toRemove){
      if (emitter.emitterNodeId == emitterToRemove && emitter.deleteBehavior != preserveType){
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

void removeEmitter(EmitterSystem& system, objid id ){
  auto emitters = filterEmitters(system, { id }, EMITTER_FINISH);
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
    if (emitter.emitterNodeId == id && emitter.deleteBehavior == EMITTER_FINISH){
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


int getNumberParticleFrames(Emitter& emitter){
  return emitter.particleConfig.particleAttributes.size();
}
GameobjAttributes& getParticleAttr(Emitter& emitter, int frame){
  return emitter.particleConfig.particleAttributes.at(frame).attr;
}

std::map<std::string, GameobjAttributes>& getSubmodelAttr(Emitter& emitter, int frame){
  return emitter.particleConfig.submodelAttributes.at(frame).attr;
}

std::vector<EmitterDelta>& getDeltas(Emitter& emitter, int frame){
  return emitter.particleConfig.deltas.at(frame).deltas;
}

void updateEmitters(
  EmitterSystem& system, 
  float currentTime, 
  std::function<std::optional<objid>(GameobjAttributes attributes, std::map<std::string, GameobjAttributes> submodelAttributes, objid emitterNodeId, NewParticleOptions newParticleOpts)> addParticle, 
  std::function<void(objid)> rmParticle,
  std::function<void(objid, std::string, AttributeValue)> updateParticle
){   
  std::vector<objid> emittersToRemove;
  for (auto &emitter : system.emitters){
    if (!emitter.active && emitter.particles.size() == 0){
      emittersToRemove.push_back(emitter.emitterNodeId);
    }
  }
  if (emittersToRemove.size() > 0){
    system.emitters  = filterEmitters(system, emittersToRemove, EMITTER_NOTYPE).remainingEmitters;
  }

  for (auto &emitter : system.emitters){
    for (auto particle : emitter.particles){
      //std::cout << "INFO: PARTICLES: " << particleId << " , attribute: " << emitter.delta.attributeName << std::endl;
      for (auto delta : getDeltas(emitter, particle.frameIndex)){   // this is incorrect, should be 
        updateParticle(particle.id, delta.attributeName, delta.value);
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

      for (int i = 0; i < emitter.numParticlesPerFrame; i++){
        auto particleFrameIndex = emitter.particleFrameIndex;
        auto particleId = addParticle(getParticleAttr(emitter, particleFrameIndex), getSubmodelAttr(emitter, particleFrameIndex), emitter.emitterNodeId, newParticleOpts);
        emitter.particleFrameIndex++;
        if(emitter.particleFrameIndex >= getNumberParticleFrames(emitter)){
          emitter.particleFrameIndex = 0;
        }
        if (particleId.has_value()){
          emitter.particles.push_back(ActiveParticle {
            .id = particleId.value(),
            .frameIndex = particleFrameIndex,
            .spawntime = currentTime,
          });
        }
        emitter.currentParticles+= 1; 
        emitter.lastSpawnTime = currentTime;
        std::cout << "INFO: particles: adding particle" << std::endl;        
      }
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
  modassert(false, std::string("could not find emitter: ") + std::to_string(emitterNodeId));
}

Emitter nullEmitter {};
Emitter& getEmitter(EmitterSystem& system, objid emitterNodeId){
  for (auto &emitter : system.emitters){
    if (emitter.emitterNodeId == emitterNodeId){
      return emitter;
    }
  }
  modassert(false, "no emitter for this emitter id");
  return nullEmitter;
}

std::optional<EmitterDelta*> getEmitterDelta(EmitterSystem& system, objid emitterNodeId, std::string attributeName, int index){
  Emitter& emitter = getEmitter(system, emitterNodeId);
  for (auto &delta : getDeltas(emitter, index)){
    if (delta.attributeName == attributeName){
      return &delta;
    }
  }
  return std::nullopt;
}
void updateEmitterOptions(EmitterSystem& system, objid emitterNodeId, EmitterUpdateOptions&& updateOptions){
  int frameIndex = 0;
  //modassert(false, "not yet implemented");
  Emitter& emitter = getEmitter(system, emitterNodeId);
  if (updateOptions.targetParticles.has_value()){
    emitter.targetParticles = updateOptions.targetParticles.value();
  }
  if (updateOptions.spawnrate.has_value()){
    emitter.spawnrate = updateOptions.spawnrate.value();
  }
  if (updateOptions.lifetime.has_value()){
    emitter.lifetime = updateOptions.lifetime.value();
  }
  if (updateOptions.deleteBehavior.has_value()){
    emitter.deleteBehavior = updateOptions.deleteBehavior.value();
  }
  if (updateOptions.particleAttributes.has_value()){
    mergeAttributes(getParticleAttr(emitter, frameIndex), updateOptions.particleAttributes.value());
  }
  if (updateOptions.submodelAttributes.has_value()){
    modassert(false, "submodelAttributes not yet implemented");
  }
  if (updateOptions.delta.has_value()){
    auto existingDelta = getEmitterDelta(system, emitterNodeId, updateOptions.delta.value().attributeName, frameIndex);
    if (!existingDelta.has_value()){
      getDeltas(emitter, frameIndex).push_back(updateOptions.delta.value());
    }else{
      *(existingDelta.value()) = updateOptions.delta.value();
    }
  }
  if (updateOptions.enabled.has_value()){
    emitter.enabled = updateOptions.enabled.value();
  }
}


