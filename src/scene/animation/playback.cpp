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

glm::mat4 getOffsetMatrix(std::string bonename, NameAndMesh meshNameToMeshes){
  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    std::string meshName = meshNameToMeshes.meshNames.at(i);
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    for (Bone& bone : mesh.bones){
      if (bone.name == bonename){
        return bone.initialOffsetMatrix;
      }
    }
  }
  assert(false);
  return glm::mat4(1.f);
}

std::map<std::string, glm::mat4> initialBonePoses;
void setNewBoneOffsets(NameAndMesh meshNameToMeshes, std::function<glm::mat4(std::string)> getModelMatrix){
  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    for (Bone& bone : mesh.bones){
      auto boneTransform =  getModelMatrix(bone.name);
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
  std::function<glm::mat4(std::string)> getModelMatrix,
  std::function<void(std::string, glm::mat4)> setPose
){  
  auto posesForTick = animationPosesAtTime(animation, currentTime, elapsedTime);
  for (auto pose : posesForTick){
    setPose(pose.channelName, pose.pose);
  }
  setNewBoneOffsets(meshNameToMeshes, getModelMatrix);
}

void updateBonePoses(
  NameAndMesh meshNameToMeshes, 
  std::function<glm::mat4(std::string)> getModelMatrix
){
  setNewBoneOffsets(meshNameToMeshes, getModelMatrix);
}