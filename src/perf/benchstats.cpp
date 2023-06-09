#include "./benchstats.h"

struct StatInfo {
  std::string name;
  AttributeValue amount;
};

std::vector<StatInfo> stats = {};

unsigned int statName(std::string name){
	for (int i = 0; i < stats.size(); i++){
		auto stat = stats.at(i);
		if (stat.name == name){
			return i;
		}
	}
	stats.push_back(StatInfo{
		.name = name,
		.amount = 0.f, 
	});
	return stats.size() - 1;
}
void registerStat(unsigned int stat, AttributeValue amount){
  stats.at(stat).amount = amount;
}
AttributeValue statValue(unsigned int stat){
  return stats.at(stat).amount;
}
AttributeValue statValue(std::string& name){
	for (int i = 0; i < stats.size(); i++){
		auto stat = stats.at(i);
		if (stat.name == name){
			return stat.amount;
		}
	}
	modassert(false, "invalid stat value: " + name);
	return 0.f;
}
