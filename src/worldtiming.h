#ifndef MOD_WORLDTIMING
#define MOD_WORLDTIMING

#include <iostream>
#include <map>
#include "./scene/animation/playback.h"
#include "./scene/animation/timeplayback.h"
#include "./scene/animation/recorder.h"
#include "./scene/scene.h"

struct AnimationState {
  std::map<int32_t, TimePlayback> playbacks;
};

struct WorldTiming {
  AnimationState animations;
  std::vector<int32_t> playbacksToRemove;
  float initialTime;
};

WorldTiming createWorldTiming(float initialTime);
void tickAnimations(WorldTiming& timings, float elapsedTime);
void addAnimation(World& world, WorldTiming& timings, objid id, std::string animationToPlay);
void removeAnimation(World& world, WorldTiming& timings, objid id);

#endif