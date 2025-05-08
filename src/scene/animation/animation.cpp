#include "./animation.h"

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

//// slightly off, check comparison func, maybe to do with the float values idk.
// this probably will need to be sped up.  Might make sense to put this into a ticktime -> position/scale/rot structure + seems to miss last key.  assumes array is in time order as well.
// ^ old note, dont remember, should verify
template<typename KeyType>   
KeyIndex findKeyIndex(std::vector<KeyType>& keys, float currentTick){
  int primaryTick = 0;
  int secondaryTick = 0;
  float primaryIndexAmount = 0.f;
  for (int i = 0; i < keys.size(); i++){                 
    auto key = keys[i];
    if (i == (keys.size() - 1)){
      if (key.mTime <= currentTick){
        primaryTick = i;
        secondaryTick = i;
        primaryIndexAmount = 1.f;
        break;
      }
    }
    int nextKeyIndex = i + 1;
    if (nextKeyIndex >= keys.size()){
      nextKeyIndex = keys.size() - 1;
    }
    modassert(nextKeyIndex < keys.size(), std::string("find key index error, next key index: ") + std::to_string(nextKeyIndex) + ", keys size = " + std::to_string(keys.size()));
    auto nextKey = keys[nextKeyIndex];
    if (key.mTime <= currentTick && nextKey.mTime > currentTick){    // mTime is in ticks
      primaryTick = i;
      secondaryTick = nextKeyIndex;
      auto howFarThroughTick = currentTick - key.mTime;
      auto totalLengthBetweenTicks = nextKey.mTime - key.mTime;
      primaryIndexAmount = howFarThroughTick / totalLengthBetweenTicks;
      break;
    }
  }

  return KeyIndex {
    .primaryIndex = primaryTick,
    .secondaryIndex = secondaryTick,
    .primaryIndexAmount = primaryIndexAmount,
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
std::vector<AnimationPose> animationPosesAtTime(float currentTime, objid sceneId, AnimationWithIds& animationWithIds){
  Animation& animation = animationWithIds.animation;
  assert(animation.ticksPerSecond != 0);                                                      // some models can have 0 ticks, probably should just set a default rate for these

  std::vector<AnimationPose> poses;
  auto currentTick = currentTime * animation.ticksPerSecond;                                  // 200 ticks / 100 ticks per second = 2 seconds
  //printAnimationInfo(animation, currentTime,currentTick);

  //modlog("animation", std::string("current time: ") + std::to_string(currentTime) + ", " + std::string("current tick: ") + std::to_string(currentTick));


  for (int i = 0; i < animation.channels.size(); i++){
    auto targetId = animationWithIds.channelObjIds.at(i);
    auto& channel = animation.channels.at(i);
    auto keyInfo = keyInfoForTick(channel, currentTick);
    Transformation newNodeTransformation = (
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
      .targetId = targetId,
      .pose = newNodeTransformation,
    });
  }
  return poses;
}
