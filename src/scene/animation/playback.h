#ifndef MOD_PLAYBACK
#define MOD_PLAYBACK

#include <string>
#include <map>
#include <glm/glm.hpp>
#include "../common/mesh.h"
#include "./animation.h"

void playbackAnimation(Animation animation, 
  NameAndMesh meshNameToMeshes, 
  float currentTime, 
  float elapsedTime, 
  std::function<glm::mat4(std::string)> getModelMatrix,
  std::function<void(std::string, glm::mat4)> setPose
);

void updateBonePoses(NameAndMesh meshNameToMeshes, std::function<glm::mat4(std::string)> getModelMatrix);

#endif