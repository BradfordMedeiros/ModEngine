#include "./playback.h"

glm::mat4 getParentBone(std::map<std::string, std::string>& boneToParent, std::map<std::string, glm::mat4>& nodeToTransformedPose, std::string boneName){
  if (boneToParent.find(boneName) != boneToParent.end()){
    return nodeToTransformedPose.at(boneToParent.at(boneName));
  }
  return glm::mat4(1.f);
} 

void processNewPoseOnMesh(std::map<std::string, glm::mat4>& nodeToTransformedPose, std::string boneName, glm::mat4 newPose, NameAndMesh& meshData){
  for (int i = 0; i < meshData.meshes.size(); i++){
    Mesh& mesh = meshData.meshes.at(i);
    for (Bone& bone : mesh.bones){
      if (bone.name == boneName){    
        nodeToTransformedPose[bone.name] = newPose;
      }
    }
  }
}

void playbackAnimation(Animation animation,  std::map<std::string, std::map<std::string, std::string>>& meshnameToBoneToParent, NameAndMesh meshNameToMeshes,  float currentTime, float elapsedTime){  
  std::map<std::string, glm::mat4> nodeToTransformedPose;
  advanceAnimation(animation, currentTime, elapsedTime, [&meshNameToMeshes, &nodeToTransformedPose](std::string boneName, glm::mat4 newPose) -> void {
    processNewPoseOnMesh(nodeToTransformedPose, boneName, newPose, meshNameToMeshes);
  });

  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    std::string meshName = meshNameToMeshes.meshNames.at(i);
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    for (Bone& bone : mesh.bones){
      if (nodeToTransformedPose.find(bone.name) != nodeToTransformedPose.end()){
        bone.offsetMatrix = getParentBone(meshnameToBoneToParent.at(meshName), nodeToTransformedPose, bone.name) * nodeToTransformedPose.at(bone.name) * bone.initialOffsetMatrix;
      }
    }
  }
}
