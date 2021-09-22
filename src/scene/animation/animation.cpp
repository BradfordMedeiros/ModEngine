#include "./animation.h"

// slightly off, check comparison func, maybe to do with the float values idk.
// this probably will need to be sped up.  Might make sense to put this into a ticktime -> position/scale/rot structure + seems to miss last key.  assumes array is in time order as well.
template<typename KeyType>   
int findIndexForKey(std::vector<KeyType>& keys, float currentTick){   //todo frameup should be uses                  
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

struct KeyIndex {
  int primaryIndex;
  int secondaryIndex;
  float primaryIndexAmount;
};
struct KeyInfo {
  KeyIndex position;
  KeyIndex scale;
  KeyIndex rotation;
};

template<typename KeyType>   
KeyIndex findKeyIndex(std::vector<KeyType>& keys, float currentTick){
  auto tick = findIndexForKey(keys, currentTick);
  return KeyIndex {
    .primaryIndex = tick,
    .secondaryIndex = tick,
    .primaryIndexAmount = 0.f,
  };
}

KeyInfo keyInfoForTick(AnimationChannel& channel, float currentTick){
  return KeyInfo {
    .position = findKeyIndex(channel.positionKeys, currentTick),
    .scale = findKeyIndex(channel.scalingKeys, currentTick),
    .rotation = findKeyIndex(channel.rotationKeys, currentTick),
  };
}

Transformation primaryPoseFromKeyInfo(AnimationChannel& channel, KeyInfo& keyInfo){
  return aiKeysToTransform(
    channel.positionKeys.at(keyInfo.position.primaryIndex), 
    channel.rotationKeys.at(keyInfo.rotation.primaryIndex), 
    channel.scalingKeys.at(keyInfo.scale.primaryIndex)
  );
}
Transformation secondaryPoseFromKeyInfo(AnimationChannel& channel, KeyInfo& keyInfo){
  return aiKeysToTransform(
    channel.positionKeys.at(keyInfo.position.secondaryIndex), 
    channel.rotationKeys.at(keyInfo.rotation.secondaryIndex), 
    channel.scalingKeys.at(keyInfo.scale.secondaryIndex)
  );
}

bool shouldInterpolate = true;
std::vector<AnimationPose> animationPosesAtTime(Animation& animation, float currentTime, float elapsedTime){
  assert(animation.ticksPerSecond != 0);                                                      // some models can have 0 ticks, probably should just set a default rate for these

  std::vector<AnimationPose> poses;
  auto currentTick = currentTime * animation.ticksPerSecond;                                  // 200 ticks / 100 ticks per second = 2 seconds
  //printAnimationInfo(animation, currentTime, elapsedTime, currentTick);

  for (auto channel : animation.channels){
    auto keyInfo = keyInfoForTick(channel, currentTick);
    glm::mat4 newNodePose = transformToGlm(
      shouldInterpolate ? 
      interpolate(
        primaryPoseFromKeyInfo(channel, keyInfo), 
        secondaryPoseFromKeyInfo(channel, keyInfo),
        keyInfo.position.primaryIndexAmount,
        keyInfo.scale.primaryIndexAmount,
        keyInfo.rotation.primaryIndexAmount
      ):
      primaryPoseFromKeyInfo(channel, keyInfo)
    );
    poses.push_back(AnimationPose{
      .channelName = channel.nodeName,
      .pose = newNodePose,
    });
  }
  return poses;
}
