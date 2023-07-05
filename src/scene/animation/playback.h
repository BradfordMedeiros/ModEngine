#ifndef MOD_PLAYBACK
#define MOD_PLAYBACK

#include <string>
#include <map>
#include <glm/glm.hpp>
#include "../common/mesh.h"
#include "./animation.h"

void playbackAnimation(
  Animation animation, 
  NameAndMeshObjName meshNameToMeshes, 
  float currentTime, 
  std::function<glm::mat4(std::string, std::string)> getModelMatrix,
  std::function<void(std::string, glm::mat4)> setPose,
  std::string rootname
);

void playbackAnimationBlend(
  Animation animation, 
  NameAndMeshObjName meshNameToMeshes, 
  float currentTime, 
  std::function<glm::mat4(std::string, std::string)> getModelMatrix,
  std::function<void(std::string, glm::mat4)> setPose,
  std::string rootname
);


void updateBonePoses(NameAndMeshObjName meshNameToMeshes, std::function<glm::mat4(std::string, std::string)> getModelMatrix, std::string rootname);

#endif