#ifndef MOD_EMITTER
#define MOD_EMITTER

#include <vector>
#include "./common/mesh.h"
#include <queue>

struct Emitter {
  std::string name;
  float initTime;
  float lastSpawnTime;
  unsigned int targetParticles;
  unsigned int currentParticles;
  std::queue<objid> particles;
  float spawnrate;
  float lifetime;
  std::map<std::string, std::string> particleAttributes;
};
struct EmitterSystem {
  std::vector<Emitter> emitters;
};

void addEmitter(EmitterSystem& system, std::string name, float currentTime, unsigned int targetParticles, float spawnrate, float lifetime, std::map<std::string, std::string> particleAttributes);
void removeEmitter(EmitterSystem& system, std::string name);
void updateEmitters(EmitterSystem& system, float currentTime, std::function<objid(std::string, std::map<std::string, std::string>)> addParticle, std::function<void(objid)> rmParticle);

#endif

