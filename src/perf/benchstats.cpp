#include "./benchstats.h"

Stats statistics {
  .initialTime = 0.f,
  .now = 0.f,
  .deltaTime = 0.f, // Time between current frame and last frame
  .previous = 0.f,
  .last60 = 0.f,
  .currentFps = 0.f,
  .frameCount = 0,
  .totalFrames = 0,
  .numTriangles = 0,
  .numObjectsStat = 0,
  .rigidBodiesStat = 0,
  .scenesLoadedStat = 0,
  .fpsStat = 0,
  .loadModelFileStat = 0,
  .loadModelFileStatCacheHit = 0,
};


struct StatInfo {
  std::string name;
  StatValue amount;
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
void registerStat(unsigned int stat, StatValue amount){
	if (stat == statistics.loadModelFileStat){
		std::cout << "load model called" << std::endl;
	}else if (stat == statistics.loadModelFileStatCacheHit){
		std::cout << "load model called: cache hit" << std::endl;
	}
  stats.at(stat).amount = amount;
}
StatValue statValue(unsigned int stat){
  return stats.at(stat).amount;
}
StatValue statValue(std::string& name){
	for (int i = 0; i < stats.size(); i++){
		auto stat = stats.at(i);
		if (stat.name == name){
			return stat.amount;
		}
	}
	assert(false);
	return 0.f;
}

void initializeStatistics(){
  statistics.fpsStat = statName("fps");
  statistics.numObjectsStat = statName("object-count");
  statistics.rigidBodiesStat = statName("rigidbody-count");
  statistics.scenesLoadedStat = statName("scenes-loaded");
  statistics.loadModelFileStat = statName("load-model-file-stat");
  statistics.loadModelFileStatCacheHit = statName("load-model-file-stat-cache-hit");
}
