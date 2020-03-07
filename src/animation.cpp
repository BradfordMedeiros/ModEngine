#include "./animation.h"

void printAnimationInfo(float currentTime, float elapsedTime, Animation& animation){
  std::cout << "playing animation: " << animation.name << std::endl;
  std::cout << "current time: " << currentTime << std::endl;
  std::cout << "elapsed time: " << elapsedTime << std::endl;

  double animationLengthSec = animation.duration / animation.ticksPerSecond;
  std::cout << "anim length: " << animationLengthSec << std::endl;
  std::cout << "num ticks: " << animation.duration << std::endl;
  std::cout << "ticks/s " << animation.ticksPerSecond << std::endl;
  std::cout << "num channels: " << animation.channels.size() << std::endl;

  std::cout << "node name: " << animation.channels.at(0).nodeName << std::endl;
  std::cout << "num chan0 pos keys: " << animation.channels.at(0).positionKeys.size() << std::endl;
  std::cout << "num chan0 rot keys: " << animation.channels.at(0).rotationKeys.size() << std::endl;
  std::cout << "num chan0 scale keys: " << animation.channels.at(0).scalingKeys.size() << std::endl; 
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


void advanceAnimation(Animation& animation, float currentTime, float elapsedTime){
  printAnimationInfo(currentTime, elapsedTime, animation);

  // 200 ticks / 100 ticks per second = 2 seconds
  assert(animation.ticksPerSecond != 0);  // some models can have 0 ticks, probably should just set a default rate for these

  auto currentTick = currentTime * animation.ticksPerSecond;   
  std::cout << "current tick: " << currentTick << std::endl;
  
  auto tickPosIndex = findIndexForKey(animation.channels.at(0).positionKeys, currentTick);
  std::cout << "tick pos key: " << tickPosIndex << std::endl;

  auto tickRotIndex = findIndexForKey(animation.channels.at(0).rotationKeys, currentTick);
  std::cout << "tick rot key: " << tickRotIndex << std::endl;

  auto tickScaleIndex = findIndexForKey(animation.channels.at(0).scalingKeys, currentTick);
  std::cout << "tick scale key: " << tickScaleIndex << std::endl;

  animateNode(
    animation.channels.at(0).nodeName,  
    animation.channels.at(0).positionKeys.at(tickPosIndex), 
    animation.channels.at(0).rotationKeys.at(tickRotIndex), 
    animation.channels.at(0).scalingKeys.at(tickScaleIndex)
  );
}
