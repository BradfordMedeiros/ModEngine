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
  std::string name;
  std::string templateName;
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

void addEmitter(EmitterSystem& system, std::string name, std::string templateName, objid emitterNodeId, float currentTime, unsigned int targetParticles, float spawnrate, float lifetime, GameobjAttributes particleAttributes, std::map<std::string, GameobjAttributes> submodelAttributes, std::vector<EmitterDelta> deltas, bool enabled, EmitterDeleteBehavior deleteBehavior);
void removeEmitter(EmitterSystem& system, std::string name);
void updateEmitters(
  EmitterSystem& system, 
  float currentTime, 
  std::function<std::optional<objid>(std::string, std::string, GameobjAttributes, std::map<std::string, GameobjAttributes>, objid, NewParticleOptions)> addParticle, 
  std::function<void(objid)> rmParticle,
  std::function<void(objid, std::string, AttributeValue)> updateParticle
);

void emitNewParticle(EmitterSystem& system, objid emitterNodeId, NewParticleOptions options);
void setEmitterEnabled(EmitterSystem& system, objid emitterNodeId, bool enabled);

#endif

