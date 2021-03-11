#ifndef MOD_EMITTER
#define MOD_EMITTER

#include <vector>
#include <queue>
#include <iostream>
#include <functional>
#include <map>
#include "../../common/util.h"
#include "../common/util/types.h"

struct EmitterDelta {
  std::string attributeName;
  AttributeValue value;
  AttributeValue variance;
  std::vector<float> lifetimeEffect;
};

struct ActiveParticle {
  objid id;
  float spawntime;
};
struct Emitter {
  std::string name;
  objid emitterNodeId;
  float initTime;
  float lastSpawnTime;
  unsigned int targetParticles;
  unsigned int currentParticles;
  std::deque<ActiveParticle> particles;
  float spawnrate;
  float lifetime;
  GameobjAttributes particleAttributes;
  std::vector<EmitterDelta> deltas;

  bool enabled;
  int numForceNextRound;
};
struct EmitterSystem {
  std::vector<Emitter> emitters;
};

void addEmitter(EmitterSystem& system, std::string name, objid emitterNodeId, float currentTime, unsigned int targetParticles, float spawnrate, float lifetime, GameobjAttributes particleAttributes, std::vector<EmitterDelta> deltas, bool enabled);
void removeEmitter(EmitterSystem& system, std::string name);
void updateEmitters(
  EmitterSystem& system, 
  float currentTime, 
  std::function<objid(std::string, GameobjAttributes, objid)> addParticle, 
  std::function<void(objid)> rmParticle,
  std::function<void(objid, std::string, AttributeValue)> updateParticle
);
void emitNewParticle(EmitterSystem& system, objid emitterNodeId);
void setEmitterEnabled(EmitterSystem& system, objid emitterNodeId, bool enabled);

#endif

