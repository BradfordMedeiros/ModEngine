#ifndef MOD_ANIMATION
#define MOD_ANIMATION

#include <functional>
#include <iostream>
#include "./animationdebug.h"
#include "../common/util/loadmodel.h"

struct AnimationPose {
  std::string channelName;
  Transformation pose;
};

std::vector<AnimationPose> animationPosesAtTime(Animation& animation, float currentTime);

#endif