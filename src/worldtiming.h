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
  AnimationWithIds animation;
  std::optional<std::set<objid>> mask;
};
struct AnimationLayer {
  int zIndex;
  AnimationWithIds animation;
  AnimationType animationType;
  float animLength;
  float initTime;
  std::optional<BlendAnimationData> blendData;
};

struct AnimationData {
  objid groupId;
  objid idScene;
  std::string rootname;
  std::vector<AnimationLayer> layer;
};

struct AnimationState {
  std::unordered_map<int32_t, AnimationData> playbacks;
};

struct PlaybackToRemove {
  int32_t id;
  int zIndex;
};
struct WorldTiming {
  AnimationState animations;
  std::set<objid> disableAnimationIds;
  std::vector<PlaybackToRemove> playbacksToRemove;
  float initialTime;
};

WorldTiming createWorldTiming(float initialTime);
void tickAnimations(World& world, WorldTiming& timings, float currentTime);

void addAnimation(World& world, WorldTiming& timings, objid id, std::string animationToPlay, float initialTime, AnimationType animationType, std::optional<std::set<objid>>& mask, int zIndex, bool invertMask);
void removeAnimation(World& world, WorldTiming& timings, objid id);
void disableAnimationIds(World& world, WorldTiming& timings, std::set<objid>& ids);
void setAnimationPose(World& world, objid id, std::string animationToPlay, float time);
std::optional<float> animationLengthSeconds(World& world, objid id, std::string& animationToPlay);

#endif