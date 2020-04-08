#ifndef MOD_ANIMATIONDEBUG
#define MOD_ANIMATIONDEBUG

#include "../common/mesh.h"

void printAnimationInfo(Animation& animation, float currentTime, float elapsedTime, float currentTick);
void printChannelInfo(AnimationChannel& channel, int tickPosIndex, int tickRotIndex, int tickScaleIndex);

#endif