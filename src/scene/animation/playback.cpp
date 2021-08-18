#include "./playback.h"


glm::mat4 getInitialPose(Bone& bone, std::function<glm::mat4(std::string, std::string)>& getModelMatrix){
  auto boneTransform =  getModelMatrix(bone.name, bone.skeletonBase);
  if (!bone.initialPoseSet){
    bone.initialBonePose = boneTransform;
    bone.initialPoseSet = true;
  }
  return bone.initialBonePose;
}

void updateBonePoses(NameAndMesh meshNameToMeshes, std::function<glm::mat4(std::string, std::string)> getModelMatrix){
  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    for (Bone& bone : mesh.bones){
      bone.offsetMatrix = getModelMatrix(bone.name, bone.skeletonBase) * glm::inverse(getInitialPose(bone, getModelMatrix));
    }
  }
}

/*void updateBonePoses(NameAndMesh meshNameToMeshes, std::function<glm::mat4(std::string, std::string)> getModelMatrix, std::function<glm::mat4(Bone&)> getInitialPose){
  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    auto meshName = meshNameToMeshes.meshNames.at(i);
    for (Bone& bone : mesh.bones){
      bone.offsetMatrix =  glm::inverse(getModelMatrix("move/Cube", "move")) * (
        getModelMatrix(bone.name, bone.skeletonBase) * 
        glm::inverse(
          getInitialPose(bone)
        )
      );
    }
  }
}*/

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