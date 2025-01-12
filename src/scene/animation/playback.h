#ifndef MOD_PLAYBACK
#define MOD_PLAYBACK

#include <string>
#include <map>
#include <glm/glm.hpp>
#include "../common/mesh.h"
#include "./animation.h"

void playbackAnimation(
  Animation& animation, 
  float currentTime, 
  std::function<void(std::string, Transformation)> setPose
);

void playbackAnimationBlend(
  Animation& animation,
  Animation& animation2,
  float currentTime,
  float currentTimeAnimation2,
  float blendFactor,
  std::function<void(std::string, Transformation)> setPose
);

#endif