#include "./animation.h"

// slightly off, check comparison func, maybe to do with the float values idk.
// this probably will need to be sped up.  Might make sense to put this into a ticktime -> position/scale/rot structure + seems to miss last key.  assumes array is in time order as well.
template<typename KeyType>   
int findIndexForKey(std::vector<KeyType>& keys, float currentTick){                    
  int tick = 0;
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
  return tick;
}

glm::mat4 advanceAnimationForNode(AnimationChannel& channel, float currentTick){
  auto tickPosIndex = findIndexForKey(channel.positionKeys, currentTick);
  auto tickRotIndex = findIndexForKey(channel.rotationKeys, currentTick);
  auto tickScaleIndex = findIndexForKey(channel.scalingKeys, currentTick);

  //printChannelInfo(channel, tickPosIndex, tickRotIndex, tickScaleIndex);

  auto newPose = aiKeysToGlm(                     // @TODO - animation - add interpolation (linear, but being able to change this to other interpolation methods could be interesting...)
    channel.positionKeys.at(tickPosIndex), 
    channel.rotationKeys.at(tickRotIndex), 
    channel.scalingKeys.at(tickScaleIndex)
  );
  return newPose;
}

void advanceAnimation(Animation& animation, float currentTime, float elapsedTime, std::function<void(std::string, glm::mat4)> setBonePose){
  assert(animation.ticksPerSecond != 0);                                                      // some models can have 0 ticks, probably should just set a default rate for these

  auto currentTick = currentTime * animation.ticksPerSecond;                                  // 200 ticks / 100 ticks per second = 2 seconds
  //printAnimationInfo(animation, currentTime, elapsedTime, currentTick);

  for (auto channel : animation.channels){
    glm::mat4 newNodePose = advanceAnimationForNode(channel, currentTick);
    setBonePose(channel.nodeName, newNodePose);  
  }
}
