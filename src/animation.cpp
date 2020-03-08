#include "./animation.h"

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

// slightly off, check comparison func, maybe to do with the float values idk.
// this probably will need to be sped up.  Might make sense to put this into a ticktime -> position/scale/rot structure + seems to miss last key.  assumes array is in time order as well.
template<typename KeyType>   
int findIndexForKey(std::vector<KeyType>& keys, float currentTick){                    
  int tick = -1;
  for (int i = 0; i < keys.size(); i++){                 
    auto key = keys[i];
    if (i == (keys.size() - 1)){
      if (key.mTime <= currentTick){
        return i;
      }
    }

    auto nextKey = keys[i+1];
    if (key.mTime <= currentTick && nextKey.mTime > currentTick){    // mTime is in ticks
      return i;
    }
  }

  std::cout << "mTime: " << keys[0].mTime << std::endl;
  std::cout << "current tick: " << currentTick << std::endl;
  return tick;
}

void animateNode(std::string nodeName, aiVectorKey& positionKey, aiQuatKey& rotationKey, aiVectorKey& scalingKey){

}

void advanceAnimationForNode(Animation& animation, AnimationChannel& channel, float currentTick, float currentTime, float elapsedTime){
  auto tickPosIndex = findIndexForKey(channel.positionKeys, currentTick);
  auto tickRotIndex = findIndexForKey(channel.rotationKeys, currentTick);
  auto tickScaleIndex = findIndexForKey(channel.scalingKeys, currentTick);

  printChannelInfo(channel, tickPosIndex, tickRotIndex, tickScaleIndex);

  animateNode(
    channel.nodeName,  
    channel.positionKeys.at(tickPosIndex), 
    channel.rotationKeys.at(tickRotIndex), 
    channel.scalingKeys.at(tickScaleIndex)
  );
}

void advanceAnimation(Animation& animation, float currentTime, float elapsedTime){
  assert(animation.ticksPerSecond != 0);                                                      // some models can have 0 ticks, probably should just set a default rate for these

  auto currentTick = currentTime * animation.ticksPerSecond;                                  // 200 ticks / 100 ticks per second = 2 seconds
  printAnimationInfo(animation, currentTime, elapsedTime, currentTick);
  advanceAnimationForNode(animation, animation.channels.at(0), currentTick, currentTime, elapsedTime);
}
