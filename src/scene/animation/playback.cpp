#include "./playback.h"

// https://gamedev.net/forums/topic/484984-skeletal-animation-non-uniform-scale/4172731/
void updateBonePoses(NameAndMeshObjName meshNameToMeshes, std::function<glm::mat4(std::string, std::string)> getModelMatrix, std::string rootname){
  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    std::string meshname = meshNameToMeshes.objnames.at(i);
    for (Bone& bone : mesh.bones){
      bone.offsetMatrix = getModelMatrix(bone.name, rootname) * glm::inverse(bone.initialBonePose);
    }
  }
}

void playbackAnimation(
  Animation animation,  
  NameAndMeshObjName meshNameToMeshes,  
  float currentTime, 
  std::function<glm::mat4(std::string, std::string)> getModelMatrix,
  std::function<void(std::string, glm::mat4)> setPose,
  std::string rootname
){  
  auto posesForTick = animationPosesAtTime(animation, currentTime);
  for (auto pose : posesForTick){
    //printMatrixInformation(pose.pose, std::string("SET_CHANNEL:") + pose.channelName);
    setPose(pose.channelName, pose.pose);
  }
  updateBonePoses(meshNameToMeshes, getModelMatrix, rootname);
}

void playbackAnimationBlend(
  Animation animation,  
  NameAndMeshObjName meshNameToMeshes,  
  float currentTime, 
  std::function<glm::mat4(std::string, std::string)> getModelMatrix,
  std::function<void(std::string, glm::mat4)> setPose,
  std::string rootname
){  
  auto posesForTick = animationPosesAtTime(animation, currentTime);
  for (auto pose : posesForTick){
    //printMatrixInformation(pose.pose, std::string("SET_CHANNEL:") + pose.channelName);
    setPose(pose.channelName, pose.pose);
  }
  updateBonePoses(meshNameToMeshes, getModelMatrix, rootname);
}