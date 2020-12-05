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
  bool hasDelta;
  std::string attributeName;
  AttributeValue value;
};

struct Emitter {
  std::string name;
  objid emitterNodeId;
  float initTime;
  float lastSpawnTime;
  unsigned int targetParticles;
  unsigned int currentParticles;
  std::deque<objid> particles;
  float spawnrate;
  float lifetime;
  GameobjAttributes particleAttributes;
  EmitterDelta delta;
};
struct EmitterSystem {
  std::vector<Emitter> emitters;
};

void addEmitter(EmitterSystem& system, std::string name, objid emitterNodeId, float currentTime, unsigned int targetParticles, float spawnrate, float lifetime, GameobjAttributes particleAttributes);
void removeEmitter(EmitterSystem& system, std::string name);
void updateEmitters(
  EmitterSystem& system, 
  float currentTime, 
  std::function<objid(std::string, GameobjAttributes, objid)> addParticle, 
  std::function<void(objid)> rmParticle,
  std::function<void(objid, std::string, AttributeValue)> updateParticle
);

#endif

