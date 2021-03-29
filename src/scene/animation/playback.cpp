#include "./playback.h"

std::map<std::string, glm::mat4> initialBonePoses;
void updateBonePoses(NameAndMesh meshNameToMeshes, std::function<glm::mat4(std::string, std::string)> getModelMatrix){
  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    for (Bone& bone : mesh.bones){
      auto bonename = bone.name;
      auto boneTransform =  getModelMatrix(bone.name, bone.skeletonBase);
      if (initialBonePoses.find(bone.name) == initialBonePoses.end()){
        initialBonePoses[bone.name] = boneTransform;
      }
      bone.offsetMatrix = boneTransform * glm::inverse(initialBonePoses.at(bone.name)) ;
    }
  }
}

void playbackAnimation(
  Animation animation,  
  NameAndMesh meshNameToMeshes,  
  float currentTime, 
  float elapsedTime, 
  std::function<glm::mat4(std::string, std::string)> getModelMatrix,
  std::function<void(std::string, glm::mat4)> setPose
){  
  auto posesForTick = animationPosesAtTime(animation, currentTime, elapsedTime);
  for (auto pose : posesForTick){
    setPose(pose.channelName, pose.pose);
  }
  updateBonePoses(meshNameToMeshes, getModelMatrix);
}