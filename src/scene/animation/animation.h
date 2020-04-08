#ifndef MOD_ANIMATION
#define MOD_ANIMATION

#include <functional>
#include <iostream>
#include "./animationdebug.h"
#include "../common/util/loadmodel.h"

void advanceAnimation(Animation& animation, float currentTime, float elapsedTime,  std::function<void(std::string, glm::mat4)> setBonePose);

#endif