#ifndef MOD_PLAYBACK
#define MOD_PLAYBACK

#include <string>
#include <map>
#include <glm/glm.hpp>
#include "../common/mesh.h"
#include "./animation.h"

std::vector<AnimationPose> playbackAnimation(
  Animation& animation, 
  float currentTime,
  objid sceneId
);



std::vector<AnimationPose> playbackAnimationBlend(
  Animation& animation,
  Animation& animation2,
  float currentTime,
  float currentTimeAnimation2,
  float blendFactor,
  objid sceneId
);

#endif