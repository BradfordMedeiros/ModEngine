#include "./playback.h"

// https://gamedev.net/forums/topic/484984-skeletal-animation-non-uniform-scale/4172731/
void updateBonePoses(NameAndMeshObjName meshNameToMeshes, std::function<glm::mat4(std::string, std::string)> getModelMatrix, std::string rootname){
  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    std::string meshname = meshNameToMeshes.objnames.at(i);
    for (Bone& bone : mesh.bones){
      bone.offsetMatrix = getModelMatrix(bone.name, rootname) * glm::inverse(bone.initialBonePose);
    }
  }
}

void playbackAnimation(
  Animation animation,  
  float currentTime, 
  NameAndMeshObjName meshNameToMeshes,  
  std::function<glm::mat4(std::string, std::string)> getModelMatrix,
  std::function<void(std::string, glm::mat4)> setPose,
  std::string rootname
){  
  auto posesForTick = animationPosesAtTime(animation, currentTime);
  for (auto pose : posesForTick){
    //printMatrixInformation(pose.pose, std::string("SET_CHANNEL:") + pose.channelName);
    setPose(pose.channelName, transformToGlm(pose.pose));
  }
  updateBonePoses(meshNameToMeshes, getModelMatrix, rootname);
}

struct ChannelWithTwoPoses {
  std::string channel;
  std::optional<Transformation> pose1;
  std::optional<Transformation> pose2;
};

// This doesn't interpolate if the channels don't exist in the other pose.
std::vector<AnimationPose> combineAnimationPoses(std::vector<AnimationPose>& pose1, std::vector<AnimationPose>& pose2, float lerpAmount){
  std::vector<ChannelWithTwoPoses> channels;

  for (auto &channel : pose1){
    channels.push_back(ChannelWithTwoPoses{
      .channel = channel.channelName,
      .pose1 = channel.pose,
      .pose2 = IDENTITY_TRANSFORMATION,
    });
  }


  for (auto &channel : pose2){
    std::optional<int> foundChannelIndex = std::nullopt;
    for (int i = 0; i < channels.size(); i++){
      if (channels.at(i).channel == channel.channelName){
        foundChannelIndex = i;
        break;
      }
    }
    if (!foundChannelIndex.has_value()){
      channels.push_back(ChannelWithTwoPoses {
        .channel = channel.channelName,
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
        .channelName = channel.channel,
        .pose = channel.pose1.value(),
      });
    }else if (!channel.pose1.has_value() && channel.pose2.has_value()){
      finalChannelPoses.push_back(AnimationPose {
        .channelName = channel.channel,
        .pose = channel.pose2.value(),
      });
    }else{
      Transformation transform1 = channel.pose1.value();
      Transformation transform2 = channel.pose2.value();
      auto interpolated = interpolate(transform2, transform1, lerpAmount, lerpAmount, lerpAmount);

      finalChannelPoses.push_back(AnimationPose {
        .channelName = channel.channel,
        .pose = interpolated,
      });  
    }
  }

  return finalChannelPoses;
}

void playbackAnimationBlend(
  Animation animation,
  Animation animation2,
  float currentTime,
  float currentTimeAnimation2,
  float blendFactor,
  NameAndMeshObjName meshNameToMeshes,  
  std::function<glm::mat4(std::string, std::string)> getModelMatrix,
  std::function<void(std::string, glm::mat4)> setPose,
  std::string rootname
){ 
  std::cout << "blend: a factor is: " << currentTime << ", " << blendFactor << std::endl;
  auto posesForTick = animationPosesAtTime(animation, currentTime);
  auto oldPosesForTick = animationPosesAtTime(animation2, currentTimeAnimation2);
  auto combinedPoses = combineAnimationPoses(posesForTick, oldPosesForTick, blendFactor);

  for (auto pose : combinedPoses){
    //printMatrixInformation(pose.pose, std::string("SET_CHANNEL:") + pose.channelName);
    setPose(pose.channelName, transformToGlm(pose.pose));
  }
  updateBonePoses(meshNameToMeshes, getModelMatrix, rootname);
}