#ifndef MOD_EMITTER
#define MOD_EMITTER

#include <vector>
#include "./common/mesh.h"

struct Emitter {
  std::string name;
  float initTime;
};
struct EmitterSystem {
  std::vector<Emitter> emitters;
};

void addEmitter(EmitterSystem& system, std::string name, float currentTime);
void removeEmitter(EmitterSystem& system, std::string name);
void updateEmitters(EmitterSystem& system);

#endif

