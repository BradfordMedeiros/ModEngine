#ifndef MOD_WORLDTIMING
#define MOD_WORLDTIMING

#include <iostream>
#include <map>
#include "./scene/animation/playback.h"
#include "./scene/animation/recorder.h"
#include "./scene/scene.h"

struct BlendAnimationData {
  float oldAnimationInit;
  float blendStartTime;
  Animation animation;
};

struct AnimationData {
  objid groupId;
  objid idScene;
  std::string rootname;
  Animation animation;
  float animLength;
  AnimationType animationType;
  float initTime;


  std::optional<BlendAnimationData> blendData;
};

struct AnimationState {
  std::map<int32_t, AnimationData> playbacks;
};

struct WorldTiming {
  AnimationState animations;
  std::vector<int32_t> playbacksToRemove;
  float initialTime;
};

WorldTiming createWorldTiming(float initialTime);
void tickAnimations(World& world, WorldTiming& timings, float currentTime);
void updateBonePose(World& world, objid id);

void addAnimation(World& world, WorldTiming& timings, objid id, std::string animationToPlay, float initialTime, AnimationType animationType);
void removeAnimation(World& world, WorldTiming& timings, objid id);

#endif