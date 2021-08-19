#include "./playback.h"


glm::mat4 getInitialPose(Bone& bone, std::function<glm::mat4(std::string, std::string)>& getModelMatrix){
  auto boneTransform =  getModelMatrix(bone.name, bone.skeletonBase);
  if (!bone.initialPoseSet){
    bone.initialBonePose = boneTransform;
    bone.initialPoseSet = true;
    auto initPose = getTransformationFromMatrix(bone.initialBonePose);
    std::cout << "BONEINFO_ANIM: " << bone.name <<  " posn: " << print(initPose.position) << std::endl;
    std::cout << "BONEINFO_ANIM: " << bone.name << " scale: " << print(initPose.scale) << std::endl;
    std::cout << "BONEINFO_ANIM: " << bone.name << " rot: " << print(initPose.rotation) << std::endl;
  }
  return bone.initialBonePose;
}

void updateBonePoses(NameAndMeshObjName meshNameToMeshes, std::function<glm::mat4(std::string, std::string)> getModelMatrix){
  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    std::string meshname = meshNameToMeshes.objnames.at(i);
    for (Bone& bone : mesh.bones){
      std::cout << "skeletonBase is: " << bone.skeletonBase << std::endl;
      auto meshTransform = getModelMatrix(meshname, "move");
      //auto meshTransform = getModelMatrix("targetmodel/sentinel", "targetmodel");
      auto boneOffsetMatrix = (glm::inverse(getInitialPose(bone, getModelMatrix)) * getModelMatrix(bone.name, bone.skeletonBase));
      auto boTransform = getTransformationFromMatrix(boneOffsetMatrix);
      std::cout << "BONEINFO_OFFSET: " << bone.name <<  " posn: " << print(boTransform.position) << std::endl;
      std::cout << "BONEINFO_OFFSET: " << bone.name << " scale: " << print(boTransform.scale) << std::endl;
      std::cout << "BONEINFO_OFFSET: " << bone.name << " rot: " << print(boTransform.rotation) << std::endl;
      std::cout << "mesh name is : " << meshname << std::endl;

      bone.offsetMatrix = boneOffsetMatrix;
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
  NameAndMeshObjName meshNameToMeshes,  
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