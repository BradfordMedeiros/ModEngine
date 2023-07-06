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

void resetMeshBones(World& world, objid groupId){
  auto meshNameToMeshes = getMeshesForGroupId(world, groupId);  
  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    for (Bone& bone : mesh.bones){
      bone.offsetMatrix = glm::mat4(1.f);
    }
  }
}

glm::mat4 armatureTransform(SceneSandbox& sandbox, objid id, std::string skeletonRoot, objid sceneId){
  auto gameobj = maybeGetGameObjectByName(sandbox, skeletonRoot, sceneId, false);
  assert(gameobj.has_value());
 
  auto groupTransform = fullModelTransform(sandbox, gameobj.value() -> id);
  auto modelTransform = fullModelTransform(sandbox, id);
  // group * something = model (aka aX = b, so X = inv(A) * B)
  // inverse(group) * model
  //auto groupToModel =  modelTransform * glm::inverse(groupTransform); 
  auto groupToModel =  glm::inverse(groupTransform) * modelTransform; 

  auto resultCheck = groupTransform * groupToModel;
  if (false && resultCheck != modelTransform){
    std::cout << "group_to_model = " << print(groupToModel) << std::endl;
    std::cout << "result_check = " << print(resultCheck) << std::endl;
    std::cout << "model_transform = " << print(modelTransform) << std::endl;
    assert(false);
  }
  return groupToModel;
}


glm::mat4 getModelMatrix(World& world, objid idScene, std::string name, std::string skeletonRoot){
  auto gameobj =  maybeGetGameObjectByName(world.sandbox, name, idScene, false);
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
    auto gameobj =  maybeGetGameObjectByName(world.sandbox, name, idScene, false);
    if (gameobj.has_value()){
      physicsLocalTransformSet(world, gameobj.value() -> id, getTransformationFromMatrix(pose));
    }else{
      std::cout << "warning no bone node named: " << name << std::endl;
      assert(false);
    }
  };
}

bool resetInitialPose = true;
bool enableBlending = true;
void tickAnimations(World& world, WorldTiming& timings, float currentTime){
  //std::cout << "animations num active playbacks: " << timings.animations.playbacks.size() << std::endl;
  for (auto &[id, playback] : timings.animations.playbacks){
    // might be better to not have this check here and instead just assume obj exists,
    // remove animation when del object, but for now!
    if (!idExists(world.sandbox, playback.groupId)){  // why are we doing this check here?
      timings.playbacksToRemove.push_back(playback.groupId);
      modlog("animation", "removed playbacks because of internal group id");
      return;
    }
    modlog("animation", "ticking animation for groupid: " + std::to_string(playback.groupId));
    auto meshNameToMeshes = getMeshesForGroupId(world, playback.groupId);

    if (enableBlending && playback.blendData.has_value()){
      float timeElapsedBlendStart = currentTime - playback.blendData.value().blendStartTime;
      float aFactor = glm::min(1.f, timeElapsedBlendStart / 0.5f);

      // if afactor > 1.f or something like that, could get rid of the old animation value

      //modassert(false, "blend not yet supported");
      playbackAnimationBlend(
        playback.animation,
        playback.blendData.value().animation, 
        currentTime - playback.initTime,
        currentTime - playback.blendData.value().oldAnimationInit, 
        aFactor,
        meshNameToMeshes, 
        scopeGetModelMatrix(world, playback.idScene), 
        scopeSetPose(world, playback.idScene), 
        playback.rootname
      );
    }else{
      playbackAnimation(
        playback.animation, 
        currentTime - playback.initTime, 
        meshNameToMeshes, 
        scopeGetModelMatrix(world, playback.idScene), 
        scopeSetPose(world, playback.idScene), 
        playback.rootname
      );
    }
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

  std::cout << "num playbacks: " << timings.animations.playbacks.size() << std::endl;

  for (auto groupId : timings.playbacksToRemove){
    if (resetInitialPose && idExists(world.sandbox, groupId)){
      resetMeshBones(world, groupId);
    }
    timings.animations.playbacks.erase(groupId);
    //modlog("animation", std::string("playbacks to remove, removing: ") + std::to_string(groupId));
  }
  timings.playbacksToRemove.clear();
}

std::optional<Animation> getAnimation(World& world, int32_t groupId, std::string animationToPlay){  
  Animation noAnimation { };
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


void updateBonePose(World& world, objid id){
  auto groupId = getGroupId(world.sandbox, id);
  auto idScene = sceneId(world.sandbox, groupId);
  auto rootname = getGameObject(world, groupId).name;
  auto meshNameToMeshes = getMeshesForGroupId(world, groupId);  
  updateBonePoses(meshNameToMeshes, scopeGetModelMatrix(world, idScene), rootname); 
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
  modlog("animation", std::string("adding animation: ") + animationname + ", length = " + std::to_string(animLength) + ", numticks = " + std::to_string(animation.duration) + ", ticks/s = " + std::to_string(animation.ticksPerSecond));

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
  modlog("animation", std::string("removing animation for obj: ") + std::to_string(id));
  auto groupId = getGroupId(world.sandbox, id);
  timings.playbacksToRemove.push_back(groupId);
}
