#ifndef MOD_PLAYBACK
#define MOD_PLAYBACK

#include <string>
#include <map>
#include <glm/glm.hpp>
#include "../common/mesh.h"
#include "./animation.h"

void playbackAnimation(Animation& animation, std::map<std::string, std::map<std::string, std::string>>& meshnameToBoneToParent, NameAndMesh& meshNameToMeshes, float currentTime, float elapsedTime);

#endif