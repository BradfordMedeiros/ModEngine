#ifndef MOD_BENCHSTATS
#define MOD_BENCHSTATS

#include "../common/util.h"

// TODO -> this should be how all the values in benchmark come from
// then benchmark would just be the aggregator 
unsigned int statName(std::string name);
void registerStat(unsigned int stat, AttributeValue amount);
AttributeValue statValue(unsigned int);
AttributeValue statValue(std::string& name);

struct Stats {
  unsigned int frameCount;
  long long totalFrames;
	int numTriangles;   // # drawn triangles (eg drawelements(x) -> missing certain calls like eg text)

  unsigned int numObjectsStat;
  unsigned int rigidBodiesStat;
  unsigned int scenesLoadedStat;
  unsigned int fpsStat;
};

#endif
