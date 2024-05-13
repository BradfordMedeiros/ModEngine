#ifndef MOD_EMITTER
#define MOD_EMITTER

#include <vector>
#include <queue>
#include <iostream>
#include <functional>
#include <optional>
#include <map>
#include "../../../common/util.h"
#include "../../common/util/types.h"

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

struct NewParticleOptions {
  std::optional<glm::vec3> position;
  std::optional<glm::quat> orientation; 
  std::optional<glm::vec3> velocity;
  std::optional<glm::vec3> angularVelocity;
  std::optional<objid> parentId;
};

enum EmitterDeleteBehavior { EMITTER_NOTYPE, EMITTER_DELETE, EMITTER_ORPHAN, EMITTER_FINISH };
struct Emitter {
  objid emitterNodeId;
  float lastSpawnTime;
  unsigned int targetParticles;
  unsigned int currentParticles;
  std::deque<ActiveParticle> particles;
  float spawnrate;
  float lifetime;
  GameobjAttributes particleAttributes;
  std::map<std::string, GameobjAttributes> submodelAttributes;
  std::vector<EmitterDelta> deltas;
  EmitterDeleteBehavior deleteBehavior;

  bool enabled;
  bool active;
  std::deque<NewParticleOptions> forceParticles;
};
struct EmitterSystem {
  std::vector<Emitter> emitters;
  std::vector<objid> additionalParticlesToRemove;
};

void addEmitter(EmitterSystem& system, objid emitterNodeId, float currentTime, unsigned int targetParticles, float spawnrate, float lifetime, GameobjAttributes particleAttributes, std::map<std::string, GameobjAttributes> submodelAttributes, std::vector<EmitterDelta> deltas, bool enabled, EmitterDeleteBehavior deleteBehavior);
void removeEmitter(EmitterSystem& system, objid id);
void updateEmitters(
  EmitterSystem& system, 
  float currentTime, 
  std::function<std::optional<objid>(GameobjAttributes, std::map<std::string, GameobjAttributes>, objid, NewParticleOptions)> addParticle, 
  std::function<void(objid)> rmParticle,
  std::function<void(objid, std::string, AttributeValue)> updateParticle
);

void emitNewParticle(EmitterSystem& system, objid emitterNodeId, NewParticleOptions options);

std::optional<EmitterDelta*> getEmitterDelta(EmitterSystem& system, objid emitterNodeId, std::string attributeName);

struct EmitterUpdateOptions{
  std::optional<unsigned int> targetParticles;
  std::optional<float> spawnrate;
  std::optional<float> lifetime;
  std::optional<EmitterDeleteBehavior> deleteBehavior;
  std::optional<GameobjAttributes> particleAttributes;
  std::optional<std::map<std::string, GameobjAttributes>> submodelAttributes;
  std::optional<EmitterDelta> delta;
  std::optional<bool> enabled;
};
void updateEmitterOptions(EmitterSystem& system, objid emitterNodeId, EmitterUpdateOptions&& update);

#endif

