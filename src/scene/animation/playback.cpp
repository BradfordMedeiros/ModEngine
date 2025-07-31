#include "./playback.h"

std::vector<AnimationPose> playbackAnimation(
  AnimationWithIds& animation,  
  float currentTime,
  objid sceneId
){  
  auto posesForTick = animationPosesAtTime(currentTime, sceneId, animation);
  return posesForTick;
}

struct ChannelWithTwoPoses {
  objid targetId;
  int directIndex;
  std::optional<Transformation> pose1;
  std::optional<Transformation> pose2;
};

// This doesn't interpolate if the channels don't exist in the other pose.
std::vector<AnimationPose> combineAnimationPoses(std::vector<AnimationPose>& pose1, std::vector<AnimationPose>& pose2, float lerpAmount){
  std::vector<ChannelWithTwoPoses> channels;

  for (auto &channel : pose1){
    channels.push_back(ChannelWithTwoPoses{
      .targetId = channel.targetId,
      .directIndex = channel.directIndex,
      .pose1 = channel.pose,
      .pose2 = IDENTITY_TRANSFORMATION,
    });
  }


  for (auto &channel : pose2){
    std::optional<int> foundChannelIndex = std::nullopt;
    for (int i = 0; i < channels.size(); i++){
      if (channels.at(i).targetId == channel.targetId){
        foundChannelIndex = i;
        break;
      }
    }
    if (!foundChannelIndex.has_value()){
      channels.push_back(ChannelWithTwoPoses {
        .targetId = channel.targetId,
        .directIndex = channel.directIndex,
        .pose1 = IDENTITY_TRANSFORMATION,
        .pose2 = channel.pose,
      });
    }else{
      channels.at(foundChannelIndex.value()).pose2 = channel.pose;
    }
  }

  std::vector<AnimationPose> finalChannelPoses;
  for (auto &channel : channels){
    if (channel.pose1.has_value() && !channel.pose2.has_value()){
      finalChannelPoses.push_back(AnimationPose {
        .targetId = channel.targetId,
        .directIndex = channel.directIndex,
        .pose = channel.pose1.value(),
      });
    }else if (!channel.pose1.has_value() && channel.pose2.has_value()){
      finalChannelPoses.push_back(AnimationPose {
        .targetId = channel.targetId,
        .directIndex = channel.directIndex,
        .pose = channel.pose2.value(),
      });
    }else{
      Transformation transform1 = channel.pose1.value();
      Transformation transform2 = channel.pose2.value();
      auto interpolated = interpolate(transform2, transform1, lerpAmount, lerpAmount, lerpAmount);

      finalChannelPoses.push_back(AnimationPose {
        .targetId = channel.targetId,
        .directIndex = channel.directIndex,
        .pose = interpolated,
      });  
    }
  }

  return finalChannelPoses;
}

std::vector<AnimationPose> playbackAnimationBlend(
  AnimationWithIds& animation,
  AnimationWithIds& animation2,
  float currentTime,
  float currentTimeAnimation2,
  float blendFactor,
  objid sceneId
){ 
  std::cout << "blend: a factor is: " << currentTime << ", " << blendFactor << std::endl;
  auto posesForTick = animationPosesAtTime(currentTime, sceneId, animation);
  auto oldPosesForTick = animationPosesAtTime(currentTimeAnimation2, sceneId, animation2);
  auto combinedPoses = combineAnimationPoses(posesForTick, oldPosesForTick, blendFactor);
  return combinedPoses;
}

