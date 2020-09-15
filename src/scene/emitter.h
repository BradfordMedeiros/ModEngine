#ifndef MOD_EMITTER
#define MOD_EMITTER

#include <vector>
#include "./common/mesh.h"

struct Emitter {
  std::string name;
  float initTime;
  int targetParticles;
  int currentParticles;
  std::function<void()> addParticle;
  std::function<void()> rmParticle;
};
struct EmitterSystem {
  std::vector<Emitter> emitters;
};

void addEmitter(EmitterSystem& system, std::string name, float currentTime, std::function<void()> addParticle, std::function<void()> rmParticle);
void removeEmitter(EmitterSystem& system, std::string name);
void updateEmitters(EmitterSystem& system, float currentTime);

#endif

