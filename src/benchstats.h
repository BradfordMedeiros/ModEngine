#ifndef MOD_BENCHSTATS
#define MOD_BENCHSTATS

#include <vector>
#include "./common/util.h"

unsigned int statName(std::string name);
void registerStat(unsigned int stat, AttributeValue amount);
AttributeValue statValue(unsigned int);
AttributeValue statValue(std::string name);

#endif
