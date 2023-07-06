#ifndef MOD_PLAYBACK
#define MOD_PLAYBACK

#include <string>
#include <map>
#include <glm/glm.hpp>
#include "../common/mesh.h"
#include "./animation.h"

void playbackAnimation(
  Animation animation, 
  float currentTime, 
  NameAndMeshObjName meshNameToMeshes, 
  std::function<glm::mat4(std::string, std::string)> getModelMatrix,
  std::function<void(std::string, Transformation)> setPose,
  std::string rootname
);

void playbackAnimationBlend(
  Animation animation,
  Animation animation2,
  float currentTime,
  float currentTimeAnimation2,
  float blendFactor,
  NameAndMeshObjName meshNameToMeshes, 
  std::function<glm::mat4(std::string, std::string)> getModelMatrix,
  std::function<void(std::string, Transformation)> setPose,
  std::string rootname
);


void updateBonePoses(NameAndMeshObjName meshNameToMeshes, std::function<glm::mat4(std::string, std::string)> getModelMatrix, std::string rootname);

#endif