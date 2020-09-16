#ifndef MOD_EMITTER
#define MOD_EMITTER

#include <vector>
#include "./common/mesh.h"
#include <queue>

struct Emitter {
  std::string name;
  float initTime;
  int targetParticles;
  int currentParticles;
  std::queue<objid> particles;
};
struct EmitterSystem {
  std::vector<Emitter> emitters;
};

void addEmitter(EmitterSystem& system, std::string name, float currentTime);
void removeEmitter(EmitterSystem& system, std::string name);
void updateEmitters(EmitterSystem& system, float currentTime, std::function<objid(std::string emitterName)> addParticle, std::function<void(objid)> rmParticle);

#endif

