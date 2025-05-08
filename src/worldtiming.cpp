#include "./worldtiming.h"

WorldTiming createWorldTiming(float initialTime){
  AnimationState animations;
  std::vector<int32_t> playbacksToRemove;

  WorldTiming timing {
    .animations = animations,
    .disableAnimationIds = {},
    .playbacksToRemove = playbacksToRemove,
    .initialTime = initialTime,
  };
  return timing;
}



bool enableBlending = true;
float blendingWindow = 0.25f;  // this should be able to be specified by the animation most likely

void setPoses(World& world, std::set<objid>& disableIds, objid idScene, std::vector<AnimationPose>& poses){
  for (auto& pose : poses){
    if (disableIds.count(pose.targetId) > 0){
      continue;
    }
    physicsLocalTransformSet(world, pose.targetId, pose.pose);
    //printMatrixInformation(pose.pose, std::string("SET_CHANNEL:") + pose.channelName);
  }
}
void tickAnimation(World& world, std::set<objid>& disableAnimationIds, AnimationData& playback, float currentTime){
  if (enableBlending && playback.blendData.has_value()){
    float timeElapsedBlendStart = currentTime - playback.blendData.value().blendStartTime;
    float aFactor = glm::min(1.f, timeElapsedBlendStart / blendingWindow);
    // if afactor > 1.f or something like that, could get rid of the old animation value
    //modassert(false, "blend not yet supported");

    auto newPoses = playbackAnimationBlend(
      playback.animation,
      playback.blendData.value().animation, 
      currentTime - playback.initTime,
      currentTime - playback.blendData.value().oldAnimationInit, 
      aFactor,
      playback.idScene
    );
    setPoses(world, disableAnimationIds, playback.idScene, newPoses);
  }else{
    auto newPoses = playbackAnimation(playback.animation, currentTime - playback.initTime, playback.idScene);
    setPoses(world,  disableAnimationIds, playback.idScene, newPoses);
  }
}

void tickAnimations(World& world, WorldTiming& timings, float currentTime){
  //std::cout << "animations num active playbacks: " << timings.animations.playbacks.size() << std::endl;
  for (auto &[id, playback] : timings.animations.playbacks){
    // might be better to not have this check here and instead just assume obj exists,
    // remove animation when del object, but for now!
    if (!idExists(world.sandbox, playback.groupId)){  // why are we doing this check here?
      timings.playbacksToRemove.push_back(playback.groupId);
      modlog("animation", std::string("removed playbacks because of internal group id: ") + std::to_string(playback.groupId));
    }else{
      tickAnimation(world, timings.disableAnimationIds, playback, currentTime);
    }
    //modlog("animation", "ticking animation for groupid: " + std::to_string(playback.groupId));
  }


  for (auto &[id, playback] : timings.animations.playbacks){
    float timeElapsed = currentTime - playback.initTime;
    if (timeElapsed > playback.animLength){
      modlog("animation", "on animation finish");
      if (playback.animationType == ONESHOT){
        timings.playbacksToRemove.push_back(playback.groupId);
        continue;
      }else if (playback.animationType == LOOP){
        playback.initTime = currentTime;
        continue;
      }else if (playback.animationType == FORWARDS){
        // do nothing
        continue;
      }
      modassert(false, "invalid animationType");
    }
  }
  //std::cout << "num playbacks: " << timings.animations.playbacks.size() << std::endl;
  for (auto groupId : timings.playbacksToRemove){
    timings.animations.playbacks.erase(groupId);
    //modlog("animation", std::string("playbacks to remove, removing: ") + std::to_string(groupId));
  }
  timings.playbacksToRemove.clear();
}

std::optional<Animation> getAnimation(World& world, int32_t groupId, std::string animationToPlay){  
  if (world.animations.find(groupId) == world.animations.end()){
    return std::nullopt;
  }
  for (auto animation :  world.animations.at(groupId)){
    if (animation.name == animationToPlay){
      return animation;
    }
  }
  std::cout << "ERROR: no animation found named: " << animationToPlay << std::endl;
  std::cout << "ERROR INFO: existing animation names [" << world.animations.at(groupId).size() << "] - ";
  for (auto animation : world.animations.at(groupId)){
    std::cout << animation.name << " ";
  }
  std::cout << std::endl;
  return  std::nullopt;  // @todo probably use optional here.
}

void invalidatePlaybackToRemove(WorldTiming& timing, objid groupId){
  std::vector<int32_t> playbacksToRemoveNew;
  for (auto id : timing.playbacksToRemove){
    if (id != groupId){
      playbacksToRemoveNew.push_back(id);
    }
  }
  timing.playbacksToRemove = playbacksToRemoveNew;
}

void addAnimation(World& world, WorldTiming& timings, objid id, std::string animationToPlay, float initialTime, AnimationType animationType){
  auto groupId = getGroupId(world.sandbox, id);
  auto rootname = getGameObject(world, groupId).name;
  auto idScene = sceneId(world.sandbox, groupId);

  std::optional<BlendAnimationData> blendData = std::nullopt;
  if (timings.animations.playbacks.find(groupId) != timings.animations.playbacks.end()){
    modlog("animation", std::string("removing playback: ") + std::to_string(groupId));

    auto oldAnimation = timings.animations.playbacks.at(groupId).animation;
    auto oldInit = timings.animations.playbacks.at(groupId).initTime;

    blendData = BlendAnimationData {
      .oldAnimationInit = oldInit,
      .blendStartTime = initialTime,
      .animation = oldAnimation,
    };

    timings.animations.playbacks.erase(groupId);
    invalidatePlaybackToRemove(timings, groupId);

  }

  auto animation = getAnimation(world, groupId, animationToPlay).value();

  std::string animationname = animation.name;
  float animLength = animationLengthSeconds(animation);
  modlog("animation", std::string("adding animation: ") + animationname + ", length = " + std::to_string(animLength) + ", numticks = " + std::to_string(animation.duration) + ", ticks/s = " + std::to_string(animation.ticksPerSecond) + ", groupId = " + std::to_string(groupId));

  timings.animations.playbacks[groupId] = AnimationData {
    .groupId = groupId,
    .idScene = idScene,
    .rootname = rootname,
    .animation = animation,
    .animLength = animLength,
    .animationType = animationType,
    .initTime = initialTime,

    .blendData = blendData,
  };
}

void removeAnimation(World& world, WorldTiming& timings, objid id){
  auto groupId = getGroupId(world.sandbox, id);
  if (!idExists(world.sandbox, groupId)){
    return;
  }
  if (timings.animations.playbacks.find(groupId) == timings.animations.playbacks.end()){
    return;
  }
  modlog("animation", std::string("removing animation for obj: ") + std::to_string(id));
  timings.playbacksToRemove.push_back(groupId);
}

void disableAnimationIds(World& world, WorldTiming& timings, std::set<objid>& ids){
  timings.disableAnimationIds = ids;
}

std::set<objid> emptySet;
void setAnimationPose(World& world, objid id, std::string animationToPlay, float time){
  auto groupId = getGroupId(world.sandbox, id);
  auto animation = getAnimation(world, groupId, animationToPlay).value();
  auto idScene = sceneId(world.sandbox, groupId);
  auto newPoses = playbackAnimation(animation, time, idScene);
  setPoses(world, emptySet, idScene, newPoses);
}