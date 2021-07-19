#include "./playback.h"

void updateBonePoses(NameAndMesh meshNameToMeshes, std::function<glm::mat4(std::string, std::string)> getModelMatrix, std::function<glm::mat4(Bone&)> getInitialPose){
  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    for (Bone& bone : mesh.bones){
      bone.offsetMatrix = getModelMatrix(bone.name, bone.skeletonBase) * glm::inverse(getInitialPose(bone));
    }
  }
}

void playbackAnimation(
  Animation animation,  
  NameAndMesh meshNameToMeshes,  
  float currentTime, 
  float elapsedTime, 
  std::function<glm::mat4(std::string, std::string)> getModelMatrix,
  std::function<void(std::string, glm::mat4)> setPose,
  std::function<glm::mat4(Bone&)> getInitialPose
){  
  auto posesForTick = animationPosesAtTime(animation, currentTime, elapsedTime);
  for (auto pose : posesForTick){
    setPose(pose.channelName, pose.pose);
  }
  updateBonePoses(meshNameToMeshes, getModelMatrix, getInitialPose);
}