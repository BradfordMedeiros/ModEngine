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

bool resetInitialPose = true;
void tickAnimations(World& world, WorldTiming& timings, float elapsedTime){
  //std::cout << "num active playbacks: " << timings.animations.playbacks.size() << std::endl;
  for (auto &[id, playback] : timings.animations.playbacks){
    playback.setElapsedTime(elapsedTime);
  }
  for (auto groupId : timings.playbacksToRemove){
    if (resetInitialPose && idExists(world.sandbox, groupId)){
      resetMeshBones(world, groupId);
    }
    timings.animations.playbacks.erase(groupId);
  }
  timings.playbacksToRemove.clear();
  //std::cout << "num animations: " << timings.animations.playbacks.size() << std::endl;
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

void updateBonePose(World& world, objid id){
  auto groupId = getGroupId(world.sandbox, id);
  auto idScene = sceneId(world.sandbox, groupId);
  auto rootname = getGameObject(world, groupId).name;
  auto meshNameToMeshes = getMeshesForGroupId(world, groupId);  
  updateBonePoses(meshNameToMeshes, scopeGetModelMatrix(world, idScene), rootname); 
}

void addAnimation(World& world, WorldTiming& timings, objid id, std::string animationToPlay, float initialTime, bool loop){
  //modassert(!loop, "add animation loop not yet implemented");
  auto groupId = getGroupId(world.sandbox, id);
  auto rootname = getGameObject(world, groupId).name;
  auto idScene = sceneId(world.sandbox, groupId);
  if (timings.animations.playbacks.find(groupId) != timings.animations.playbacks.end()){
    timings.animations.playbacks.erase(groupId);
  }

  auto animation = getAnimation(world, groupId, animationToPlay).value();
  std::string animationname = animation.name;
  float animLength = animationLengthSeconds(animation);
  modlog("animation", std::string("adding animation: ") + animationname + ", length = " + std::to_string(animLength) + ", numticks = " + std::to_string(animation.duration) + ", ticks/s = " + std::to_string(animation.ticksPerSecond));

  TimePlayback playback(
    initialTime, 
    [&world, &timings, animation, groupId, idScene, rootname, initialTime, loop](float currentTime, float elapsedTime) -> void { 
      // might be better to not have this check here and instead just assume obj exists,
      // remove animation when del object, but for now!
      if (!idExists(world.sandbox, groupId)){  // why are we doing this check here?
        timings.playbacksToRemove.push_back(groupId);
        return;
      }

      modlog("animation", "ticking animation for groupid: " + std::to_string(groupId));
      auto meshNameToMeshes = getMeshesForGroupId(world, groupId);  
      playbackAnimation(animation, meshNameToMeshes, currentTime - initialTime, elapsedTime, scopeGetModelMatrix(world, idScene), scopeSetPose(world, idScene), rootname);
    }, 
    [groupId, &timings, animationname]() -> void { 
      std::cout << "INFO: onfinish: " << animationname << " remove: " << groupId << std::endl;
      timings.playbacksToRemove.push_back(groupId);
    },
    animLength,
    PAUSE
  );  
  timings.animations.playbacks[groupId] = playback;
}

void removeAnimation(World& world, WorldTiming& timings, objid id){
  modlog("animation", std::string("removing animation for obj: ") + std::to_string(id));
  auto groupId = getGroupId(world.sandbox, id);
  timings.playbacksToRemove.push_back(groupId);
}