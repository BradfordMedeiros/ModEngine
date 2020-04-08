#include "./animationdebug.h"

void printAnimationInfo(Animation& animation, float currentTime, float elapsedTime, float currentTick){
  std::cout << "-animation info-" << std::endl;
  std::cout << "playing animation: " << animation.name << std::endl;
  std::cout << "current time: " << currentTime << std::endl;
  std::cout << "elapsed time: " << elapsedTime << std::endl;

  double animationLengthSec = animation.duration / animation.ticksPerSecond;
  std::cout << "anim length: " << animationLengthSec << std::endl;
  std::cout << "num ticks: " << animation.duration << std::endl;
  std::cout << "ticks/s " << animation.ticksPerSecond << std::endl;
  std::cout << "num channels: " << animation.channels.size() << std::endl;
  std::cout << "current tick: " << currentTick << std::endl;
}

void printChannelInfo(AnimationChannel& channel, int tickPosIndex, int tickRotIndex, int tickScaleIndex){
  std::cout << "-channel info-" << std::endl; 
  std::cout << "node name: " << channel.nodeName << std::endl;
  std::cout << "num chan0 pos keys: " << channel.positionKeys.size() << std::endl;
  std::cout << "num chan0 rot keys: " << channel.rotationKeys.size() << std::endl;
  std::cout << "num chan0 scale keys: " << channel.scalingKeys.size() << std::endl; 

  std::cout << "tick pos key: " << tickPosIndex << std::endl;
  std::cout << "tick rot key: " << tickRotIndex << std::endl;
  std::cout << "tick scale key: " << tickScaleIndex << std::endl;
  std::cout << std::endl;
}
