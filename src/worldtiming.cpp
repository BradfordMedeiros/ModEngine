#include "./worldtiming.h"

std::optional<objid> getGameObjectByName(std::string name, objid sceneId);

WorldTiming createWorldTiming(float initialTime){
  AnimationState animations;
  std::vector<PlaybackToRemove> playbacksToRemove;

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

// Probably could remove the excessive copying. 
// Probably the keyframes shouldn't be stored in the playback and then 
// the copying doesnt matter
void removePlayback(WorldTiming& timings, objid groupId, int zIndex){
  std::vector<AnimationLayer> newLayers;
  for (auto &layer : timings.animations.playbacks.at(groupId).layer){
    if (!layer.zIndex == zIndex){
      newLayers.push_back(layer);
    }
  }

  if (newLayers.size() == 0){
    timings.animations.playbacks.erase(groupId);
  }else{
    timings.animations.playbacks.at(groupId).layer = newLayers;
  }
}


void setPoses(World& world, objid idScene, std::vector<AnimationPose>& poses){
  for (auto& pose : poses){
    physicsLocalTransformSet(world, pose.targetId, pose.pose, pose.directIndex);
    //printMatrixInformation(pose.pose, std::string("SET_CHANNEL:") + pose.channelName);
  }
}

std::vector<float> computeLayerWeights(AnimationData& playback, float currentTime){
  float blendTime = 0.2f;

  float totalWeightUsed = 0.f;

  std::vector<float> weights(playback.layer.size(), 0.f);
  for (int i = (playback.layer.size() - 1); i >= 0; i--){
    float weight = 1.f;
    if (i != 0){
      float elapsedTime = currentTime - playback.layer.at(i).initTime;
      weight = elapsedTime / blendTime;
      if (weight > 1.f){
        weight = 1.f;
      }
    }
    float remainingWeight = 1.f - totalWeightUsed;
    modassert(remainingWeight >= 0.f, "remaining weight should be bigger");
    if (weight > remainingWeight){
      weight = remainingWeight;
    }

    weights[i] = weight;
    totalWeightUsed += weight;
  }
  return weights;
}

Transformation blendTransforms(std::vector<Transformation>& transforms, std::vector<float>& weights){
    modassert(transforms.size() > 0, "no transforms");
    Transformation result = transforms.at(0);
    float accumulatedWeight = weights.at(0);
    for (size_t i = 1; i < transforms.size(); ++i) {
        float w = weights.at(i);
        float blendAmount = w / (accumulatedWeight + w); // relative weight

        result.position = glm::lerp(result.position, transforms[i].position, blendAmount);
        result.scale = glm::lerp(result.scale,    transforms[i].scale,    blendAmount);

        result.rotation = glm::slerp(result.rotation, transforms[i].rotation, blendAmount);
        accumulatedWeight += w;
    }
    return result;
}

AnimationPose calculateCombinedPose(std::vector<AnimationPose*> poses, std::vector<float>& layerWeights){
  bool allNullPoses = true;
  for (auto& pose : poses){
    if (pose != NULL){
      allNullPoses = false;
      break;
    }
  }
  modassert(!allNullPoses, "all poses were null");

  AnimationPose basePose{};
  for (auto& pose : poses){
    if (pose != NULL){
      basePose.targetId = pose -> targetId;
      basePose.directIndex = pose -> directIndex;
      break;
    }
  }
  std::vector<Transformation> transforms;
  std::vector<float> channelWeights;
  for (int i = 0; i < poses.size(); i++){
    auto& pose = poses.at(i);
    if (pose != NULL){
      transforms.push_back(pose -> pose);
      channelWeights.push_back(layerWeights.at(i));
    }
  }
  basePose.pose = blendTransforms(transforms, channelWeights);

  return basePose;
}

// TODO - probably slow, could precompute stuff
std::vector<AnimationPose> blendPoses(std::vector<std::vector<AnimationPose>>& layers, std::vector<float>& layerWeights){
  std::unordered_set<objid> ids;
  for (auto &layer : layers){
    for (auto& pose : layer){
      ids.insert(pose.targetId);
    }
  }

  std::vector<AnimationPose> finalPose;

  for (auto targetId : ids){
    std::vector<AnimationPose*> poseForLayer;
    for (auto &layer : layers){
      bool foundLayer = false;
      for (auto& pose : layer){
        if (pose.targetId == targetId){
          foundLayer = true;
          poseForLayer.push_back(&pose);
          break;
        }
      }
      if (!foundLayer){
        poseForLayer.push_back(NULL);
      }
      modassert(poseForLayer.size() > 0, "zero poses for layer");
    }    
    finalPose.push_back(calculateCombinedPose(poseForLayer, layerWeights));
  }
  return finalPose;
}

void tickAnimation(World& world, AnimationData& playback, float currentTime){
  std::cout << "debug1 tickAnimation -----------------------_" << std::endl << "debug1 ";
  for (auto& layer : playback.layer){
    std::cout << layer.zIndex << " ";
  }
  std::cout << "\ndebug1tickAnimation end-----------------------_" << std::endl;

  auto layerWeights = computeLayerWeights(playback, currentTime);
  std::cout << "layerWeights: " << print(layerWeights) << std::endl;
  std::vector<std::vector<AnimationPose>> layerPoses;

  bool shouldBlendPoses = false;

  for (auto& layer : playback.layer){ // These are sorted
    auto elapsedTime = currentTime - layer.initTime;
    if (enableBlending && layer.blendData.has_value()){
      float timeElapsedBlendStart = currentTime - layer.blendData.value().blendStartTime;
      float aFactor = glm::min(1.f, timeElapsedBlendStart / blendingWindow);
      // if afactor > 1.f or something like that, could get rid of the old animation value
      //modassert(false, "blend not yet supported");
      modlog("tickAnimation", std::to_string(aFactor) + ", " + std::to_string(timeElapsedBlendStart));
      modlog("tickAnimation 1", std::to_string(playback.layer.size()));
      auto newPoses = playbackAnimationBlend(
        layer.animation,
        layer.blendData.value().animation, 
        elapsedTime,
        currentTime - layer.blendData.value().oldAnimationInit, 
        aFactor,
        playback.idScene
      );
      if (shouldBlendPoses){
        layerPoses.push_back(newPoses);
      }else{
        setPoses(world, playback.idScene, newPoses);    
      }
    }else{
      auto newPoses = playbackAnimation(layer.animation, elapsedTime, playback.idScene);
      if (shouldBlendPoses){
        layerPoses.push_back(newPoses);
      }else{
        setPoses(world, playback.idScene, newPoses);    
      }
    }    
  }

  if (shouldBlendPoses){
    auto blendedPoses = blendPoses(layerPoses, layerWeights);
    setPoses(world, playback.idScene, blendedPoses);    
  }

}

void tickAnimations(World& world, WorldTiming& timings, float currentTime){
  //std::cout << "animations num active playbacks: " << timings.animations.playbacks.size() << std::endl;
  for (auto &[id, playback] : timings.animations.playbacks){
    // might be better to not have this check here and instead just assume obj exists,
    // remove animation when del object, but for now!
    if (!idExists(world.sandbox, playback.groupId)){  // why are we doing this check here?
      auto& layers = timings.animations.playbacks.at(playback.groupId).layer;
      for (int i = 0; i < layers.size(); i++){
        timings.playbacksToRemove.push_back(
          PlaybackToRemove {
            .id = playback.groupId,
            .zIndex = layers.at(i).zIndex,
          }
        );        
      }

      modlog("animation", std::string("removed playbacks because of internal group id: ") + std::to_string(playback.groupId));
    }else{
      tickAnimation(world, playback, currentTime);
    }
    //modlog("animation", "ticking animation for groupid: " + std::to_string(playback.groupId));
  }


  for (auto &[id, playback] : timings.animations.playbacks){
    for (auto& layer : playback.layer){
      float timeElapsed = currentTime - layer.initTime;
      if (timeElapsed > layer.animLength){
        modlog("animation", "on animation finish");
        if (layer.animationType == ONESHOT){
          timings.playbacksToRemove.push_back(PlaybackToRemove {
            .id = playback.groupId,
            .zIndex = layer.zIndex,
          });
          continue;
        }else if (layer.animationType == LOOP){
          layer.initTime = currentTime;
          continue;
        }else if (layer.animationType == FORWARDS){
          // do nothing
          continue;
        }
        modassert(false, "invalid animationType");
      }
    }
  }
  //std::cout << "num playbacks: " << timings.animations.playbacks.size() << std::endl;
  for (auto& playback : timings.playbacksToRemove){
    removePlayback(timings, playback.id, playback.zIndex);
    //modlog("animation", std::string("playbacks to remove, removing: ") + std::to_string(groupId));
  }
  timings.playbacksToRemove.clear();
}

AnimationWithIds resolveAnimationIds(World& world, Animation& animation, objid sceneId, std::optional<std::set<objid>> mask, bool invertMask) {
  std::vector<objid> channelObjIds;
  std::vector<objid> channelObjDirectIds;
  std::vector<KeyInfoLookup> lookup;
  for (auto &channel : animation.channels){
    auto id = getGameObjectByName(channel.nodeName, sceneId).value();
    channelObjIds.push_back(id);

    auto directIndexId = getDirectIndexForId(world.sandbox, id);
    channelObjDirectIds.push_back(directIndexId);
    lookup.push_back(KeyInfoLookup{});
  }
  return AnimationWithIds {
    .animation = animation,
    .channelObjIds = channelObjIds,
    .channelObjDirectIds = channelObjDirectIds,
    .lookup = lookup,
    .mask = mask,
    .invertMask = invertMask,
  };
}

std::optional<AnimationWithIds> getAnimation(World& world, int32_t groupId, std::string animationToPlay, std::optional<std::set<objid>> mask, bool invertMask){  
  if (world.animations.find(groupId) == world.animations.end()){
    return std::nullopt;
  }
  for (auto& animation :  world.animations.at(groupId)){
    if (animation.name == animationToPlay){
      auto idForScene = sceneId(world.sandbox, groupId);
      return resolveAnimationIds(world, animation, idForScene, mask, invertMask);
    }
  }
  std::cout << "ERROR: no animation found named: " << animationToPlay << std::endl;
  std::cout << "ERROR INFO: existing animation names [" << world.animations.at(groupId).size() << "] - ";
  for (auto& animation : world.animations.at(groupId)){
    std::cout << animation.name << " ";
  }
  std::cout << std::endl;
  return  std::nullopt;
}

void invalidatePlaybackToRemove(WorldTiming& timing, objid groupId, int zIndex){
  std::vector<PlaybackToRemove> playbacksToRemoveNew;
  for (auto& playback : timing.playbacksToRemove){
    bool matchingPlayback = ((playback.id == groupId) && (playback.zIndex == zIndex));
    if (!matchingPlayback){
      playbacksToRemoveNew.push_back(playback);
    }
  }
  timing.playbacksToRemove = playbacksToRemoveNew;
}


bool hasAnimationLayer(WorldTiming& timings, objid groupId, int zIndex){
  if (timings.animations.playbacks.find(groupId) == timings.animations.playbacks.end()){
    return false;
  }
  auto& layers = timings.animations.playbacks.at(groupId).layer;
  for (auto& layer : layers){
    if (layer.zIndex == zIndex){
      return true;
    }
  }
  return false;
}

void sortAnimationLayers(std::vector<AnimationLayer>& layers){
  std::sort(layers.begin(), layers.end(), [](AnimationLayer& layer1, AnimationLayer& layer2) -> bool { 
    return layer1.zIndex <= layer2.zIndex; 
  });
}

void addAnimation(World& world, WorldTiming& timings, objid id, std::string animationToPlay, float initialTime, AnimationType animationType, std::optional<std::set<objid>>& mask, int zIndex, bool invertMask){
  auto groupId = getGroupId(world.sandbox, id);
  auto rootname = getGameObject(world, groupId).name;
  auto idScene = sceneId(world.sandbox, groupId);

  int numAnimations = 0;
  {
    if(timings.animations.playbacks.find(groupId) != timings.animations.playbacks.end()){
      auto currAnimations = timings.animations.playbacks.at(groupId).layer.size();
      numAnimations += currAnimations;
    }
  }
  std::optional<int> currentIndex;
  {
    if (timings.animations.playbacks.find(groupId) != timings.animations.playbacks.end()){
      AnimationData& animationData = timings.animations.playbacks.at(groupId);
      std::vector<AnimationLayer>& layers = animationData.layer;
      for (int i = 0; i < layers.size(); i++){
        auto sameZIndex = layers.at(i).zIndex == zIndex;
        if (sameZIndex){
          currentIndex = i;
          break;
        }
      }
    }
  }

  modlog("animation zIndex = ", std::to_string(zIndex) + ", numAnimations = " + std::to_string(numAnimations) + ", currentIndex = " + print(currentIndex));
  ///////////////////////



  std::optional<BlendAnimationData> blendData = std::nullopt;
  if (currentIndex.has_value()){
    modlog("animation", std::string("removing playback: ") + std::to_string(groupId));

    auto& oldAnimation = timings.animations.playbacks.at(groupId).layer.at(currentIndex.value()).animation;
    auto oldInit = timings.animations.playbacks.at(groupId).layer.at(currentIndex.value()).initTime;

    blendData = BlendAnimationData {
      .oldAnimationInit = oldInit,
      .blendStartTime = initialTime,
      .animation = oldAnimation,
    };

    removePlayback(timings, groupId, zIndex);
    invalidatePlaybackToRemove(timings, groupId, zIndex);
  }

  auto animation = getAnimation(world, groupId, animationToPlay, mask.has_value() ? mask.value() : timings.disableAnimationIds, invertMask);
  modassert(animation.has_value(), std::string("animation does not exist: ") + animationToPlay);

  std::string& animationname = animation.value().animation.name;
  float animLength = animationLengthSeconds(animation.value().animation);
  modlog("animation", std::string("adding animation: ") + animationname + ", length = " + std::to_string(animLength) + ", numticks = " + std::to_string(animation.value().animation.duration) + ", ticks/s = " + std::to_string(animation.value().animation.ticksPerSecond) + ", groupId = " + std::to_string(groupId));

  modassert(!hasAnimationLayer(timings, groupId, zIndex), "animation already exists on this layer for this group id");

  if (timings.animations.playbacks.find(groupId) == timings.animations.playbacks.end()){
    timings.animations.playbacks[groupId] = AnimationData {
      .groupId = groupId,
      .idScene = idScene,
      .rootname = rootname,
      .layer = {},
    };  
  }

  auto& layers = timings.animations.playbacks.at(groupId).layer;
  layers.push_back(
    AnimationLayer {
        .zIndex = zIndex,
        .animation = animation.value(),
        .animationType = animationType,
        .animLength = animLength,
        .initTime = initialTime,
        .blendData = blendData,
    }
  );

  sortAnimationLayers(layers);
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

  auto layers = timings.animations.playbacks.at(groupId).layer;
  for (int i = 0; i < layers.size(); i++){
    timings.playbacksToRemove.push_back(
      PlaybackToRemove { 
        .id = groupId,
        .zIndex = layers.at(i).zIndex,
      }
    );
  }
}

void disableAnimationIds(World& world, WorldTiming& timings, std::set<objid>& ids){
  timings.disableAnimationIds = ids;
}

void setAnimationPose(World& world, objid id, std::string animationToPlay, float time){
  auto groupId = getGroupId(world.sandbox, id);
  auto animation = getAnimation(world, groupId, animationToPlay, std::nullopt, false).value();
  auto idScene = sceneId(world.sandbox, groupId);
  auto newPoses = playbackAnimation(animation, time, idScene);
  setPoses(world, idScene, newPoses);
}


std::optional<float> animationLengthSeconds(World& world, objid id, std::string& animationToPlay){
  auto groupId = getGroupId(world.sandbox, id);
  auto animation = getAnimation(world, groupId, animationToPlay, std::nullopt, false);
  if (!animation.has_value()){
    return std::nullopt;
  }
  float animLength = animationLengthSeconds(animation.value().animation);
  return animLength;
}
