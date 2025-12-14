#ifndef MOD_EFFEKSEER
#define MOD_EFFEKSEER

#include <iostream>
#include <codecvt>
#include <Effekseer.h>
#include <EffekseerRendererGL.h>
#include "../../../common/util.h"

void onEffekSeekerFrame();
void onEffekSeekerRender(float windowSizeX, float windowSizeY, float fovRadians,  glm::vec3 viewPosition, glm::quat viewDirection, float nearPlane, float farPlane);

struct EffekEffect {
	objid effectId;
};

EffekEffect createEffect(std::string effec, glm::vec3 position, glm::quat rotation);
void freeEffect(EffekEffect& effect);
void setEffectState(EffekEffect& effect, bool loopContinuously);
void playEffect(EffekEffect& effect, glm::vec3 position);
void stopEffect(EffekEffect& effect);
void updateEffectPosition(EffekEffect& effectEffect, glm::vec3 position, glm::quat rotation);

void reloadEffect(std::string file);

int effekSeekerTriangleCount();
int effekSeekerDrawCount();

#endif
