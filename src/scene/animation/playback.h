#ifndef MOD_PLAYBACK
#define MOD_PLAYBACK

#include <string>
#include <map>
#include <glm/glm.hpp>
#include "../common/mesh.h"
#include "./animation.h"

glm::mat4 getParentBone(std::map<std::string, std::string>& boneToParent, std::map<std::string, glm::mat4>& nodeToTransformedPose, std::string boneName);
void processNewPoseOnMesh(std::map<std::string, glm::mat4>& nodeToTransformedPose, std::string boneName, glm::mat4 newPose, NameAndMesh& meshData);
void playbackAnimation(Animation& animation, std::map<std::string, std::map<std::string, std::string>>& meshnameToBoneToParent, NameAndMesh& meshNameToMeshes, float currentTime, float elapsedTime);

#endif