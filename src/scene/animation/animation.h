#ifndef MOD_ANIMATION
#define MOD_ANIMATION

#include <functional>
#include <iostream>
#include "./animationdebug.h"
#include "../common/util/loadmodel.h"

struct AnimationPose {
  objid targetId;
  Transformation pose;
};

std::vector<AnimationPose> animationPosesAtTime(float currentTime, objid sceneId, AnimationWithIds& animationWithIds);

#endif