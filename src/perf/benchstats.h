#ifndef MOD_BENCHSTATS
#define MOD_BENCHSTATS

#include <string>
#include <vector>
#include <variant>
#include <assert.h>

typedef std::variant<int, float> StatValue;
// TODO -> this should be how all the values in benchmark come from
// then benchmark would just be the aggregator 
unsigned int statName(std::string name);
void registerStat(unsigned int stat, StatValue amount);
StatValue statValue(unsigned int);
StatValue statValue(std::string& name);

template<typename T>
T unwrapStat(StatValue value) {   
  T* unwrappedValue = std::get_if<T>(&value);
  if (unwrappedValue == NULL){
    assert(false);
  }
  return *unwrappedValue;
}

struct Stats {
  float initialTime;
  float now;
  float deltaTime; // Time between current frame and last frame
  float previous;
  float last60;
  float currentFps;

  unsigned int frameCount;
  long long totalFrames;
	int numTriangles;   // # drawn triangles (eg drawelements(x) -> missing certain calls like eg text)

  unsigned int numObjectsStat;
  unsigned int rigidBodiesStat;
  unsigned int scenesLoadedStat;
  unsigned int fpsStat;
};

void initializeStatistics();

#endif
