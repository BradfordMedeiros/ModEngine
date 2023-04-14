#ifndef MOD_BENCHSTATS
#define MOD_BENCHSTATS

#include "./common/util.h"

// TODO -> this should be how all the values in benchmark come from
// then benchmark would just be the aggregator 
unsigned int statName(std::string name);
void registerStat(unsigned int stat, AttributeValue amount);
AttributeValue statValue(unsigned int);
AttributeValue statValue(std::string name);

#endif
