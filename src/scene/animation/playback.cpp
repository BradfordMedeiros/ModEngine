#include "./playback.h"

bool boneExistsInMesh(std::string boneName, NameAndMesh& meshData){
  for (int i = 0; i < meshData.meshes.size(); i++){
    Mesh& mesh = meshData.meshes.at(i);
    for (Bone& bone : mesh.bones){
      if (bone.name == boneName){    
        return true;
      }
    }
  }
  return false;
}


void playbackAnimation(Animation animation,  std::map<std::string, std::map<std::string, std::string>>& meshnameToBoneToParent, NameAndMesh meshNameToMeshes,  float currentTime, float elapsedTime, 
  std::function<glm::mat4(std::string, bool)> getModelMatrix,
  std::function<void(std::string, glm::mat4)> setPose
){  
  std::map<std::string, glm::mat4> nodeToTransformedPose;
  auto posesForTick = animationPosesAtTime(animation, currentTime, elapsedTime);
  for (auto pose : posesForTick){
    setPose(pose.channelName, pose.pose);
  }

  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    std::string meshName = meshNameToMeshes.meshNames.at(i);
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    for (Bone& bone : mesh.bones){
      bone.offsetMatrix =  getModelMatrix(bone.name, true) ;
    }
  }
}
