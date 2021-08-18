#include "./worldtiming.h"

WorldTiming createWorldTiming(float initialTime){
  AnimationState animations;
  std::vector<int32_t> playbacksToRemove;

  WorldTiming timing {
    .animations = animations,
    .playbacksToRemove = playbacksToRemove,
    .initialTime = initialTime,
  };
  return timing;
}

void tickAnimations(WorldTiming& timings, float elapsedTime){
  for (auto groupId : timings.playbacksToRemove){
    timings.animations.playbacks.erase(groupId);
  }
  for (auto &[id, playback] : timings.animations.playbacks){
    playback.setElapsedTime(elapsedTime);
  }
  timings.playbacksToRemove.clear();
}

Animation getAnimation(World& world, int32_t groupId, std::string animationToPlay){  
  Animation noAnimation { };
  for (auto animation :  world.animations.at(groupId)){
    if (animation.name == animationToPlay){
      return animation;
    }
  }
  std::cout << "no animation found" << std::endl;
  assert(false);
  return  noAnimation;  // @todo probably use optional here.
}


glm::mat4 getModelMatrix(World& world, objid idScene, std::string name, std::string skeletonRoot){
  auto gameobj =  maybeGetGameObjectByName(world.sandbox, name, idScene);
  if (gameobj.has_value()){
    return armatureTransform(world.sandbox, gameobj.value() -> id, skeletonRoot, idScene);
  }
  std::cout << "no value: " << name << std::endl;
  assert(false);
  return glm::mat4(1.f);   
}

std::function<glm::mat4(std::string, std::string)> scopeGetModelMatrix(World& world, objid idScene){
  return [&world, idScene](std::string name, std::string skeletonRoot) -> glm::mat4 {
    return getModelMatrix(world, idScene, name, skeletonRoot);
  };
}

std::function<void(std::string name, glm::mat4 pose)> scopeSetPose(World& world, objid idScene){
  return [&world, idScene](std::string name, glm::mat4 pose) -> void {
    auto gameobj =  maybeGetGameObjectByName(world.sandbox, name, idScene);
    if (gameobj.has_value()){
      updateRelativeTransform(world.sandbox, gameobj.value() -> id, getTransformationFromMatrix(pose));
    }else{
      std::cout << "warning no bone node named: " << name << std::endl;
      assert(false);
    }
  };
}

void updateBonePose(World& world, objid id){
  auto groupId = getGroupId(world.sandbox, id);
  auto idScene = sceneId(world.sandbox, id);
  auto meshNameToMeshes = getMeshesForGroupId(world, groupId);  
  updateBonePoses(meshNameToMeshes, scopeGetModelMatrix(world, idScene)); 
}

void addAnimation(World& world, WorldTiming& timings, objid id, std::string animationToPlay){
  auto groupId = getGroupId(world.sandbox, id);
  auto idScene = sceneId(world.sandbox, id);
  if (timings.animations.playbacks.find(groupId) != timings.animations.playbacks.end()){
    timings.animations.playbacks.erase(groupId);
  }

  auto animation = getAnimation(world, groupId, animationToPlay);
  TimePlayback playback(
    timings.initialTime, 
    [&world, animation, groupId, idScene](float currentTime, float elapsedTime) -> void { 
      auto meshNameToMeshes = getMeshesForGroupId(world, groupId);  
      playbackAnimation(animation, meshNameToMeshes, currentTime, elapsedTime, scopeGetModelMatrix(world, idScene), scopeSetPose(world, idScene));
    }, 
    [groupId, &timings]() -> void { 
      timings.playbacksToRemove.push_back(groupId);
    },
    animation.duration,
    PAUSE
  );  
  timings.animations.playbacks[groupId] = playback;
}

void removeAnimation(World& world, WorldTiming& timings, objid id){
  auto groupId = getGroupId(world.sandbox, id);
  timings.playbacksToRemove.push_back(groupId);
}